#include "conv.h"
#include "utils/Coord_t.h"

namespace crslice
{
	void convert(const ClipperLib::Paths& paths, std::vector<trimesh::vec3>& lines, float z, bool loop)
	{
		for (const ClipperLib::Path& path : paths)
			convert(path, lines, z, loop, true);
	}

	void convert(const ClipperLib::Path& path, std::vector<trimesh::vec3>& lines, float z, bool loop, bool append)
	{
		if (!append)
			lines.clear();

		size_t size = path.size();
		if (size <= 1)
			return;

		size_t end = loop ? size : size - 1;
		for (size_t i = 0; i < end; ++i)
		{
			const ClipperLib::IntPoint& point = path.at(i);
			const ClipperLib::IntPoint& point1 = path.at((i + 1) % size);
			lines.push_back(trimesh::vec3(FDM_I2F(point.X), FDM_I2F(point.Y), z));
			lines.push_back(trimesh::vec3(FDM_I2F(point1.X), FDM_I2F(point1.Y), z));
		}
	}

	trimesh::vec3 convert(const ClipperLib::IntPoint& point, float z)
	{
		return trimesh::vec3(FDM_I2F(point.X), FDM_I2F(point.Y), z);
	}

	trimesh::vec3 convert(const cura52::Point3& point)
	{
		return trimesh::vec3(FDM_I2F(point.x), FDM_I2F(point.y), FDM_I2F(point.z));
	}

	void convertRaw(const ClipperLib::Paths& paths, std::vector<std::vector<trimesh::vec3>>& polygons, float z)
	{
		int size = (int)paths.size();
		if (size == 0)
			return;

		polygons.resize(size);
		for (int i = 0; i < size; ++i)
			convert(paths.at(i), polygons.at(i), z);
	}
}