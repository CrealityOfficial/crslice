//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef CX_UTILS_COORD_T_H
#define CX_UTILS_COORD_T_H


//Include Clipper to get the ClipperLib::IntPoint definition, which we reuse as Point definition.
#include "polyclipping/clipper.hpp"
#include "cxutil/math/chash.h"
#include <vector>
#include <list>
#include <set>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <ostream>
#include <functional>
#include <queue>
#include <cmath>

typedef std::function<void(ClipperLib::PolyNode*)> polyNodeFunc;
typedef std::function<void(std::vector<ClipperLib::PolyNode*>&)> polyLevelFunc;

namespace cxutil
{

	using coord_t = ClipperLib::cInt;
	#define INT2MM(n) (float(n) / 1000.0f)
	#define INT2MM2(n) (double(n) / 1000000.0)
	#define MM2INT(n) (cxutil::coord_t((n) * 1000 + 0.5 * (((n) > 0) - ((n) < 0))))
	#define MM2_2INT(n) (cxutil::coord_t((n) * 1000000 + 0.5 * (((n) > 0) - ((n) < 0))))
	#define MM3_2INT(n) (cxutil::coord_t((n) * 1000000000 + 0.5 * (((n) > 0) - ((n) < 0))))

	#define INT2MICRON(n) ((n) / 1)
	#define MICRON2INT(n) ((n) * 1)
} // namespace cura


#endif // CX_UTILS_COORD_T_H
