#include "crslice/crscene.h"

#include "crgroup.h"
#include "crobject.h"

#include "ccglobal/log.h"

namespace crslice
{
	CrScene::CrScene()
	{
		m_settings.reset(new crcommon::Settings());
	}

	CrScene::~CrScene()
	{
		release();
	}

	int CrScene::addOneGroup()
	{
		int groupID = (int)m_groups.size();
		CrGroup* group = new CrGroup();
		m_groups.push_back(group);
		return groupID;
	}

	int CrScene::addObject2Group(int groupID)
	{
		int objectID = -1;
		if (groupID >= 0 && groupID < (int)m_groups.size())
		{
			CrGroup* group = m_groups.at(groupID);
			objectID = group->addObject();
		}
		return objectID;
	}

	void CrScene::setOjbectMesh(int groupID, int objectID, TriMeshPtr mesh)
	{
		if (groupID < 0 || groupID >= (int)m_groups.size())
		{
			LOGE("CrScene::setOjbectMesh [%d] not exist.", groupID);
			return;
		}

		CrGroup* group = m_groups.at(groupID);
		group->setObjectMesh(objectID, mesh);
	}

	void CrScene::setObjectSettings(int groupID, int objectID, SettingsPtr settings)
	{
		if (groupID < 0 || groupID >= (int)m_groups.size())
		{
			LOGE("CrScene::setOjbectMesh [%d] not exist.", groupID);
			return;
		}

		CrGroup* group = m_groups.at(groupID);
		group->setObjectSettings(objectID, settings);
	}

	void CrScene::setGroupSettings(int groupID, SettingsPtr settings)
	{
		if (groupID < 0 || groupID >= (int)m_groups.size())
		{
			LOGE("CrScene::setOjbectMesh [%d] not exist.", groupID); 
			return;
		}

		CrGroup* group = m_groups.at(groupID);
		group->setSettings(settings);
	}

	void CrScene::setSceneSettings(SettingsPtr settings)
	{
		m_settings = settings;
	}

	void CrScene::release()
	{
		for (CrGroup* group : m_groups)
			delete group;
		m_groups.clear();
	}
	CrGroup* CrScene::getGroupsIndex(int groupID)
	{
		if (groupID < m_groups.size())
			return  m_groups.at(groupID);
		else
			return nullptr;
	}
}