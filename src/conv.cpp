#include "conv.h"

namespace crslice2
{
	trimesh::vec3 convert(const Slic3r::Point& point, float z)
	{
		return trimesh::vec3(unscale_(point.x()), unscale_(point.y()), z);
	}

	ccglobal::Polygon convert(const Slic3r::Polygon& in, float z)
	{
		ccglobal::Polygon poly;
		for (const Slic3r::Point& point : in.points)
			poly.emplace_back(convert(point, z));
		return poly;
	}
}