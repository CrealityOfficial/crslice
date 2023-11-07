#ifndef CRSLICE_TEST_SKELETAL_1698397403190_H
#define CRSLICE_TEST_SKELETAL_1698397403190_H
#include "crslice/test/test_voronoi.h"

#include "src/conv.h"

#include "tools/BoostInterface.h"
#include "utils/PolygonsSegmentIndex.h"
#include "voronoi_visual_utils.hpp"
#include <fstream>

namespace crslice
{
    void testVoronoi(const CrPolygons & polys, VoronoiData & data)
    {
        cura52::Polygons input;
        crslice::convertPolygonRaw(polys, input);

        typedef cura52::PolygonsSegmentIndex Segment;

        std::vector<Segment> segments;
        for (size_t poly_idx = 0; poly_idx < input.size(); poly_idx++)
        {
            for (size_t point_idx = 0; point_idx < input[poly_idx].size(); point_idx++)
            {
                segments.emplace_back(&input, poly_idx, point_idx);
            }
        }
    
        boost::polygon::voronoi_diagram<double> vonoroi_diagram;
        boost::polygon::construct_voronoi(segments.begin(), segments.end(), &vonoroi_diagram);

        const std::vector<boost::polygon::voronoi_cell<double>>& cells = vonoroi_diagram.cells();
        const std::vector<boost::polygon::voronoi_vertex<double>>& vertexes = vonoroi_diagram.vertices();
        const std::vector<boost::polygon::voronoi_edge<double>>& edges = vonoroi_diagram.edges();

        typedef boost::polygon::voronoi_cell<double> cell_type;
        typedef boost::polygon::voronoi_vertex<double> vertex_type;
        typedef boost::polygon::voronoi_edge<double> edge_type;
        typedef boost::polygon::segment_traits<CSegment>::point_type point_type;
        boost::polygon::point_data;
        boost::polygon::segment_data;

        auto f = [](const vertex_type& vertex)->trimesh::vec3 {
            return trimesh::vec3(vertex.x() / 1000.0f, vertex.y() / 1000.0f, 0.0f);
        };

        auto retrieve_point = [&segments](const cell_type& cell)->point_type {
            cell_type::source_index_type index = cell.source_index();
            cell_type::source_category_type category = cell.source_category();

            if (category == boost::polygon::SOURCE_CATEGORY_SEGMENT_START_POINT) {
                return boost::polygon::low(segments[index]);
            }
            else {
                return boost::polygon::high(segments[index]);
            }
            return point_type();
        };

        auto retrieve_segment = [&segments](const cell_type& cell)->Segment {
            cell_type::source_index_type index = cell.source_index();
            return segments[index];
        };

        for (const vertex_type& vertex : vertexes)
            data.vertexes.push_back(f(vertex));

        for (const edge_type& edge : edges)
        {
            if (!edge.is_finite())
            {
                //const cell_type& cell1 = *edge.cell();
                //const cell_type& cell2 = *edge.twin()->cell();
                //point_type origin, direction;
                //// Infinite edges could not be created by two segment sites.
                //if (cell1.contains_point() && cell2.contains_point()) {
                //    point_type p1 = retrieve_point(cell1);
                //    point_type p2 = retrieve_point(cell2);
                //    origin.X = (p1.X + p2.X) * 0.5;
                //    origin.Y = (p1.Y + p2.Y) * 0.5;
                //    direction.X = p1.Y - p2.Y;
                //    direction.Y = p2.X - p1.X;
                //}
                //else {
                //    origin = cell1.contains_segment() ?
                //        retrieve_point(cell2) :
                //        retrieve_point(cell1);
                //    Segment segment = cell1.contains_segment() ?
                //        retrieve_segment(cell1) :
                //        retrieve_segment(cell2);
                //    double dx = boost::polygon::high(segment).X - boost::polygon::low(segment).X;
                //    double dy = boost::polygon::high(segment).Y - boost::polygon::low(segment).Y;
                //    if ((boost::polygon::low(segment) == origin) ^ cell1.contains_point()) {
                //        direction.X = dy;
                //        direction.Y = -dx;
                //    }
                //    else {
                //        direction.X = -dy;
                //        direction.Y = dx;
                //    }
                //}
                //coordinate_type side = xh(brect_) - xl(brect_);
                //coordinate_type koef =
                //    side / (std::max)(fabs(direction.x()), fabs(direction.y()));
                //if (edge.vertex0() == NULL) {
                //    clipped_edge->push_back(point_type(
                //        origin.x() - direction.x() * koef,
                //        origin.y() - direction.y() * koef));
                //}
                //else {
                //    clipped_edge->push_back(
                //        point_type(edge.vertex0()->x(), edge.vertex0()->y()));
                //}
                //if (edge.vertex1() == NULL) {
                //    clipped_edge->push_back(point_type(
                //        origin.x() + direction.x() * koef,
                //        origin.y() + direction.y() * koef));
                //}
                //else {
                //    clipped_edge->push_back(
                //        point_type(edge.vertex1()->x(), edge.vertex1()->y()));
                //}
            }
            else {
                if(edge.is_curved()) {
                    double max_dist = 0.1;
                    std::vector<trimesh::dvec3> sampled_edge;
                    point_type point = edge.cell()->contains_point() ?
                        retrieve_point(*edge.cell()) :
                        retrieve_point(*edge.twin()->cell());
                    Segment segment = edge.cell()->contains_point() ?
                        retrieve_segment(*edge.twin()->cell()) :
                        retrieve_segment(*edge.cell());
                    //boost::polygon::voronoi_visual_utils<double>::discretize(
                    //    point, segment, max_dist, &sampled_edge);
                }
                else {
                    data.edges.push_back(f(*edge.vertex0()));
                    data.edges.push_back(f(*edge.vertex1()));
                }
            }
        }

        for (const boost::polygon::voronoi_cell<double>& cell : cells)
        {
            if (cell.is_degenerate() || !cell.contains_segment())
                continue;

            const boost::polygon::voronoi_edge<double>* edge = cell.incident_edge();
            const boost::polygon::voronoi_edge<double>* start = edge;
            do {
                if (edge->is_finite())
                {
                    data.cells.push_back(f(*edge->vertex0()));
                    data.cells.push_back(f(*edge->vertex1()));
                }
                edge = edge->next();
            } while (edge != start && edge);
        }
    }

    void saveVoronoi(const CrPolygons& polys, const std::string& fileName)
    {
        cura52::Polygons input;
        crslice::convertPolygonRaw(polys, input);

        typedef cura52::PolygonsSegmentIndex Segment;

        std::vector<Segment> segments;
        for (size_t poly_idx = 0; poly_idx < input.size(); poly_idx++)
        {
            for (size_t point_idx = 0; point_idx < input[poly_idx].size(); point_idx++)
            {
                segments.emplace_back(&input, poly_idx, point_idx);
            }
        }

        std::fstream out;
        out.open(fileName, std::ios::out);
        if (out.is_open())
        {
            out << 0 << "\n";
            out << (int)segments.size() << "\n";
            for (std::size_t i = 0; i < segments.size(); ++i) {
                const Segment& seg = segments.at(i);
                cura52::Point f = seg.from();
                cura52::Point t = seg.to();

                out << f.X << " "
                    << f.Y << " "
                    << t.X << " "
                    << t.Y << "\n";
            }
        }
        out.close();
    }
}

#endif // CRSLICE_TEST_SKELETAL_1698397403190_H