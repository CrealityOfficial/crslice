#include "skeletalimpl.h"

namespace crslice
{
    void SkeletalCheckImpl::classifyEdge()
    {
        discretize_edges.clear();
        discretize_cells.clear();

        for (cell_type cell : graph.cells())
        {
            if (!cell.incident_edge())
            { // There is no spoon
                continue;
            }

            DiscretizeCell DC;

            Point start_source_point;
            Point end_source_point;
            edge_type* starting_vonoroi_edge = nullptr;
            edge_type* ending_vonoroi_edge = nullptr;
            // Compute and store result in above variables

            if (cell.contains_point())
            {
                const bool keep_going = VoronoiUtils::computePointCellRange(cell, start_source_point, end_source_point, starting_vonoroi_edge, ending_vonoroi_edge, points, segments);
                if (!keep_going)
                {
                    continue;
                }
            }
            else
            {
                VoronoiUtils::computeSegmentCellRange(cell, start_source_point, end_source_point, starting_vonoroi_edge, ending_vonoroi_edge, points, segments);
            }

            DC.ending_vonoroi_edge = ending_vonoroi_edge;
            DC.end_source_point = end_source_point;
            DC.starting_vonoroi_edge = starting_vonoroi_edge;
            DC.start_source_point = start_source_point;
            discretize_cells.push_back(DC);

            auto f = [this](edge_type* edge) {
                assert(edge->is_finite());

                DiscretizeEdge DE;
                DE.type = EdgeDiscretizeType::edt_edge;
                DE.edge = edge;

                const cell_type* left_cell = edge->cell();
                const cell_type* right_cell = edge->twin()->cell();

                bool point_left = left_cell->contains_point();
                bool point_right = right_cell->contains_point();
                if ((!point_left && !point_right) || edge->is_secondary())
                {
                    DE.type = EdgeDiscretizeType::edt_edge;
                }
                else if (point_left != point_right)
                {
                    DE.type = EdgeDiscretizeType::edt_parabola;
                }
                else
                {
                    DE.type = EdgeDiscretizeType::edt_straightedge;
                }

                discretize_edges.push_back(DE);
            };

            for (edge_type* edge = starting_vonoroi_edge; edge != ending_vonoroi_edge; edge = edge->next())
                f(edge);
            f(ending_vonoroi_edge);
        }
    }

    void SkeletalCheckImpl::transferEdges(CrDiscretizeEdges& discretizeEdges)
    {
        const std::vector<edge_type>& edges = graph.edges();
        for (const DiscretizeEdge& DE : discretize_edges)
        {
            edge_type* edge = DE.edge;
            trimesh::vec3 p0 = convert(VoronoiUtils::p(edge->vertex0()));
            trimesh::vec3 p1 = convert(VoronoiUtils::p(edge->vertex1()));

            if (DE.type == EdgeDiscretizeType::edt_edge)
            {
                discretizeEdges.edges.push_back(p0);
                discretizeEdges.edges.push_back(p1);
            }
            else if (DE.type == EdgeDiscretizeType::edt_parabola)
            {
                discretizeEdges.parabola.push_back(p0);
                discretizeEdges.parabola.push_back(p1);
            }
            else if (DE.type == EdgeDiscretizeType::edt_straightedge)
            {
                discretizeEdges.straightedges.push_back(p0);
                discretizeEdges.straightedges.push_back(p1);
            }
        }
    }

    bool SkeletalCheckImpl::transferCell(int index, CrDiscretizeCell& discretizeCell)
    {
        if (index >= 0 && index < discretize_cells.size())
        {
            const DiscretizeCell& cell = discretize_cells.at(index);

            discretizeCell.start = convert(cell.start_source_point);
            discretizeCell.end = convert(cell.end_source_point);

            auto f = [&discretizeCell](edge_type* edge) {
                trimesh::vec3 p0 = convert(VoronoiUtils::p(edge->vertex0()));
                trimesh::vec3 p1 = convert(VoronoiUtils::p(edge->vertex1()));
                discretizeCell.edges.push_back(p0);
                discretizeCell.edges.push_back(p1);
            };

            for (edge_type* edge = cell.starting_vonoroi_edge; edge != cell.ending_vonoroi_edge; edge = edge->next()){
                f(edge);
            }
            f(cell.ending_vonoroi_edge);

            return true;
        }

        return false;
    }
}
