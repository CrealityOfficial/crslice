// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher.

#include <algorithm> //For std::partition_copy and std::min_element.
#include <unordered_set>

#include "WallToolPaths.h"

#include "SkeletalTrapezoidation.h"
#include "utils/SparsePointGrid.h" //To stitch the inner contour.
#include "utils/polygonUtils.h"
#include "ExtruderTrain.h"
#include "utils/PolylineStitcher.h"
#include "utils/Simplify.h"
#include "utils/inputserial.h"

namespace cura52
{

WallToolPaths::WallToolPaths(const Polygons& outline, const coord_t nominal_bead_width, const size_t inset_count, const coord_t wall_0_inset,
                             const Settings& settings)
    : outline(outline)
    , bead_width_0(nominal_bead_width)
    , bead_width_x(nominal_bead_width)
    , inset_count(inset_count)
    , wall_0_inset(wall_0_inset)
    , print_thin_walls(settings.get<bool>("fill_outline_gaps"))
    , min_feature_size(settings.get<coord_t>("min_feature_size"))
    , min_bead_width(settings.get<coord_t>("min_bead_width"))
    , small_area_length(INT2MM(static_cast<double>(nominal_bead_width) / 2))
    , toolpaths_generated(false)
    , settings(settings)
{
}

WallToolPaths::WallToolPaths(const Polygons& outline, const coord_t bead_width_0, const coord_t bead_width_x,
                             const size_t inset_count, const coord_t wall_0_inset, const Settings& settings)
    : outline(outline)
    , bead_width_0(bead_width_0)
    , bead_width_x(bead_width_x)
    , inset_count(inset_count)
    , wall_0_inset(wall_0_inset)
    , print_thin_walls(settings.get<bool>("fill_outline_gaps"))
    , min_feature_size(settings.get<coord_t>("min_feature_size"))
    , min_bead_width(settings.get<coord_t>("min_bead_width"))
    , small_area_length(INT2MM(static_cast<double>(bead_width_0) / 2))
    , toolpaths_generated(false)
    , settings(settings)
{
}

const std::vector<VariableWidthLines>& WallToolPaths::generate()
{
    const coord_t allowed_distance = settings.get<coord_t>("meshfix_maximum_deviation");
    const coord_t epsilon_offset = (allowed_distance / 2) - 1;
    const AngleRadians transitioning_angle = settings.get<AngleRadians>("wall_transition_angle");
    constexpr coord_t discretization_step_size = MM2INT(0.8);

    double scaled_spacing_wall_0 = bead_width_0;
    double scaled_spacing_wall_X = bead_width_x;
    const coord_t layer_thickness = settings.get<coord_t>("layer_height");
    const bool exact_flow_enable = settings.get<bool>("special_exact_flow_enable");
    auto getScaledSpacing = [layer_thickness](size_t line_width)
    {
        size_t out = line_width - layer_thickness * float(1. - 0.25 * M_PI);
        if (out <= 0.f)
            return line_width;
        return out;
    };
    if (exact_flow_enable)
    {
        scaled_spacing_wall_0 = getScaledSpacing(bead_width_0);
        scaled_spacing_wall_X = getScaledSpacing(bead_width_x);        
    }

    auto delete_shape_point = [&](Polygons& polys)
    {
        int delete_point_num = 0;
        do
        {
            delete_point_num = 0;
            for (int i = 0; i < polys.size(); i++)
            {
                int pol_size = polys[i].size();
                for (int j = 0; j < pol_size; j++)
                {
                    Point pt = polys[i][j];
                    Point before = polys[i][(j - 1 + pol_size) % pol_size];
                    Point after = polys[i][(j + 1 + pol_size) % pol_size];
                    float angle = LinearAlg2D::getAngleLeft(before, pt, after);
                    if (angle < 0.5 || angle > 2 * M_PI - 0.5)
                    {
                        coord_t length2_before = vSize2(before - pt);
                        coord_t length2_after = vSize2(after - pt);
                        coord_t v_ab_dis;
                        if (length2_before < length2_after)
                        {
                            v_ab_dis = LinearAlg2D::getDistFromLine(before, pt, after);
                        }
                        else
                        {
                            v_ab_dis = LinearAlg2D::getDistFromLine(after, pt, before);
                        }
                        if (v_ab_dis < epsilon_offset)
                        {
                            polys[i].remove(j);
                            j--;
                            pol_size--;
                            delete_point_num++;
                        }
                    }
                }
            }
        } while (delete_point_num > 0);
    };

    auto cut_hole = [&](Polygons& polys, coord_t cut_len)
    {
        Polygons polys_a;
        Polygons polys_n;

        for (auto path : polys)
        {
            Polygon poly(path);
            if (poly.orientation())
                polys_a.add(poly);
            else
            {
                poly.reverse();
                polys_n.add(poly);
            }
        }
        polys_n = polys_n.intersection(polys_a.offset(-cut_len));
        polys = polys_a.difference(polys_n);
    };
    // Simplify outline for boost::voronoi consumption. Absolutely no self intersections or near-self intersections allowed:
    // TODO: Open question: Does this indeed fix all (or all-but-one-in-a-million) cases for manifold but otherwise possibly complex polygons?
    Polygons prepared_outline = outline.offset(-epsilon_offset).offset(epsilon_offset * 2).offset(-epsilon_offset);    
    prepared_outline = Simplify(settings).polygon(prepared_outline);
    PolygonUtils::fixSelfIntersections(epsilon_offset, prepared_outline);
    prepared_outline.removeDegenerateVerts();
    prepared_outline.removeColinearEdges(AngleRadians(0.005));
    // Removing collinear edges may introduce self intersections, so we need to fix them again
    PolygonUtils::fixSelfIntersections(epsilon_offset, prepared_outline);
    prepared_outline.removeDegenerateVerts();
    prepared_outline.removeSmallAreas(small_area_length * small_area_length, false);
    delete_shape_point(prepared_outline);
    cut_hole(prepared_outline, 3);
    prepared_outline = prepared_outline.unionPolygons();

    if (prepared_outline.area() <= 0)
    {
        assert(toolpaths.empty());
        return toolpaths;
    }

    //Polygons poltoWrite;
    //{
    //    Point error_point = Point(117607, 141474);
    //    Polygon error_area;
    //    error_area.add(Point(error_point.X + 50, error_point.Y + 50));
    //    error_area.add(Point(error_point.X + 50, error_point.Y - 50));
    //    error_area.add(Point(error_point.X - 50, error_point.Y - 50));
    //    error_area.add(Point(error_point.X - 50, error_point.Y + 50));
    //    Polygons error_areas;
    //    error_areas.add(error_area);
    //    Polygons intersection_polys = prepared_outline.intersection(error_areas);
    //    if (!intersection_polys.empty())
    //    {
    //        poltoWrite.add(intersection_polys);
    //    }
    //    if (!poltoWrite.empty() && std::fabs(poltoWrite.area()) != std::fabs(error_area.area()))
    //    {
    //        AABB aabb(poltoWrite);
    //        SVG svg("D://outline.svg", aabb);
    //        svg.writePolygons(poltoWrite, SVG::Color::BLACK, 0.1);
    //        svg.writePoint(error_point, false, 1);
    //    }
    //}

    const coord_t wall_transition_length = settings.get<coord_t>("wall_transition_length");

    // When to split the middle wall into two:
    const double min_even_wall_line_width = settings.get<double>("min_even_wall_line_width");
    const double wall_line_width_0 = settings.get<double>("wall_line_width_0");
    const Ratio wall_split_middle_threshold = std::max(1.0, std::min(99.0, 100.0 * (2.0 * min_even_wall_line_width - wall_line_width_0) / wall_line_width_0)) / 100.0;

    // When to add a new middle in between the innermost two walls:
    const double min_odd_wall_line_width = settings.get<double>("min_odd_wall_line_width");
    const double wall_line_width_x = settings.get<double>("wall_line_width_x");
    const Ratio wall_add_middle_threshold = std::max(1.0, std::min(99.0, 100.0 * min_odd_wall_line_width / wall_line_width_x)) / 100.0;

    const int wall_distribution_count = settings.get<int>("wall_distribution_count");
    const size_t max_bead_count = (inset_count < std::numeric_limits<coord_t>::max() / 2) ? 2 * inset_count : std::numeric_limits<coord_t>::max();
    const auto beading_strat = BeadingStrategyFactory::makeStrategy
        (
            scaled_spacing_wall_0,
            scaled_spacing_wall_X,
            wall_transition_length,
            transitioning_angle,
            print_thin_walls,
            min_bead_width,
            min_feature_size,
            wall_split_middle_threshold,
            wall_add_middle_threshold,
            max_bead_count,
            wall_0_inset,
            wall_distribution_count
        );
    const coord_t transition_filter_dist = settings.get<coord_t>("wall_transition_filter_distance");
    const coord_t allowed_filter_deviation = settings.get<coord_t>("wall_transition_filter_deviation");

    SkeletalTrapezoidation wall_maker
    (
        prepared_outline,
        *beading_strat,
        beading_strat->getTransitioningAngle(),
        discretization_step_size,
        transition_filter_dist,
        allowed_filter_deviation,
        wall_transition_length
    );

#if CURA_SERIAL_DATA
        SkeletalTrapezoidationTester tester;
        
        tester.allowed_distance = allowed_distance;
        tester.transitioning_angle = transitioning_angle;
        tester.discretization_step_size = discretization_step_size;
        tester.scaled_spacing_wall_0 = scaled_spacing_wall_0;
        tester.scaled_spacing_wall_X = scaled_spacing_wall_X;
        tester.wall_transition_length = wall_transition_length;
        tester.min_even_wall_line_width = min_even_wall_line_width;
        tester.wall_line_width_0 = wall_line_width_0;
        tester.wall_split_middle_threshold = wall_split_middle_threshold;
        tester.min_odd_wall_line_width = min_odd_wall_line_width;
        tester.wall_line_width_x = wall_line_width_x;
        tester.wall_add_middle_threshold = wall_add_middle_threshold;
        tester.wall_distribution_count = wall_distribution_count;
        tester.max_bead_count = max_bead_count;
        tester.transition_filter_dist = transition_filter_dist;
        tester.allowed_filter_deviation = allowed_filter_deviation;
        tester.print_thin_walls = print_thin_walls;
        tester.min_feature_size = min_feature_size;
        tester.min_bead_width = min_bead_width;
        tester.wall_0_inset = wall_0_inset;
        tester.prepared_outline = prepared_outline;

        char name[256] = { 0 };
        sprintf(name, "SkeletalTrapezoidation-%d", reinterpret_cast<uint64_t>(this));
        tester.save(name);
#endif

    wall_maker.generateToolpaths(toolpaths);

    stitchToolPaths(toolpaths, settings);
    
    removeSmallLines(toolpaths);

    separateOutInnerContour();
    
    simplifyToolPaths(toolpaths, settings);

    removeEmptyToolPaths(toolpaths);

    for (VariableWidthLines& path : toolpaths)
    {        
        VariableWidthLines path_new;
        for (ExtrusionLine& line : path)
        {
            if (line.getLength() < allowed_distance) continue;
            line.start_idx = -1;
            path_new.push_back(line);
        }
        path.swap(path_new);
    }
    assert(std::is_sorted(toolpaths.cbegin(), toolpaths.cend(),
                          [](const VariableWidthLines& l, const VariableWidthLines& r)
                          {
                              return l.front().inset_idx < r.front().inset_idx;
                          }) && "WallToolPaths should be sorted from the outer 0th to inner_walls");
    toolpaths_generated = true;
    return toolpaths;
}


void WallToolPaths::stitchToolPaths(std::vector<VariableWidthLines>& toolpaths, const Settings& settings)
{
    const coord_t stitch_distance = settings.get<coord_t>("wall_line_width_x") - 1; //In 0-width contours, junctions can cause up to 1-line-width gaps. Don't stitch more than 1 line width.

    for (unsigned int wall_idx = 0; wall_idx < toolpaths.size(); wall_idx++)
    {
        VariableWidthLines& wall_lines = toolpaths[wall_idx];
        
        VariableWidthLines stitched_polylines;
        VariableWidthLines closed_polygons;
        PolylineStitcher<VariableWidthLines, ExtrusionLine, ExtrusionJunction>::stitch(wall_lines, stitched_polylines, closed_polygons, stitch_distance);
        wall_lines = stitched_polylines; // replace input toolpaths with stitched polylines

        for (ExtrusionLine& wall_polygon : closed_polygons)
        {
            if (wall_polygon.junctions.empty())
            {
                continue;
            }
            // PolylineStitcher, in some cases, produced closed extrusion (polygons),
            // but the endpoints differ by a small distance. So we reconnect them.
            if (wall_polygon.junctions.front().p != wall_polygon.junctions.back().p &&
                vSize(wall_polygon.junctions.back().p - wall_polygon.junctions.front().p) < stitch_distance) {
                wall_polygon.junctions.emplace_back(wall_polygon.junctions.front());
            }
            wall_polygon.is_closed = true;
            wall_lines.emplace_back(std::move(wall_polygon)); // add stitched polygons to result
        }
#ifdef DEBUG
        for (ExtrusionLine& line : wall_lines)
        {
            assert(line.inset_idx == wall_idx);
        }
#endif // DEBUG
    }
}

void WallToolPaths::removeSmallLines(std::vector<VariableWidthLines>& toolpaths)
{
    for (VariableWidthLines& inset : toolpaths)
    {
        for (size_t line_idx = 0; line_idx < inset.size(); line_idx++)
        {
            ExtrusionLine& line = inset[line_idx];
            coord_t min_width = std::numeric_limits<coord_t>::max();
            for (const ExtrusionJunction& j : line)
            {
                min_width = std::min(min_width, j.w);
            }
            if (line.is_odd && ! line.is_closed && shorterThan(line, min_width / 2))
            { // remove line
                line = std::move(inset.back());
                inset.erase(--inset.end());
                line_idx--; // reconsider the current position
            }
        }
    }
}

void WallToolPaths::simplifyToolPaths(std::vector<VariableWidthLines>& toolpaths, const Settings& settings)
{
    const Simplify simplifier(settings);
    for(size_t toolpaths_idx = 0; toolpaths_idx < toolpaths.size(); ++toolpaths_idx)
    {
        for(ExtrusionLine& line : toolpaths[toolpaths_idx])
        {
            line = simplifier.polyline(line);
        }
    }
}

const std::vector<VariableWidthLines>& WallToolPaths::getToolPaths()
{
    if (!toolpaths_generated)
    {
        return generate();
    }
    return toolpaths;
}

void WallToolPaths::pushToolPaths(std::vector<VariableWidthLines>& paths)
{
    if (! toolpaths_generated)
    {
        generate();
    }
    paths.insert(paths.end(), toolpaths.begin(), toolpaths.end());
}

void WallToolPaths::separateOutInnerContour()
{
    //We'll remove all 0-width paths from the original toolpaths and store them separately as polygons.
    std::vector<VariableWidthLines> actual_toolpaths;
    actual_toolpaths.reserve(toolpaths.size()); //A bit too much, but the correct order of magnitude.
    std::vector<VariableWidthLines> contour_paths;
    contour_paths.reserve(toolpaths.size() / inset_count);
    inner_contour.clear();
    for (const VariableWidthLines& inset : toolpaths)
    {
        if (inset.empty())
        {
            continue;
        }
        bool is_contour = false;
        for (const ExtrusionLine& line : inset)
        {
            for (const ExtrusionJunction& j : line)
            {
                if (j.w == 0)
                {
                    is_contour = true;
                }
                else
                {
                    is_contour = false;
                }
                break;
            }
        }
                    
                    
        if (is_contour)
        {
#ifdef DEBUG
            for (const ExtrusionLine& line : inset)
            {
                for (const ExtrusionJunction& j : line)
                {
                    assert(j.w == 0);
                }
            }
#endif // DEBUG
            for (const ExtrusionLine& line : inset)
            {
                if (line.is_odd)
                {
                    continue; // odd lines don't contribute to the contour
                }
                else if (line.is_closed) // sometimes an very small even polygonal wall is not stitched into a polygon
                {
                    inner_contour.emplace_back(line.toPolygon());
                }
            }
        }
        else
        {
            actual_toolpaths.emplace_back(inset);
        }
    }
    if (! actual_toolpaths.empty())
    {
        toolpaths = std::move(actual_toolpaths); //Filtered out the 0-width paths.
    }
    else
    {
        toolpaths.clear();
    }

    //The output walls from the skeletal trapezoidation have no known winding order, especially if they are joined together from polylines.
    //They can be in any direction, clockwise or counter-clockwise, regardless of whether the shapes are positive or negative.
    //To get a correct shape, we need to make the outside contour positive and any holes inside negative.
    //This can be done by applying the even-odd rule to the shape. This rule is not sensitive to the winding order of the polygon.
    //The even-odd rule would be incorrect if the polygon self-intersects, but that should never be generated by the skeletal trapezoidation.
    inner_contour = inner_contour.processEvenOdd();
}

const Polygons& WallToolPaths::getInnerContour()
{
    if (!toolpaths_generated && inset_count > 0)
    {
        generate();
    }
    else if(inset_count == 0)
    {
        return outline;
    }
    return inner_contour;
}

bool WallToolPaths::removeEmptyToolPaths(std::vector<VariableWidthLines>& toolpaths)
{
    toolpaths.erase(std::remove_if(toolpaths.begin(), toolpaths.end(), [](const VariableWidthLines& lines)
                                   {
                                       return lines.empty();
                                   }), toolpaths.end());
    return toolpaths.empty();
}

} // namespace cura52
