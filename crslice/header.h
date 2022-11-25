#ifndef CRSLICE_HEADER_INTERFACE
#define CRSLICE_HEADER_INTERFACE
#include "crcommon/header.h"
#include <memory>

namespace crslice
{
	struct CrObject
	{
		TriMeshPtr mesh;
		trimesh::fxform xf;
		SettingPtr setting;
	};
}
#endif // CRSLICE_HEADER_INTERFACE