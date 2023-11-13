#include "skeletalimpl.h"

namespace crslice
{
    void constructSegments(const cura52::Polygons& input, std::vector<Segment>& segments)
    {
        for (size_t poly_idx = 0; poly_idx < input.size(); poly_idx++)
        {
            for (size_t point_idx = 0; point_idx < input[poly_idx].size(); point_idx++)
            {
                segments.emplace_back(&input, poly_idx, point_idx);
            }
        }
    }


    void SkeletalCheckImpl::setInput(const SerailCrSkeletal& skeletal)
    {
        clear();

        crslice::convertPolygonRaw(skeletal.polygons, input);
        setParam(skeletal.param);

        constructSegments(input, segments);
        boost::polygon::construct_voronoi(segments.begin(), segments.end(), &graph);

        const bool   has_missing_voronoi_vertex = VoronoiUtils::detect_missing_voronoi_vertex(graph, segments);
        // Detection of non-planar Voronoi diagram detects at least GH issues #8474, #8514 and #8446.
        const bool   is_voronoi_diagram_planar = VoronoiUtilsCgal::is_voronoi_diagram_planar_angle(graph);

        if (has_missing_voronoi_vertex || !is_voronoi_diagram_planar) {
            LOGE("has_missing_voronoi_vertex || !is_voronoi_diagram_planar");
        }

        bool degenerated_voronoi_diagram = has_missing_voronoi_vertex || !is_voronoi_diagram_planar;
        if (degenerated_voronoi_diagram) {
            LOGE("degenerated_voronoi_diagram");
        }

        for (const ClipperLib::Path& path : input.paths)
            for (const Point& p : path)
                box.include(p);

        box.expand(1000);

        classifyEdge();
        transfer();
    }

    bool SkeletalCheckImpl::isValid()
    {
        return segments.size() > 0 &&
            graph.num_cells() > 0 && graph.num_edges() > 0 &&
            input.size() > 0;
    }

    void SkeletalCheckImpl::clear()
    {
        input.clear();
        segments.clear();
        points.clear();
        graph.clear();
    }

    int SkeletalCheckImpl::edgeIndex(edge_type* edge)
    {
        if (graph.num_edges() == 0)
            return -1;

        return edge - &*graph.edges().begin();
    }

    void SkeletalCheckImpl::setParam(const CrSkeletalParam& param)
    {
        allowed_distance = MM2INT(param.allowed_distance);
        transitioning_angle = param.transitioning_angle;
        discretization_step_size = MM2INT(param.discretization_step_size);
        scaled_spacing_wall_0 = param.scaled_spacing_wall_0;
        scaled_spacing_wall_X = param.scaled_spacing_wall_X;
        wall_transition_length = MM2INT(param.wall_transition_length);
        min_even_wall_line_width = param.min_even_wall_line_width;
        wall_line_width_0 = param.wall_line_width_0;
        wall_split_middle_threshold = param.wall_split_middle_threshold;
        min_odd_wall_line_width = param.min_odd_wall_line_width;
        wall_line_width_x = param.wall_line_width_x;
        wall_add_middle_threshold = param.wall_add_middle_threshold;
        wall_distribution_count = param.wall_distribution_count;
        max_bead_count = param.max_bead_count;
        transition_filter_dist = MM2INT(param.transition_filter_dist);
        allowed_filter_deviation = MM2INT(param.allowed_filter_deviation);
        print_thin_walls = param.print_thin_walls > 0;
        min_feature_size = MM2INT(param.min_feature_size);
        min_bead_width = MM2INT(param.min_bead_width);
        wall_0_inset = MM2INT(param.wall_0_inset);

        wall_inset_count = param.wall_inset_count;
        stitch_distance = MM2INT(param.stitch_distance);
        max_resolution = MM2INT(param.max_resolution);
        max_deviation = MM2INT(param.max_deviation);
        max_area_deviation = MM2INT(param.max_area_deviation);
    }
}
