#ifndef CRSLICE_CLIPPERUTIL_CONV1_1682319629911_H
#define CRSLICE_CLIPPERUTIL_CONV1_1682319629911_H
#include "libslic3r/ExPolygon.hpp"
#include "ccglobal/debugger.h"

namespace crslice2
{
	trimesh::vec3 convert(const Slic3r::Point& point, float z = 0.0f);
	ccglobal::Polygon convert(const Slic3r::Polygon& in, float z = 0.0f);
}

#endif // CRSLICE_CLIPPERUTIL_CONV1_1682319629911_H