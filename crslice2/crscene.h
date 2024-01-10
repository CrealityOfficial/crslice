#ifndef CRSLICE_SCENE_1668840402293_H_2
#define CRSLICE_SCENE_1668840402293_H_2
#include "crslice2/interface.h"
#include "crslice2/header.h"
#include <vector>

#include <fstream>

namespace crslice2
{
	enum class CalibMode : int {
		Calib_None = 0,
		Calib_PA_Line,
		Calib_PA_Tower,
		Calib_Temp_Tower,
		Calib_Vol_speed_Tower,
		Calib_VFA_Tower,
		Calib_Retraction_tower

	};
	struct Calib_Params
	{
		Calib_Params() {}
		double start, end, step;
		bool print_numbers;
		CalibMode mode;
	};


	class CrGroup;
	class CRSLICE2_API CrScene
	{
		friend class CrSlice;
	public:
		CrScene();
		~CrScene();

		int addOneGroup();
		int addObject2Group(int groupID);
		void setOjbectMesh(int groupID, int objectID, TriMeshPtr mesh);
		void setOjbectMeshPaint(int groupID, int objectID, TriMeshPtr mesh
			, const std::vector<std::string>& colors2Facets
			, const std::vector<std::string>& seam2Facets
			, const std::vector<std::string>& support2Facets);

		void setGroupOffset(int groupID, trimesh::vec3 offset);

		void setObjectSettings(int groupID, int objectID, SettingsPtr settings);
		void setGroupSettings(int groupID, SettingsPtr settings);
		void setSceneSettings(SettingsPtr settings);
		void setSceneJsonFile(const std::string& fileName);
		void addSceneParameter(const std::string& key, const std::string& value);
		void setTempDirectory(const std::string& directory);

		void release();
		CrGroup* getGroupsIndex(int groupID);

		void setOutputGCodeFileName(const std::string& fileName);
		void setPloygonFileName(const std::string& fileName);
		void setSupportFileName(const std::string& fileName);
		void setAntiSupportFileName(const std::string& fileName);
		void setSeamFileName(const std::string& fileName);
		void setAntiSeamFileName(const std::string& fileName);
		void setcalibParams(Calib_Params& _calibParams);
		bool valid();

		void save(const std::string& fileName);
		void load(const std::string& fileName);

		//save ploygon
		void savePloygons(const std::vector<std::vector<trimesh::vec2>>& polys);
		void savePloygons(const std::vector<std::vector<trimesh::vec2>>& polys, const std::string filename);
		void setOjbectExclude(int groupID, int objectID, const std::string& fileName, std::vector<trimesh::vec3>& outline_ObjectExclude);
		void makeSureParameters();
	public:
		std::vector<CrGroup*> m_groups;
		SettingsPtr m_settings;
		std::vector<SettingsPtr> m_extruders;
		bool machine_center_is_zero;

		std::string m_gcodeFileName;
		std::string m_ploygonFileName;
		std::string m_supportFile;
		std::string m_antiSupportFile;
		std::string m_seamFile;
		std::string m_antiSeamFile;
	
		std::string m_tempDirectory;
		std::vector<std::string> m_Object_Exclude_FileName;
		Calib_Params m_calibParams;
	};

	class SceneCreator
	{
	public:
		virtual ~SceneCreator() {}

		virtual CrScene* create(ccglobal::Tracer* tracer = nullptr) = 0;
	};

	typedef std::shared_ptr<crslice2::CrScene> CrScenePtr;
}

#endif // CRSLICE_SCENE_1668840402293_H_2