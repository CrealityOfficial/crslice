#include "cxutil/input/meshobject.h"

namespace cxutil
{
	MeshObject::MeshObject()
	{

	}

	MeshObject::~MeshObject()
	{

	}

	AABB3D MeshObject::box()
	{
		return m_aabb;
	}

	void MeshObject::calculateBox()
	{
		for (MeshVertex& v : vertices)
			m_aabb.include(v.p);
	}

	void MeshObject::clear()
	{
		vertices.clear();
		faces.clear();
	}
}