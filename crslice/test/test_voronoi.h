#ifndef CRSLICE_TEST_SKELETAL_VORONOI_1698397403190_H
#define CRSLICE_TEST_SKELETAL_VORONOI_1698397403190_H
#include "crslice/load.h"

namespace crslice
{
	struct VCell
	{

	};

	struct VVertex
	{

	};

	struct VEdge
	{

	};

	struct VoronoiData
	{
		std::vector<VCell> cells;
		std::vector<VEdge> edges;
		std::vector<VVertex> vertexes;
	};

	CRSLICE_API void testVoronoi(const CrPolygons& polys, VoronoiData& data);
}

#endif // CRSLICE_TEST_SKELETAL_VORONOI_1698397403190_H