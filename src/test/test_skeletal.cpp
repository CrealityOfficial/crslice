#include "crslice/test/test_skeletal.h"
#include "../conv.h"

#include "skeletal/BeadingStrategyFactory.h"
#include "skeletal/SkeletalTrapezoidation.h"
#include "skeletal/VariableLineUtils.h"
#include "tools/Cache.h"
#include "utils/VoronoiUtils.h"
#include "utils/linearAlg2D.h"

namespace crslice
{
    using namespace cura52;

    void convertSkeletalGraph(const SkeletalTrapezoidationGraph& graph, SkeletalGraph& skeletalGraph)
    {
        for (const STHalfEdge& edge : graph.edges)
        {
            SkeletalEdge e;
            e.from = convert(edge.from->p);
            e.to = convert(edge.to->p);
            skeletalGraph.edges.push_back(e);
        }

        for (const STHalfEdgeNode& node : graph.nodes)
        {
            SkeletalNode n;
            n.p = convert(node.p);
            skeletalGraph.nodes.push_back(n);
        }
    }

	void testSkeletal(const SerailCrSkeletal& skeletal, CrPolygons& innerPoly, std::vector<CrVariableLines>& out, SkeletalDetail* detail)
	{
        const CrSkeletalParam& param = skeletal.param;

        coord_t allowed_distance = MM2INT(param.allowed_distance);
        AngleRadians transitioning_angle = param.transitioning_angle;
        coord_t discretization_step_size = MM2INT(param.discretization_step_size);
        double scaled_spacing_wall_0 = param.scaled_spacing_wall_0;
        double scaled_spacing_wall_X = param.scaled_spacing_wall_X;
        coord_t wall_transition_length = MM2INT(param.wall_transition_length);
        double min_even_wall_line_width = param.min_even_wall_line_width;
        double wall_line_width_0 = param.wall_line_width_0;
        Ratio wall_split_middle_threshold = param.wall_split_middle_threshold;
        double min_odd_wall_line_width = param.min_odd_wall_line_width;
        double wall_line_width_x = param.wall_line_width_x;
        Ratio wall_add_middle_threshold = param.wall_add_middle_threshold;
        int wall_distribution_count = param.wall_distribution_count;
        size_t max_bead_count = param.max_bead_count;
        coord_t transition_filter_dist = MM2INT(param.transition_filter_dist);
        coord_t allowed_filter_deviation = MM2INT(param.allowed_filter_deviation);
        bool print_thin_walls = param.print_thin_walls > 0;
        coord_t min_feature_size = MM2INT(param.min_feature_size);
        coord_t min_bead_width = MM2INT(param.min_bead_width);
        coord_t wall_0_inset = MM2INT(param.wall_0_inset);

        coord_t stitch_distance = MM2INT(param.stitch_distance);
        coord_t max_resolution = MM2INT(param.max_resolution);
        coord_t max_deviation = MM2INT(param.max_deviation);
        coord_t max_area_deviation = MM2INT(param.max_area_deviation);

        const BeadingStrategyPtr beading_strat = BeadingStrategyFactory::makeStrategy
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

        Polygons prepared_outline;
        convertPolygonRaw(skeletal.polygons, prepared_outline);

        cura52::SkeletalTrapezoidation wall_maker
        (
            prepared_outline,
            *beading_strat,
            beading_strat->getTransitioningAngle(),
            discretization_step_size,
            transition_filter_dist,
            allowed_filter_deviation,
            wall_transition_length
        );

        if (detail)
            convertSkeletalGraph(wall_maker.graph, detail->graph);

        std::vector<VariableWidthLines> variableLines;
        wall_maker.generateToolpaths(variableLines);

        stitchToolPaths(variableLines, stitch_distance);
        removeSmallLines(variableLines);

        Polygons inner = separateOutInnerContour(variableLines, param.wall_inset_count);
        simplifyToolPaths(variableLines, max_resolution, max_deviation, max_area_deviation);
        removeEmptyToolPaths(variableLines);

        convertPolygonRaw(inner, innerPoly);
        convertVectorVariableLines(variableLines, out);
	}

    void testDiscretizeParabola(CrPolygon& points)
    {
        if (points.size() != 5)
            return;

        Polygons polys;
        polys.emplace_back(Polygon());
        convertRaw(points, polys.paths.at(0));
        const ClipperLib::Path& path = polys.paths.at(0);

        Point a = path.at(0);
        Point b = path.at(1);
        Point s = path.at(2);
        Point e = path.at(3);
        Point p = path.at(4);

        PolygonsSegmentIndex segment(&polys, 0, 0);

        coord_t approximate_step_size = 800;
        float transitioning_angle = 0.17453292519943275;

        do{
            std::vector<Point> discretized;
            // x is distance of point projected on the segment ab
            // xx is point projected on the segment ab
            const Point a = segment.from();
            const Point b = segment.to();
            const Point ab = b - a;
            const Point as = s - a;
            const Point ae = e - a;
            const coord_t ab_size = vSize(ab);
            if (ab_size == 0)
            {
                discretized.emplace_back(s);
                discretized.emplace_back(e);
                break;
            }
            const coord_t sx = dot(as, ab) / ab_size;
            const coord_t ex = dot(ae, ab) / ab_size;
            const coord_t sxex = ex - sx;

            const Point ap = p - a;
            const coord_t px = dot(ap, ab) / ab_size;

            const Point pxx = LinearAlg2D::getClosestOnLine(p, a, b);
            const Point ppxx = pxx - p;
            const coord_t d = vSize(ppxx);
            const PointMatrix rot = PointMatrix(turn90CCW(ppxx));

            if (d == 0)
            {
                discretized.emplace_back(s);
                discretized.emplace_back(e);
                break;
            }

            const float marking_bound = atan(transitioning_angle * 0.5);
            coord_t msx = -marking_bound * d; // projected marking_start
            coord_t mex = marking_bound * d; // projected marking_end
            const coord_t marking_start_end_h = msx * msx / (2 * d) + d / 2;
            Point marking_start = rot.unapply(Point(msx, marking_start_end_h)) + pxx;
            Point marking_end = rot.unapply(Point(mex, marking_start_end_h)) + pxx;
            const int dir = (sx > ex) ? -1 : 1;
            if (dir < 0)
            {
                std::swap(marking_start, marking_end);
                std::swap(msx, mex);
            }

            bool add_marking_start = msx * dir > (sx - px) * dir && msx * dir < (ex - px)* dir;
            bool add_marking_end = mex * dir > (sx - px) * dir && mex * dir < (ex - px)* dir;

            const Point apex = rot.unapply(Point(0, d / 2)) + pxx;
            // Only at the apex point if the projected start and end points
            // are more than 10 microns away from the projected apex
            bool add_apex = (sx - px) * dir < -10 && (ex - px) * dir > 10;

            //assert(!(add_marking_start && add_marking_end) || add_apex);
            if (add_marking_start && add_marking_end && !add_apex)
            {
                LOGW("Failing to discretize parabola! Must add an apex or one of the endpoints.");
                break;
            }

            const coord_t step_count = static_cast<coord_t>(static_cast<float>(std::abs(ex - sx)) / approximate_step_size + 0.5);

            discretized.emplace_back(s);
            for (coord_t step = 1; step < step_count; step++)
            {
                const coord_t x = sx + sxex * step / step_count - px;
                const coord_t y = x * x / (2 * d) + d / 2;

                if (add_marking_start && msx * dir < x * dir)
                {
                    discretized.emplace_back(marking_start);
                    add_marking_start = false;
                }
                if (add_apex && x * dir > 0)
                {
                    discretized.emplace_back(apex);
                    add_apex = false; // only add the apex just before the
                }
                if (add_marking_end && mex * dir < x * dir)
                {
                    discretized.emplace_back(marking_end);
                    add_marking_end = false;
                }
                const Point result = rot.unapply(Point(x, y)) + pxx;
                discretized.emplace_back(result);
            }
            if (add_apex)
            {
                discretized.emplace_back(apex);
            }
            if (add_marking_end)
            {
                discretized.emplace_back(marking_end);
            }
            discretized.emplace_back(e);
        } while (0);
    }
}
