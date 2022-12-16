#include "cxutil/input/groupinput.h"
#include "cxutil/util/meshbuilder.h"

namespace cxutil
{
	GroupInput::GroupInput()
	{
		m_setting = new Settings();
		m_param = new MeshGroupParam();
	}

	GroupInput::~GroupInput()
	{
		delete m_setting;
		delete m_param;
	}

	void GroupInput::addMeshInput(MeshInputPtr mesh)
	{
		m_meshInputs.push_back(mesh);
	}

	void GroupInput::removeMeshInput(MeshInputPtr mesh) 
	{

	}

	void GroupInput::loadMesh(const std::string& fileName)
	{
		MeshObjectPtr mesh(loadSTLBinaryMesh(fileName.c_str()));
		if (mesh)
		{
			MeshInputPtr meshInput(new MeshInput());
			meshInput->setMeshObject(mesh);

			addMeshInput(meshInput);
		}
	}

	const std::vector<MeshInputPtr>& GroupInput::meshes() const
	{
		return m_meshInputs;
	}

	Settings* GroupInput::settings()
	{
		return m_setting;
	}

	MeshGroupParam* GroupInput::param()
	{
		return m_param;
	}

	AABB3D GroupInput::box()
	{
		return m_box;
	}

	void GroupInput::release()
	{

	}

	void GroupInput::finalize()
	{
		for (int i=0;i< m_meshInputs.size();i++)
		{
			Settings* meshSetting = m_meshInputs[i]->settings();
			meshSetting->setParent(m_setting);

			m_meshInputs[i]->finalize();
			m_param->initialize(this);

			AABB3D b = m_meshInputs[i]->box();
			m_box.include(b);
		}
	}

	void GroupInput::releaseMeshData()
	{
		for (int i = 0; i < m_meshInputs.size(); i++)
		{
			m_meshInputs[i]->object()->clear();
		}
	}
}