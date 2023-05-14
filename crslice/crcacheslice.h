#ifndef CRSLICE_CACHE_SLICE_H
#define CRSLICE_CACHE_SLICE_H
#include "crslice/interface.h"
#include "crslice/crscene.h"

namespace crslice
{
	class CacheDebugger;
	class CRSLICE_API CacheLayer
	{
	public:
		CacheLayer(int layer, CacheDebugger* debugger);
		~CacheLayer();

		int partCount();
		void traitAllParts(std::vector<trimesh::vec3>& lines);
	protected:
		CacheDebugger* debugger;
		int layer;
	};
	class CRSLICE_API CrSCacheSlice
	{
	public:
		CrSCacheSlice();
		~CrSCacheSlice();

		void sliceFromScene(CrScenePtr scene, ccglobal::Tracer* tracer = nullptr);

		//just for 1st group
		trimesh::box3 groupBox();
		int layers();

		CacheLayer* createCacheLayer(int layer);
	protected:
		std::unique_ptr<CacheDebugger> m_debugger;
	};
}

typedef std::shared_ptr<crslice::CacheLayer> CacheLayerPtr;
typedef std::shared_ptr<crslice::CrSCacheSlice> CrSCacheSlicePtr;
#endif  // CRSLICE_CACHE_SLICE_H
