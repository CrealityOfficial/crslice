#ifndef CX_POLYGONTRIMMER_1600307745513_H
#define CX_POLYGONTRIMMER_1600307745513_H
#include "cxutil/slicer/slicedmesh.h"
#include "cxutil/input/param.h"

namespace cxutil
{
	void overhangPrintable(SlicedMesh& slicedMesh, MeshParam* param);

	void carveCuttingMeshes(std::vector<SlicedMesh>& slicedMesh, std::vector<MeshInput*>& meshInputs);

	void carveMultipleVolumes(std::vector<SlicedMesh>& slicedMesh, std::vector<MeshInput*>& meshInputs, bool alternate_carve_order);

	void generateMultipleVolumesOverlap(std::vector<SlicedMesh>& slicedMesh, std::vector<MeshInput*>& meshInputs);
}

#endif // CX_POLYGONTRIMMER_1600307745513_H