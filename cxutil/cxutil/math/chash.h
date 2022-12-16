//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef CX_UTILS_HASH_COORD_T_H
#define CX_UTILS_HASH_COORD_T_H


//Include Clipper to get the ClipperLib::IntPoint definition, which we reuse as Point definition.
#include "polyclipping/clipper.hpp"
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

namespace std {
	template <>
	struct hash<ClipperLib::IntPoint> {
		size_t operator()(const ClipperLib::IntPoint& pp) const
		{
			static int prime = 31;
			int result = 89;
			result = result * prime + pp.X;
			result = result * prime + pp.Y;
			return result;
		}
	};
}

typedef std::function<void(ClipperLib::PolyNode*)> polyNodeFunc;
typedef std::function<void(std::vector<ClipperLib::PolyNode*>&)> polyLevelFunc;


#endif // CX_UTILS_HASH_COORD_T_H
