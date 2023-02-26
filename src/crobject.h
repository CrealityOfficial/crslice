#ifndef CRSLICE_CROBJECT_1669515380930_H
#define CRSLICE_CROBJECT_1669515380930_H
#include "crslice/header.h"
#include <fstream>

namespace crslice
{
	class CrObject
	{
	public:
		CrObject();
		~CrObject();

		void load(std::ifstream& in);
		void save(std::ofstream& out);

		TriMeshPtr m_mesh;
		SettingsPtr m_settings;
	};
}

#endif // CRSLICE_CROBJECT_1669515380930_H