#ifndef CRSLICE_TEST_SKELETAL_VORONOI_1698397403190_H
#define CRSLICE_TEST_SKELETAL_VORONOI_1698397403190_H
#include "crslice/load.h"

namespace crslice
{
	struct VoronoiData
	{
		std::vector<trimesh::vec3> vertexes;
		std::vector<trimesh::vec3> edges;
		std::vector<trimesh::vec3> cells;
	};

	CRSLICE_API void testVoronoi(const CrPolygons& polys, VoronoiData& data);

	CRSLICE_API void saveVoronoi(const CrPolygons& polys, const std::string& fileName);
}

#endif // CRSLICE_TEST_SKELETAL_VORONOI_1698397403190_H