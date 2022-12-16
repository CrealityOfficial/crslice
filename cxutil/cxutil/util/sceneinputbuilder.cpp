#include "cxutil/util/sceneinputbuilder.h"
#include "cxutil/input/sceneinput.h"

#include "cxutil/util/jsonsettingsloader.h"
#include "cxutil/util/meshbuilder.h"

#include "cxutil/util/cmdparsor.h"

namespace cxutil
{
	SceneInputBuilder::SceneInputBuilder()
		:m_input(nullptr)
		, m_currentGroup(nullptr)
	{
	}

	SceneInputBuilder::~SceneInputBuilder()
	{

	}

	SceneInput* SceneInputBuilder::create(int argc, char* argv[])
	{
		CmdParsor parsor(argc, argv);

		parsor.registerFunc("mesh", std::bind(&SceneInputBuilder::loadMesh, this, std::placeholders::_1));
		parsor.registerFunc("gsetting", std::bind(&SceneInputBuilder::loadGlobalSettings, this, std::placeholders::_1));
		parsor.process();

		return m_input;
	}

	void SceneInputBuilder::createSceneInput()
	{
		if (!m_input)
		{
			m_input = new SceneInput();
			m_input->addExtruderInput(cxutil::ExtruderInputPtr(new ExtruderInput()));
		}
	}

	void SceneInputBuilder::loadGlobalSettings(const std::string& settingFile)
	{
		createSceneInput();
		loadJsonSetting(settingFile.c_str(), m_input->settings());
	}

	void SceneInputBuilder::loadMesh(const std::string& fileName)
	{
		MeshObjectPtr mesh(loadSTLBinaryMesh(fileName.c_str()));
		if (mesh)
		{
			createSceneInput();

			MeshInputPtr meshInput(new MeshInput());
			meshInput->setMeshObject(mesh);

			if (!m_currentGroup) addGroup("");
			m_currentGroup->addMeshInput(meshInput);
		}
	}

	void SceneInputBuilder::addGroup(const std::string&)
	{
		createSceneInput();

		GroupInputPtr groupInput(new GroupInput());
		m_input->addGroupInput(groupInput);

		m_currentGroup = &*groupInput;
	}
}