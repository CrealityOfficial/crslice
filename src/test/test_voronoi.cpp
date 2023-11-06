#ifndef CRSLICE_TEST_SKELETAL_1698397403190_H
#define CRSLICE_TEST_SKELETAL_1698397403190_H
#include "crslice/test/test_voronoi.h"

#include "src/conv.h"

#include "tools/BoostInterface.h"
#include "utils/PolygonsSegmentIndex.h"

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
    }
}

#endif // CRSLICE_TEST_SKELETAL_1698397403190_H