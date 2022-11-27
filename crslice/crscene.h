#ifndef CRSLICE_SCENE_1668840402293_H
#define CRSLICE_SCENE_1668840402293_H
#include "crslice/interface.h"
#include "crslice/header.h"
#include <vector>

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

		void setObjectSettings(int groupID, int objectID, SettingsPtr settings);
		void setGroupSettings(int groupID, SettingsPtr settings);
		void setSceneSettings(SettingsPtr settings);

		void release();
	protected:
		std::vector<CrGroup*> m_groups;
		SettingsPtr m_settings;
	};
}

typedef std::shared_ptr<crslice::CrScene> CrScenePtr;

#endif // CRSLICE_SCENE_1668840402293_H