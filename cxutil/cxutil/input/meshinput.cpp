#include "cxutil/input/meshinput.h"

namespace cxutil
{
	MeshInput::MeshInput()
	{
		m_settings = new Settings();
		m_param = new MeshParam();
	}

	MeshInput::~MeshInput()
	{
		delete m_settings;
		delete m_param;
	}

	void MeshInput::setMeshObject(MeshObjectPtr object)
	{
		m_mesh = object;
	}

	MeshObject* MeshInput::object()  // don't store it
	{
		return &*m_mesh;
	}

	Settings* MeshInput::settings()
	{
		return m_settings;
	}

	MeshParam* MeshInput::param()
	{
		return m_param;
	}

	void MeshInput::finalize()
	{
		m_param->initialize(this);
	}

	AABB3D MeshInput::box()
	{
		return m_mesh->box();
	}
}