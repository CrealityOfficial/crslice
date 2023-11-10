#include "crslice/test/test_skeletal.h"
#include "../conv.h"

#include "skeletal/BeadingStrategyFactory.h"
#include "skeletal/SkeletalTrapezoidation.h"
#include "skeletal/VariableLineUtils.h"
#include "tools/Cache.h"
#include "utils/VoronoiUtils.h"

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
        Point b = path.at(0);
        Point start = path.at(0);
        Point end = path.at(0);
        Point point = path.at(0);

        PolygonsSegmentIndex segment(&polys, 0, 0);
        ClipperLib::Path dePath = VoronoiUtils::discretizeParabola(point, segment, start, end, 800, 0.1);
    }
}
