#ifndef CX_PRESLICE_1600132451200_H
#define CX_PRESLICE_1600132451200_H
#include "cxutil/math/Coord_t.h"
#include "cxutil/input/sceneinput.h"
#include "cxutil/input/dlpinput.h"

namespace cxutil
{
	void buildSliceInfos(GroupInput* meshGroup, std::vector<int>& z, std::vector<coord_t>& printZ, std::vector<coord_t>& thicknesses);
	void buildSliceInfos(DLPInput* input, std::vector<int>& z);
}

#endif // CX_PRESLICE_1600132451200_H