#ifndef MSIMPLIFY_EXPORT
#define MSIMPLIFY_EXPORT
#include "ccglobal/export.h"

#if USE_CRSLICE_DLL
	#define CRSLICE_API CC_DECLARE_IMPORT
#elif USE_CRSLICE_STATIC
	#define CRSLICE_API CC_DECLARE_STATIC
#else
	#if CRSLICE_DLL
		#define CRSLICE_API CC_DECLARE_EXPORT
	#else
		#define CRSLICE_API CC_DECLARE_STATIC
	#endif
#endif


#include "trimesh2/TriMesh.h"
#include <memory>


#endif // MSIMPLIFY_EXPORT