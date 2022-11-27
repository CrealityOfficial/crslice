#ifndef CRSLICE_CROBJECT_1669515380930_H
#define CRSLICE_CROBJECT_1669515380930_H
#include "crslice/header.h"

namespace crslice
{
	class CrObject
	{
	public:
		CrObject();
		~CrObject();

		TriMeshPtr m_mesh;
		SettingsPtr m_setting;
	};
}

#endif // CRSLICE_CROBJECT_1669515380930_H