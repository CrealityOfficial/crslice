#ifndef CRSLICE_LOAD_1698397403190_H
#define CRSLICE_LOAD_1698397403190_H
#include "crslice/interface.h"
#include "crslice/header.h"
#include "ccglobal/serial.h"

typedef std::vector<trimesh::vec3> CrPolygon;
typedef std::vector<CrPolygon> CrPolygons;

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

		int z = 0;
		CrPolygons polygon;
		CrPolygons open_polygon;
	};



	///file name
	CRSLICE_API std::string sliced_layer_name(const std::string& root, int meshId, int layer);
}

#endif // CRSLICE_LOAD_1698397403190_H