#ifndef CRSLICE_CRGROUP_1669515380929_H
#define CRSLICE_CRGROUP_1669515380929_H
#include "crobject.h"
#include <vector>
#include <fstream>

namespace crslice2
{
	class CrGroup
	{
	public:
		CrGroup();
		~CrGroup();

		int addObject();
		void setObjectMesh(int objectID, TriMeshPtr mesh);
		void setObjectMeshPaint(int objectID, TriMeshPtr mesh
			, const std::vector<std::string>& colors2Facets
			, const std::vector<std::string>& seam2Facets
			, const std::vector<std::string>& support2Facets);

		void setObjectSettings(int objectID, SettingsPtr settings);
		void setSettings(SettingsPtr settings);
		void setOffset(trimesh::vec3 offset);

		void load(std::ifstream& in);
		void save(std::ofstream& out);

		void load(std::fstream& in, int version);   //for version
		void save(std::fstream& out, int version);  //for version

		std::vector<CrObject> m_objects;
		SettingsPtr m_settings;
		trimesh::vec3 m_offset;
	};
}

#endif // CRSLICE_CRGROUP_1669515380929_H