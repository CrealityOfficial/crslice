#ifndef CX_MESHSLICE_1599726705111_H
#define CX_MESHSLICE_1599726705111_H
#include <vector>
#include "cxutil/slicer/slicedmesh.h"
#include "cxutil/input/meshobject.h"

namespace trimesh {
	class TriMesh;
}

namespace cxutil
{
	class SliceHelper;
	void sliceMeshes(std::vector<MeshObject*>& meshes, std::vector<SlicedMesh>& slicedMeshes, std::vector<int>& z);
	void sliceMesh(MeshObject* mesh, SlicedMesh& slicedMesh, std::vector<int>& z);
	void sliceMesh(MeshObject* mesh, SlicedMeshLayer& slicedMeshLayer, int z, SliceHelper* helper);
	void sliceMeshes_src(std::vector<trimesh::TriMesh*>& meshes, std::vector<SlicedMesh>& slicedMeshes, std::vector<int>& z);
	void sliceMesh_src(trimesh::TriMesh* mesh, SlicedMesh& slicedMesh, std::vector<int>& z);
	void sliceMesh_src(SlicedMeshLayer& slicedMeshLayer, int z, SliceHelper* helper);

}

#endif // CX_MESHSLICE_1599726705111_H