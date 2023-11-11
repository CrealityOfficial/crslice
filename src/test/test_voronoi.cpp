#ifndef CRSLICE_TEST_SKELETAL_1698397403190_H
#define CRSLICE_TEST_SKELETAL_1698397403190_H
#include "crslice/test/test_voronoi.h"

#include "src/conv.h"

#include "tools/BoostInterface.h"
#include "utils/PolygonsSegmentIndex.h"
#include "utils/VoronoiUtils.h"
#include "utils/linearAlg2D.h"
#include "voronoi_visual_utils.hpp"
#include "tools/SVG.h"
#include <fstream>

using namespace cura52;
namespace crslice
{
    typedef ClipperLib::IntPoint Point;
    typedef cura52::PolygonsSegmentIndex Segment;
    typedef boost::polygon::voronoi_cell<double> cell_type;
    typedef boost::polygon::voronoi_vertex<double> vertex_type;
    typedef boost::polygon::voronoi_edge<double> edge_type;
    typedef boost::polygon::segment_traits<CSegment>::point_type point_type;
    typedef boost::polygon::voronoi_diagram<double> voronoi_type;

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
    class VoronoiCheckImpl
    {
    public:
        void setInput(const CrPolygons& polys)
        {
            crslice::convertPolygonRaw(polys, input);
            constructSegments(input, segments);
            boost::polygon::construct_voronoi(segments.begin(), segments.end(), &graph);

            for (const ClipperLib::Path& path : input.paths)
                for (const Point& p : path)
                    box.include(p);

            box.expand(100);
        }

        void saveSVG(const std::string& fileName)
        {
            SVG out(fileName, box);
            out.setFlipY(true);
            ClipperLib::cInt o = (double)vSize(box.max - box.min) / 1000.0;

            for (const vertex_type& vertex : graph.vertices())
            {
                out.writePoint(VoronoiUtils::p(&vertex), false, 2.0f);
            }

            SVG::ColorObject bColor(SVG::Color::BLUE);
            SVG::ColorObject rColor(SVG::Color::RED);
            SVG::ColorObject gColor(SVG::Color::GREEN);
            SVG::ColorObject color(SVG::Color::BLACK);

            for (const edge_type& edge : graph.edges())
            {
                if (edge.is_infinite())
                    continue;

                const cell_type* left_cell = edge.cell();
                const cell_type* right_cell = edge.twin()->cell();
                Point start = VoronoiUtils::p(edge.vertex0());
                Point end = VoronoiUtils::p(edge.vertex1());

                bool point_left = left_cell->contains_point();
                bool point_right = right_cell->contains_point();
                if ((!point_left && !point_right) || edge.is_secondary())
                {
                    color = gColor;
                }
                else if (point_left != point_right)
                {
                    color = rColor;
                }
                else
                {
                    color = bColor;
                }

                out.writeLine(start, end, color, 2.0f);

                Point s = start;
                Point e = end;
                cura52::halfEdgeOffset(s, e, 100);
                out.writeArrow(s, e, SVG::Color::BLACK, 1.0f, 3.0f);
            }
        }

        cura52::Polygons input;

        AABB box;
        std::vector<Segment> segments;
        std::vector<Point> points;
        voronoi_type graph;
    };

    VoronoiCheck::VoronoiCheck()
        :m_impl(new VoronoiCheckImpl())
    {

    }

    VoronoiCheck::~VoronoiCheck()
    {
        delete m_impl;
    }

    void VoronoiCheck::setInput(const CrPolygons& polys)
    {
        m_impl->setInput(polys);
    }

    void VoronoiCheck::saveSVG(const std::string& fileName)
    {
        m_impl->saveSVG(fileName);
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