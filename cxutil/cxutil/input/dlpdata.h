#ifndef _CXSW_DLPDATA_1593762618888_H
#define _CXSW_DLPDATA_1593762618888_H
#include "polyclipping/clipper.hpp"

namespace cxutil
{
	struct DLPLayer
	{
		ClipperLib::cInt printZ;     //!< The height at which this layer needs to be printed. Can differ from sliceZ due to the raft.
		std::vector< ClipperLib::PolyTree*> parts;
	};

	struct DLPmesh
	{
		std::vector<DLPLayer> layers;
		std::string mesh_name;
	};

	struct DLPmeshs
	{
		std::vector<DLPmesh> dlpmeshs;
	};

	struct DLPmeshsgroup
	{
		std::vector<DLPmeshs> dlpmeshsgroup;
	};


	class DLPData
	{
	public:
		DLPData();
		virtual ~DLPData();
		std::vector<ClipperLib::PolyTree*> trait(int layer);
		int layers();
		DLPmeshsgroup m_dlpmeshsgroup;
		void clearLayer(int layer);
	};
}
#endif // _CXSW_DLPDATA_1593762618888_H
