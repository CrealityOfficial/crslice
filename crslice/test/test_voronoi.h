#ifndef CRSLICE_TEST_SKELETAL_VORONOI_1698397403190_H
#define CRSLICE_TEST_SKELETAL_VORONOI_1698397403190_H
#include "crslice/load.h"

namespace crslice
{
	CRSLICE_API void saveVoronoi(const CrPolygons& polys, const std::string& fileName);

	class VoronoiCheckImpl;
	class CRSLICE_API VoronoiCheck
	{
	public:
		VoronoiCheck();
		~VoronoiCheck();

		void setInput(const CrPolygons& polys);
		void saveSVG(const std::string& fileName);
	protected:
		VoronoiCheckImpl* m_impl;
	};
}

#endif // CRSLICE_TEST_SKELETAL_VORONOI_1698397403190_H