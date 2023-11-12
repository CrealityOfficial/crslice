#include "skeletalimpl.h"

namespace crslice
{
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

    void SkeletalCheckImpl::skeletalTrapezoidation(CrPolygons& innerPoly, std::vector<CrVariableLines>& out,
        SkeletalDetail* detail)
    {
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

        cura52::SkeletalTrapezoidation wall_maker
        (
            input,
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

        Polygons inner = separateOutInnerContour(variableLines, wall_inset_count);
        simplifyToolPaths(variableLines, max_resolution, max_deviation, max_area_deviation);
        removeEmptyToolPaths(variableLines);

        convertPolygonRaw(inner, innerPoly);
        convertVectorVariableLines(variableLines, out);
    }
}
