#include "cxutil/input/dlpinput.h"
#include "trimesh2/TriMesh.h"

namespace cxutil
{
	DLPInput::DLPInput()
	{

	}

	DLPInput::~DLPInput()
	{
	}

	void DLPInput::addMeshObject(MeshObjectPtr object)
	{
		m_meshes.push_back(object);
	}

	void DLPInput::addTriMesh(trimesh::TriMesh* Mesh)
	{
		m_meshesSrc.push_back(Mesh);
	}

	std::vector<trimesh::TriMesh*>& DLPInput::getMeshesSrc()
	{
		return m_meshesSrc;
	}

	const std::vector< MeshObjectPtr>& DLPInput::meshes() const
	{
		return m_meshes;
	}

	std::vector<MeshObjectPtr>& DLPInput::meshes()
	{
		return m_meshes;
	}

	AABB3D DLPInput::box()
	{
		AABB3D box;
		for (MeshObjectPtr& ptr : m_meshes)
			box.include(ptr->box());
		return box;
	}
	void DLPInput::addParam(DLPParam param)
	{
		m_param = param;
	}

	DLPParam& DLPInput::param()
	{
		return m_param;
	}
}