#include "crslice/crscene.h"

#include "crgroup.h"
#include "crobject.h"
#include "crcommon/jsonloader.h"
#include "ccglobal/log.h"

namespace crslice
{
	CrScene::CrScene()
	{
		m_settings.reset(new crcommon::Settings());
		machine_center_is_zero = false;
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

	void CrScene::setGroupOffset(int groupID, trimesh::vec3 offset)
	{
		if (groupID < 0 || groupID >= (int)m_groups.size())
		{
			LOGE("CrScene::setOjbectMesh [%d] not exist.", groupID);
			return;
		}

		CrGroup* group = m_groups.at(groupID);
		group->setOffset(offset);
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

	void CrScene::setSceneJsonFile(const std::string& fileName)
	{
		std::vector<crcommon::KValues> extruders;
		if (crcommon::loadJSON(fileName, m_settings->settings, extruders) != 0)
		{
			LOGE("setSceneJsonFile invalid json file: %s", fileName.c_str());
			return;
		}

		for (crcommon::KValues& kvs : extruders)
		{
			SettingsPtr settings(new crcommon::Settings());
			settings->settings.swap(kvs);
			m_extruders.push_back(settings);
		}
	}

	void CrScene::setTempDirectory(const std::string& directory)
	{
		m_tempDirectory = directory;
	}

	void CrScene::release()
	{
		for (CrGroup* group : m_groups)
			delete group;
		m_groups.clear();
	}

	void CrScene::setOutputGCodeFileName(const std::string& fileName)
	{
		m_gcodeFileName = fileName;
	}

	void CrScene::setPloygonFileName(const std::string& fileName)
	{
		m_ploygonFileName = fileName;
	}

	bool CrScene::valid()
	{
		return true;
	}

	void CrScene::save(const std::string& fileName)
	{
		std::ofstream out;
		out.open(fileName, std::ios_base::binary);
		if (out.is_open())
		{
			m_settings->save(out);
			int extruderCount = (int)m_extruders.size();
			templateSave<int>(extruderCount, out);
			for (int i = 0; i < extruderCount; ++i)
				m_extruders.at(i)->save(out);

			int groupCount = (int)m_groups.size();
			templateSave<int>(groupCount, out);
			for (int i = 0; i < groupCount; ++i)
				m_groups.at(i)->save(out);
		}
		out.close();
	}

	void CrScene::savePloygons(const std::vector<std::vector<trimesh::vec2>>& polys)
	{
		int pNum = polys.size();
		std::fstream in(m_ploygonFileName, std::ios::out | std::ios::binary);
		if (in.is_open() && pNum> 0)
		{
			in.write((char*)&pNum, sizeof(int));
			if (pNum > 0)
			{
				for (int i = 0; i < pNum; ++i)
				{
					int num = polys.at(i).size();
					in.write((char*)&num, sizeof(int));
					for (int j = 0; j < num; j++)
					{
						in.write((char*)&polys[i][j].x, sizeof(float));
						in.write((char*)&polys[i][j].y, sizeof(float));
					}
				}
			}
		}
	}

	void CrScene::load(const std::string& fileName)
	{
		std::ifstream in;
		in.open(fileName, std::ios_base::binary);
		if (in.is_open())
		{
			m_settings->load(in);
			int extruderCount = templateLoad<int>(in);
			for (int i = 0; i < extruderCount; ++i)
			{
				SettingsPtr setting(new crcommon::Settings());
				setting->load(in);
				m_extruders.push_back(setting);
			}

			int groupCount = templateLoad<int>(in);
			for (int i = 0; i < groupCount; ++i)
			{
				addOneGroup();
				m_groups.at(i)->load(in);
			}
		}
		in.close();
	}

	CrGroup* CrScene::getGroupsIndex(int groupID)
	{
		if (groupID < m_groups.size())
			return  m_groups.at(groupID);
		else
			return nullptr;
	}
}