// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher.

#include "WallsComputation.h"
#include "Application.h"
#include "ExtruderTrain.h"
#include "Slice.h"
#include "WallToolPaths.h"
#include "settings/types/Ratio.h"
#include "sliceDataStorage.h"
#include "utils/Simplify.h" // We're simplifying the spiralized insets.
#include "Slice3rBase/ExPolygon.hpp"
#include "Slice3rBase/BoundingBox.hpp"

#include "narrow_infill.h"



namespace cura52
{

WallsComputation::WallsComputation(const Settings& settings, const LayerIndex layer_nr, Application* _application)
    : settings(settings)
    , layer_nr(layer_nr)
    , application(_application)
{
}

/*
 * This function is executed in a parallel region based on layer_nr.
 * When modifying make sure any changes does not introduce data races.
 *
 * generateWalls only reads and writes data for the current layer
 */

void WallsComputation::split_top_surfaces(Slic3r::ExPolygons& orig_polygons, Slic3r::ExPolygons& top_fills,
    Slic3r::ExPolygons& non_top_polygons, Slic3r::ExPolygons& fill_clip, Slic3r::ExPolygons& upper_slices, Slic3r::ExPolygons& lower_slices)  {
    //// other perimeters
    //coord_t perimeter_width = this->perimeter_flow.scaled_width();
    //coord_t perimeter_spacing = this->perimeter_flow.scaled_spacing();
    //// external perimeters
    //coord_t ext_perimeter_width = this->ext_perimeter_flow.scaled_width();
    //coord_t ext_perimeter_spacing = this->ext_perimeter_flow.scaled_spacing();
    //bool has_gap_fill = this->config->gap_infill_speed.value > 0;
    // split the polygons with top/not_top
    // get the offset from solid surface anchor
    //coord_t offset_top_surface =
    //    scale_(1.5 * (config->wall_loops.value == 0
    //        ? 0.
    //        : unscaled(double(ext_perimeter_width +
    //            perimeter_spacing * int(int(config->wall_loops.value) - int(1))))));
    //// if possible, try to not push the extra perimeters inside the sparse infill
    //if (offset_top_surface >
    //    0.9 * (config->wall_loops.value <= 1 ? 0. : (perimeter_spacing * (config->wall_loops.value - 1))))
    //    offset_top_surface -=
    //    coord_t(0.9 * (config->wall_loops.value <= 1 ? 0. : (perimeter_spacing * (config->wall_loops.value - 1))));
    //else
    //    offset_top_surface = 0;
    //// don't takes into account too thin areas
    //// skip if the exposed area is smaller than "min_width_top_surface"
    //double min_width_top_surface = std::max(double(ext_perimeter_spacing / 2 + 10), config->min_width_top_surface.get_abs_value(perimeter_width));
    coord_t perimeter_width = 449;
    coord_t perimeter_spacing = 407;
    double min_width_top_surface = 134;
    coord_t ext_perimeter_spacing = 377;
    bool has_gap_fill = true;
    coord_t offset_top_surface = 874;
    Slic3r::Polygons grown_upper_slices = Slic3r::offset(upper_slices, min_width_top_surface);

    // get boungding box of last
    Slic3r::BoundingBox last_box = Slic3r::get_extents(orig_polygons);
    last_box.offset(SCALED_EPSILON);

    // get the Polygons upper the polygon this layer
    Slic3r::Polygons upper_polygons_series_clipped =
        Slic3r::ClipperUtils::clip_clipper_polygons_with_subject_bbox(grown_upper_slices, last_box);

    // set the clip to a virtual "second perimeter"
    fill_clip = offset_ex(orig_polygons, -double(ext_perimeter_spacing));
    // get the real top surface
    Slic3r::ExPolygons grown_lower_slices;
    Slic3r::ExPolygons bridge_checker;
    auto nozzle_diameter = settings.get<size_t>("machine_nozzle_size");
    // Check whether surface be bridge or not
    if (!lower_slices.empty()) {
        // BBS: get the Polygons below the polygon this layer
        Slic3r::Polygons lower_polygons_series_clipped =
            Slic3r::ClipperUtils::clip_clipper_polygons_with_subject_bbox(lower_slices, last_box);
        double bridge_offset = std::max(double(ext_perimeter_spacing), (double(perimeter_width)));
        // SoftFever: improve bridging
        const float bridge_margin =
            std::min(float(scale_(BRIDGE_INFILL_MARGIN)), float(scale_(nozzle_diameter * BRIDGE_INFILL_MARGIN / 0.4)));
        bridge_checker = Slic3r::offset_ex(diff_ex(orig_polygons, lower_polygons_series_clipped, Slic3r::ApplySafetyOffset::Yes),
            1.5 * bridge_offset + bridge_margin + perimeter_spacing / 2);
    }
    Slic3r::ExPolygons delete_bridge = Slic3r::diff_ex(orig_polygons, bridge_checker, Slic3r::ApplySafetyOffset::Yes);

    Slic3r::ExPolygons top_polygons = Slic3r::diff_ex(delete_bridge, upper_polygons_series_clipped, Slic3r::ApplySafetyOffset::Yes);
    // get the not-top surface, from the "real top" but enlarged by external_infill_margin (and the
    // min_width_top_surface we removed a bit before)
    Slic3r::ExPolygons temp_gap = Slic3r::diff_ex(top_polygons, fill_clip);
    Slic3r::ExPolygons inner_polygons =
        Slic3r::diff_ex(orig_polygons,
            Slic3r::offset_ex(top_polygons, offset_top_surface + min_width_top_surface - double(ext_perimeter_spacing / 2)),
            Slic3r::ApplySafetyOffset::Yes);
    // get the enlarged top surface, by using inner_polygons instead of upper_slices, and clip it for it to be exactly
    // the polygons to fill.
    top_polygons = Slic3r::diff_ex(fill_clip, inner_polygons, Slic3r::ApplySafetyOffset::Yes);
    // increase by half peri the inner space to fill the frontier between last and stored.
    top_fills = Slic3r::union_ex(top_fills, top_polygons);
    //set the clip to the external wall but go back inside by infill_extrusion_width/2 to be sure the extrusion won't go outside even with a 100% overlap.
    //double infill_spacing_unscaled = this->config->sparse_infill_line_width.get_abs_value(nozzle_diameter);
    double infill_spacing_unscaled = 0.45 ;
    double sssAS = scale_(infill_spacing_unscaled / 2);
    //if (infill_spacing_unscaled == 0) infill_spacing_unscaled = Slic3r::Flow::auto_extrusion_width(frInfill, nozzle_diameter);
    fill_clip = Slic3r::offset_ex(orig_polygons, double(ext_perimeter_spacing / 2) - sssAS);
    // ExPolygons oldLast = last;

    non_top_polygons = Slic3r::intersection_ex(inner_polygons, orig_polygons);
    if (has_gap_fill)
        non_top_polygons = Slic3r::union_ex(non_top_polygons, temp_gap);
    //{
    //    std::stringstream stri;
    //    stri << this->layer_id << "_1_"<< i <<"_only_one_peri"<< ".svg";
    //    SVG svg(stri.str());
    //    svg.draw(to_polylines(top_fills), "green");
    //    svg.draw(to_polylines(inner_polygons), "yellow");
    //    svg.draw(to_polylines(top_polygons), "cyan");
    //    svg.draw(to_polylines(oldLast), "orange");
    //    svg.draw(to_polylines(last), "red");
    //    svg.Close();
    //}
}

void WallsComputation::generateWalls(SliceLayerPart* part, SliceLayer* layer_upper, SliceLayer* layer_lower)
{
    size_t wall_count = settings.get<size_t>("wall_line_count");
    if (wall_count == 0) // Early out if no walls are to be generated
    {
        part->print_outline = part->outline;
        part->inner_area = part->outline;
        return;
    }

    const bool spiralize = settings.get<bool>("magic_spiralize");
    const size_t alternate = ((layer_nr % 2) + 2) % 2;
    if (spiralize && layer_nr < LayerIndex(settings.get<size_t>("initial_bottom_layers")) && alternate == 1) //Add extra insets every 2 layers when spiralizing. This makes bottoms of cups watertight.
    {
        wall_count += 5;
    }
    if (settings.get<bool>("alternate_extra_perimeter"))
    {
        wall_count += alternate;
    }

    const bool first_layer = layer_nr == 0;
    const Ratio line_width_0_factor = first_layer ? settings.get<ExtruderTrain&>("wall_0_extruder_nr").settings.get<Ratio>("initial_layer_line_width_factor") : 1.0_r;
    const coord_t line_width_0 = settings.get<coord_t>("wall_line_width_0") * line_width_0_factor;
    const coord_t wall_0_inset = settings.get<coord_t>("wall_0_inset");

    const Ratio line_width_x_factor = first_layer ? settings.get<ExtruderTrain&>("wall_x_extruder_nr").settings.get<Ratio>("initial_layer_line_width_factor") : 1.0_r;
    const coord_t line_width_x = settings.get<coord_t>("wall_line_width_x") * line_width_x_factor;

    // When spiralizing, generate the spiral insets using simple offsets instead of generating toolpaths
    if (spiralize)
    {
        const bool recompute_outline_based_on_outer_wall =
                settings.get<bool>("support_enable") && !settings.get<bool>("fill_outline_gaps");

        generateSpiralInsets(part, line_width_0, wall_0_inset, recompute_outline_based_on_outer_wall);
        if (layer_nr <= static_cast<LayerIndex>(settings.get<size_t>("bottom_layers")))
        {
            WallToolPaths wall_tool_paths(part->outline, line_width_0, line_width_x, wall_count, wall_0_inset, settings);
            part->wall_toolpaths = wall_tool_paths.getToolPaths();
            part->inner_area = wall_tool_paths.getInnerContour();
        }
    }
    else
    {
        const bool roofing_only_one_wall = settings.get<bool>("roofing_only_one_wall");
        Polygons upLayerPart;
        Polygons lowLayerPart;
        if (roofing_only_one_wall && layer_upper != nullptr)
        {
            for (const SliceLayerPart& part2 : layer_upper->parts)
            {
                if (part->boundaryBox.hit(part2.boundaryBox))
                {
                    upLayerPart.add(part2.outline);
                }
            }
        }
        if (roofing_only_one_wall && layer_lower != nullptr)
        {
            for (const SliceLayerPart& part2 : layer_lower->parts)
            {
                if (part->boundaryBox.hit(part2.boundaryBox))
                {
                    lowLayerPart.add(part2.outline);
                }
            }
        }
        Polygons layer_different_area = part->outline.difference(upLayerPart);
        Polygons layer_inter_area = part->outline.intersection(upLayerPart.offset(-line_width_0));
        if (wall_count > 1 && roofing_only_one_wall && !first_layer && layer_different_area.area() > line_width_0 * line_width_0)
        {
            WallToolPaths OuterWall_tool_paths(part->outline, line_width_0, line_width_x, 1, wall_0_inset, settings);
            part->wall_toolpaths = OuterWall_tool_paths.getToolPaths();
            Polygons non_OuterWall_area = OuterWall_tool_paths.getInnerContour();
            Polygons roof_area = non_OuterWall_area.difference(upLayerPart);


            //Slic3r::ExPolygons top_fills;
            //Slic3r::ExPolygons fill_clip;
            //Slic3r::ExPolygons non_top_polygons;
            //Slic3r::ExPolygon last = convert(part->outline);
            //Slic3r::ExPolygons lasts;
            //lasts.emplace_back(last);
            //Slic3r::ExPolygon upper_slice = convert(upLayerPart);
            //Slic3r::ExPolygons upper_slices;
            //upper_slices.emplace_back(upper_slice);
            //Slic3r::ExPolygon lower_slice = convert(lowLayerPart);
            //Slic3r::ExPolygons lower_slices;
            //lower_slices.emplace_back(lower_slice);
            //split_top_surfaces(lasts, top_fills, non_top_polygons, fill_clip, upper_slices, lower_slices);  //this function need to study carefully  , which belong in to orca . param lower layer is vi
            //if (!top_fills.empty())

            ////offjob  co-worker files in feishu that is about  topsurface 
            coord_t half_min_roof_width = (line_width_0 + (wall_count - 1) * line_width_x) / 2;
            roof_area = roof_area.intersection(non_OuterWall_area);
            if (result_is_narrow_infill_area(roof_area) && roof_area.offset(-half_min_roof_width).size() < roof_area.paths.size())
                roof_area = roof_area.offset(-half_min_roof_width).offset(half_min_roof_width + line_width_0).intersection(non_OuterWall_area);  //narrow polygon get 0  no small  scraps in surface
            else
                roof_area = roof_area.intersection(non_OuterWall_area);  // for better polygons , beacause of offset have some bad influence on utimate plygons
            Polygons inside_area = non_OuterWall_area.difference(roof_area);
           
            if (!inside_area.empty())
            {
                WallToolPaths innerWall_tool_paths(inside_area, line_width_x, line_width_x, wall_count - 1, 0, settings);
                std::vector<VariableWidthLines> innerWall_toolpaths = innerWall_tool_paths.getToolPaths();
                for (VariableWidthLines& paths : innerWall_toolpaths)
                {
                    for (ExtrusionLine& path : paths)
                    {
                        path.inset_idx++;
                    }
                }
                part->wall_toolpaths.insert(part->wall_toolpaths.end(), innerWall_toolpaths.begin(), innerWall_toolpaths.end());
                part->inner_area = innerWall_tool_paths.getInnerContour();
            }
            part->inner_area.add(roof_area);  //top suface inner wall
        }
        else
        {
            WallToolPaths wall_tool_paths(part->outline, line_width_0, line_width_x, wall_count, wall_0_inset, settings);
            part->wall_toolpaths = wall_tool_paths.getToolPaths();
            part->inner_area = wall_tool_paths.getInnerContour();
        }
    }
    part->print_outline = part->outline;
}

/*
 * This function is executed in a parallel region based on layer_nr.
 * When modifying make sure any changes does not introduce data races.
 *
 * generateWalls only reads and writes data for the current layer
 */
void WallsComputation::generateWalls(SliceLayer* layer, SliceLayer* layer_upper, SliceLayer* layer_lower)
{
    for(SliceLayerPart& part : layer->parts)
    {
        INTERRUPT_BREAK("WallsComputation::generateWalls. ");
        generateWalls(&part, layer_upper,  layer_lower);
    }

    //Remove the parts which did not generate a wall. As these parts are too small to print,
    // and later code can now assume that there is always minimal 1 wall line.
    if(settings.get<size_t>("wall_line_count") >= 1 && !settings.get<bool>("fill_outline_gaps"))
    {
        for(size_t part_idx = 0; part_idx < layer->parts.size(); part_idx++)
        {
            if (layer->parts[part_idx].wall_toolpaths.empty() && layer->parts[part_idx].spiral_wall.empty())
            {
                if (part_idx != layer->parts.size() - 1)
                { // move existing part into part to be deleted
                    layer->parts[part_idx] = std::move(layer->parts.back());
                }
                layer->parts.pop_back(); // always remove last element from array (is more efficient)
                part_idx -= 1; // check the part we just moved here
            }
        }
    }
}

void WallsComputation::generateSpiralInsets(SliceLayerPart *part, coord_t line_width_0, coord_t wall_0_inset, bool recompute_outline_based_on_outer_wall)
{
    part->spiral_wall = part->outline.offset(-line_width_0 / 2 - wall_0_inset);

    //Optimize the wall. This prevents buffer underruns in the printer firmware, and reduces processing time in CuraEngine.
    const ExtruderTrain& train_wall = settings.get<ExtruderTrain&>("wall_0_extruder_nr");
    part->spiral_wall = Simplify(train_wall.settings).polygon(part->spiral_wall);
    part->spiral_wall.removeDegenerateVerts();
    if (recompute_outline_based_on_outer_wall)
    {
        part->print_outline = part->spiral_wall.offset(line_width_0 / 2, ClipperLib::jtSquare);
    }
    else
    {
        part->print_outline = part->outline;
    }
}

}//namespace cura52
