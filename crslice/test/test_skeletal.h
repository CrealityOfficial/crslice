#ifndef CRSLICE_TEST_SKELETAL_1698397403190_H
#define CRSLICE_TEST_SKELETAL_1698397403190_H
#include "crslice/load.h"

namespace crslice
{
	struct SkeletalNode
	{
		trimesh::vec3 p;
		float r;
	};

	struct SkeletalEdge
	{
		trimesh::vec3 from;
		trimesh::vec3 to;
	};

	struct SkeletalGraph
	{
		std::vector<SkeletalEdge> edges;
		std::vector<SkeletalNode> nodes;
	};

	struct SkeletalDetail
	{
		SkeletalGraph graph;
	};

	CRSLICE_API void testSkeletal(const SerailCrSkeletal& skeletal, CrPolygons& innerPoly, std::vector<CrVariableLines>& out,
		SkeletalDetail* detail = nullptr);

	struct ParabolaDetal
	{
		std::vector<trimesh::vec3> points;
	};

	CRSLICE_API void testDiscretizeParabola(CrPolygon& points, ParabolaDetal& detail);
}

#endif // CRSLICE_TEST_SKELETAL_1698397403190_H