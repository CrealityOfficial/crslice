#ifndef CRSLICE_LOAD_1698397403190_H
#define CRSLICE_LOAD_1698397403190_H
#include "crslice/interface.h"
#include "crslice/header.h"
#include "ccglobal/serial.h"

typedef std::vector<trimesh::vec3> CrPolygon;
typedef std::vector<CrPolygon> CrPolygons;

namespace crslice
{
	CRSLICE_API void _load(std::fstream& in, CrPolygon& poly);
	CRSLICE_API void _save(std::fstream& out, const CrPolygon& poly);
	CRSLICE_API void _load(std::fstream& in, CrPolygons& polys);
	CRSLICE_API void _save(std::fstream& out, const CrPolygons& polys);

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

	///file name
	CRSLICE_API std::string sliced_layer_name(const std::string& root, int meshId, int layer);
	CRSLICE_API std::string processed_sliced_layer_name(const std::string& root, int meshId, int layer);
	CRSLICE_API std::string mesh_layer_part_name(const std::string& root, int meshId, int layer, int part);
	CRSLICE_API std::string mesh_layer_part_wall_name(const std::string& root, int meshId, int layer, int part);
}

#endif // CRSLICE_LOAD_1698397403190_H