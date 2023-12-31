#ifndef CRSLICE_SCENE_1668840402293_H
#define CRSLICE_SCENE_1668840402293_H
#include "crslice/interface.h"
#include "crslice/header.h"
#include <vector>

#include <fstream>

namespace crslice
{
	class CrGroup;
	class CRSLICE_API CrScene
	{
		friend class CrSlice;
	public:
		CrScene();
		~CrScene();

		int addOneGroup();
		int addObject2Group(int groupID);
		void setOjbectMesh(int groupID, int objectID, TriMeshPtr mesh);
		void setGroupOffset(int groupID, trimesh::vec3 offset);

		void setObjectSettings(int groupID, int objectID, SettingsPtr settings);
		void setGroupSettings(int groupID, SettingsPtr settings);
		void setSceneSettings(SettingsPtr settings);
		void setSceneJsonFile(const std::string& fileName);
		void setTempDirectory(const std::string& directory);

		void release();
		CrGroup* getGroupsIndex(int groupID);

		void setOutputGCodeFileName(const std::string& fileName);
		void setPloygonFileName(const std::string& fileName);
		bool valid();

		void save(const std::string& fileName);
		void load(const std::string& fileName);

		//save ploygon
		void savePloygons(const std::vector<std::vector<trimesh::vec2>>& polys);
	public:
		std::vector<CrGroup*> m_groups;
		SettingsPtr m_settings;
		std::vector<SettingsPtr> m_extruders;
		bool machine_center_is_zero;

		std::string m_gcodeFileName;
		std::string m_ploygonFileName;
		std::string m_tempDirectory;

		FDMDebugger* m_debugger;
	};

	class SceneCreator
	{
	public:
		virtual ~SceneCreator() {}

		virtual CrScene* create(ccglobal::Tracer* tracer = nullptr) = 0;
	};
}

typedef std::shared_ptr<crslice::CrScene> CrScenePtr;

#endif // CRSLICE_SCENE_1668840402293_H