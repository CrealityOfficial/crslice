#ifndef CRSLICE_LOAD_1698397403190_H
#define CRSLICE_LOAD_1698397403190_H
#include "crslice/interface.h"
#include "crslice/header.h"

typedef std::vector<trimesh::vec3> CrPolygon;
typedef std::vector<CrPolygon> CrPolygons;

namespace crslice
{
	struct CrSkinPart
	{
		CrPolygons outline;
		std::vector<CrPolygons> inset_paths;
		CrPolygons skin_fill;
		CrPolygons roofing_fill;
		CrPolygons top_most_surface_fill;
		CrPolygons bottom_most_surface_fill;
	};

	struct CrSliceLayerPart
	{
		CrPolygons outline;
		CrPolygons inner_area;
		std::vector<CrSkinPart> skin_parts;
		std::vector<CrPolygons> wall_toolpaths;
		std::vector<CrPolygons> infill_wall_toolpaths;
	};

	struct CrSliceLayer
	{
		std::vector<CrSliceLayerPart> parts;
	};

	CRSLICE_API void _load(std::fstream& in, CrPolygon& poly);
	CRSLICE_API void _save(std::fstream& out, const CrPolygon& poly);
	CRSLICE_API void _load(std::fstream& in, CrPolygons& polys);
	CRSLICE_API void _save(std::fstream& out, const CrPolygons& polys);
	CRSLICE_API void _load(std::fstream& in, CrSliceLayerPart& part);
	CRSLICE_API void _save(std::fstream& out, const CrSliceLayerPart& part);
	CRSLICE_API void _load(std::fstream& in, CrSkinPart& skin);
	CRSLICE_API void _save(std::fstream& out, const CrSkinPart& skin);
}

#include "ccglobal/serial.h"   // for _load _save

namespace crslice
{
	class CRSLICE_API SerailSlicedLayer : public ccglobal::Serializeable
	{
	public:
		SerailSlicedLayer() {}
		virtual ~SerailSlicedLayer() {}

		int version() override;
		bool save(std::fstream& out, ccglobal::Tracer* tracer) override;
		bool load(std::fstream& in, int ver, ccglobal::Tracer* tracer) override;

		float z = 0;
		CrPolygons polygons;
		CrPolygons open_polygons;
	};

	class CRSLICE_API SerialWalls : public ccglobal::Serializeable
	{
	public:
		SerialWalls() {}
		virtual ~SerialWalls() {}

		int version() override;
		bool save(std::fstream& out, ccglobal::Tracer* tracer) override;
		bool load(std::fstream& in, int ver, ccglobal::Tracer* tracer) override;

		CrPolygons print_outline;
		CrPolygons inner_area;
		std::vector<CrPolygons> walls;
	};

	class CRSLICE_API SerailPolygons : public ccglobal::Serializeable
	{
	public:
		SerailPolygons() {}
		virtual ~SerailPolygons() {}

		int version() override;
		bool save(std::fstream& out, ccglobal::Tracer* tracer) override;
		bool load(std::fstream& in, int ver, ccglobal::Tracer* tracer) override;

		CrPolygons polygons;
	};

	class CRSLICE_API SerailCrSliceLayer : public ccglobal::Serializeable
	{
	public:
		SerailCrSliceLayer() {}
		virtual ~SerailCrSliceLayer() {}

		int version() override;
		bool save(std::fstream& out, ccglobal::Tracer* tracer) override;
		bool load(std::fstream& in, int ver, ccglobal::Tracer* tracer) override;

		CrSliceLayer layer;
	};

	///file name
	CRSLICE_API std::string sliced_layer_name(const std::string& root, int meshId, int layer);
	CRSLICE_API std::string processed_sliced_layer_name(const std::string& root, int meshId, int layer);
	CRSLICE_API std::string mesh_layer_part_name(const std::string& root, int meshId, int layer, int part);
	CRSLICE_API std::string mesh_layer_part_wall_name(const std::string& root, int meshId, int layer, int part);

	CRSLICE_API std::string crslicelayer_name(const std::string& root, int meshId, int layer);
}

#endif // CRSLICE_LOAD_1698397403190_H