#ifndef CX_MESHINPUT_1600053607519_H
#define CX_MESHINPUT_1600053607519_H
#include "cxutil/input/meshobject.h"
#include "cxutil/settings/Settings.h"
#include "cxutil/input/param.h"

namespace cxutil
{
	class MeshInput
	{
	public:
		MeshInput();
		~MeshInput();

		void setMeshObject(MeshObjectPtr object);
		MeshObject* object();  // don't store it
		Settings* settings();
		MeshParam* param();

		void finalize();
		AABB3D box();
	protected:
		MeshObjectPtr m_mesh;
		Settings* m_settings;
		MeshParam* m_param;
	};

	typedef std::shared_ptr<MeshInput> MeshInputPtr;
}

#endif // CX_MESHINPUT_1600053607519_H