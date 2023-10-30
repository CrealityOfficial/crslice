#include "crslice/load.h"

namespace crslice
{
	void _load(std::fstream& in, CrPolygon& poly)
	{
		ccglobal::cxndLoadVectorT(in, poly);
	}

	void _save(std::fstream& out, const CrPolygon& poly)
	{
		ccglobal::cxndSaveVectorT(out, poly);
	}

	void _load(std::fstream& in, CrPolygons& polys)
	{
		int size = 0;
		ccglobal::cxndLoadT(in, size);
		if (size > 0)
		{
			polys.resize(size);
			for(int i = 0; i < size; ++i)
				_load(in, polys.at(i));
		}
	}

	void _save(std::fstream& out, const CrPolygons& polys)
	{
		int size = (int)polys.size();
		ccglobal::cxndSaveT(out, size);
		for (int i = 0; i < size; ++i)
			_save(out, polys.at(i));
	}

	void _load(std::fstream& in, CrSliceLayerPart& part)
	{
		_load(in, part.outline);
		_load(in, part.inner_area);
		cxndLoadComplexVectorT(in, part.skin_parts);
		cxndLoadComplexVectorT(in, part.wall_toolpaths);
		cxndLoadComplexVectorT(in, part.infill_wall_toolpaths);
	}

	void _save(std::fstream& out, const CrSliceLayerPart& part)
	{
		_save(out, part.outline);
		_save(out, part.inner_area);
		cxndSaveComplexVectorT(out, part.skin_parts);
		cxndSaveComplexVectorT(out, part.wall_toolpaths);
		cxndSaveComplexVectorT(out, part.infill_wall_toolpaths);
	}

	void _load(std::fstream& in, CrSkinPart& skin)
	{
		_load(in, skin.outline);
		cxndLoadComplexVectorT(in, skin.inset_paths);
		_load(in, skin.skin_fill);
		_load(in, skin.roofing_fill);
		_load(in, skin.top_most_surface_fill);
		_load(in, skin.bottom_most_surface_fill);
	}

	void _save(std::fstream& out, const CrSkinPart& skin)
	{
		_save(out, skin.outline);
		cxndSaveComplexVectorT(out, skin.inset_paths);
		_save(out, skin.skin_fill);
		_save(out, skin.roofing_fill);
		_save(out, skin.top_most_surface_fill);
		_save(out, skin.bottom_most_surface_fill);
	}

	int SerailSlicedLayer::version()
	{
		return 0;
	}

	bool SerailSlicedLayer::save(std::fstream& out, ccglobal::Tracer* tracer)
	{
		ccglobal::cxndSaveT(out, z);
		_save(out, open_polygons);
		_save(out, polygons);

		return true;
	}

	bool SerailSlicedLayer::load(std::fstream& in, int ver, ccglobal::Tracer* tracer)
	{
		if (ver == 0)
		{
			ccglobal::cxndLoadT(in, z);
			_load(in, open_polygons);
			_load(in, polygons);

			return true;
		}
		return false;
	}

	int SerailPolygons::version()
	{
		return 0;
	}

	bool SerailPolygons::save(std::fstream& out, ccglobal::Tracer* tracer)
	{
		_save(out, polygons);

		return true;
	}

	bool SerailPolygons::load(std::fstream& in, int ver, ccglobal::Tracer* tracer)
	{
		if (ver == 0)
		{
			_load(in, polygons);

			return true;
		}
		return false;
	}

	int SerialWalls::version()
	{
		return 0;
	}

	bool SerialWalls::save(std::fstream& out, ccglobal::Tracer* tracer)
	{
		_save(out, print_outline);
		_save(out, inner_area);
		int wallCount = (int)walls.size();
		ccglobal::cxndSaveT(out, wallCount);
		for (int i = 0; i < wallCount; ++i)
			_save(out, walls.at(i));
		return true;
	}

	bool SerialWalls::load(std::fstream& in, int ver, ccglobal::Tracer* tracer)
	{
		if (ver == 0)
		{
			_load(in, print_outline);
			_load(in, inner_area);
			int wallCount = 0;
			ccglobal::cxndLoadT(in, wallCount);
			if (wallCount > 0)
			{
				walls.resize(wallCount);
				for (int i = 0; i < wallCount; ++i)
					_load(in, walls.at(i));
			}
			return true;
		}

		return false;
	}

	int SerailCrSliceLayer::version()
	{
		return 0;
	}

	bool SerailCrSliceLayer::save(std::fstream& out, ccglobal::Tracer* tracer)
	{
		cxndSaveComplexVectorT(out, layer.parts);
		return true;
	}

	bool SerailCrSliceLayer::load(std::fstream& in, int ver, ccglobal::Tracer* tracer)
	{
		if (ver == 0)
		{
			cxndLoadComplexVectorT(in, layer.parts);
			return true;
		}

		return false;
	}

	/// file name
	std::string sliced_layer_name(const std::string& root, int meshId, int layer)
	{
		char buffer[1024] = { 0 };

		sprintf(buffer, "%s/mesh%d_layer%d.sliced_layer", root.c_str(), meshId, layer);
		return std::string(buffer);
	}

	std::string processed_sliced_layer_name(const std::string& root, int meshId, int layer)
	{
		char buffer[1024] = { 0 };

		sprintf(buffer, "%s/mesh%d_layer%d.processed_sliced_layer", root.c_str(), meshId, layer);
		return std::string(buffer);
	}

	std::string mesh_layer_part_name(const std::string& root, int meshId, int layer, int part)
	{
		char buffer[1024] = { 0 };

		sprintf(buffer, "%s/mesh%d_layer%d_part%d.layer_part", root.c_str(), meshId, layer, part);
		return std::string(buffer);
	}

	std::string mesh_layer_part_wall_name(const std::string& root, int meshId, int layer, int part)
	{
		char buffer[1024] = { 0 };

		sprintf(buffer, "%s/mesh%d_layer%d_part%d.walls", root.c_str(), meshId, layer, part);
		return std::string(buffer);
	}

	std::string crslicelayer_name(const std::string& root, int meshId, int layer)
	{
		char buffer[1024] = { 0 };

		sprintf(buffer, "%s/mesh%d_layer%d.all", root.c_str(), meshId, layer);
		return std::string(buffer);
	}
}