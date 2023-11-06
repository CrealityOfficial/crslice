#ifndef CRSLICE_TEST_SKELETAL_1698397403190_H
#define CRSLICE_TEST_SKELETAL_1698397403190_H
#include "crslice/load.h"

namespace crslice
{
	struct SkeletalDetail
	{

	};

	CRSLICE_API void testSkeletal(const SerailCrSkeletal& skeletal, CrPolygons& innerPoly, std::vector<CrVariableLines>& out,
		SkeletalDetail* detail = nullptr);
}

#endif // CRSLICE_TEST_SKELETAL_1698397403190_H