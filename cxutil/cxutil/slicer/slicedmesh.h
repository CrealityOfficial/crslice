#ifndef CX_SLICEDMESH_1599726180169_H
#define CX_SLICEDMESH_1599726180169_H
#include <vector>
#include "cxutil/math/polygon.h"

namespace cxutil
{
	class SlicedMeshLayer
	{
	public:
		SlicedMeshLayer();
		~SlicedMeshLayer();

		Polygons polygons;
		Polygons openPolylines;
	};

	class SlicedMesh
	{
	public:
		SlicedMesh();
		~SlicedMesh();

		std::vector<SlicedMeshLayer> m_layers;
	};
}

#endif // CX_SLICEDMESH_1599726180169_H