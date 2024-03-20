#ifndef CRSLICE_CROBJECT_1669515380930_H
#define CRSLICE_CROBJECT_1669515380930_H
#include "crslice2/header.h"
#include <fstream>

namespace crslice2
{
	class CrObject
	{
	public:
		CrObject();
		~CrObject();

		void load(std::ifstream& in);
		void save(std::ofstream& out);

		void load(std::fstream& in, int version);   //for version
		void save(std::fstream& out, int version);  //for version

		TriMeshPtr m_mesh;
		SettingsPtr m_settings;

		std::vector<std::string> m_colors2Facets; //���л�����
		std::vector<std::string> m_seam2Facets; //ͿĨZ��
		std::vector<std::string> m_support2Facets; //ͿĨ֧��

		std::string m_objectName;
		std::vector<double> m_layerHeight;

		//column major
		float m_xform[16];
	};
}

#endif // CRSLICE_CROBJECT_1669515380930_H