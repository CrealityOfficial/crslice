#ifndef CX_GROUPINPUT_1600053607518_H
#define CX_GROUPINPUT_1600053607518_H
#include "cxutil/input/meshinput.h"
#include "cxutil/settings/Settings.h"

#include <vector>

namespace cxutil
{
	class GroupInput
	{
	public:
		GroupInput();
		~GroupInput();

		void addMeshInput(MeshInputPtr mesh);
		void removeMeshInput(MeshInputPtr mesh);
		void loadMesh(const std::string& fileName);

		const std::vector<MeshInputPtr>& meshes() const;
		void release();
		Settings* settings();
		MeshGroupParam* param();
		AABB3D box();

		void finalize();
		void releaseMeshData();
	protected:
		std::vector<MeshInputPtr> m_meshInputs;
		Settings* m_setting;
		MeshGroupParam* m_param;
		AABB3D m_box;
	};

	typedef std::shared_ptr<GroupInput> GroupInputPtr;
}

#endif // CX_GROUPINPUT_1600053607518_H