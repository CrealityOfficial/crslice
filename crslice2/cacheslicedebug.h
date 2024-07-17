#ifndef CRSLICE_CACHE_SLICE_DEBUG_H_2
#define CRSLICE_CACHE_SLICE_DEBUG_H_2
#include "crslice2/cacheslice.h"

namespace ccglobal
{
	class VisualDebugger;
}

namespace crslice2
{
	class CacheSliceImpl;
	class CRSLICE2_API CacheSlice
	{
	public:
		CacheSlice();
		~CacheSlice();

		bool slice(const CacheSliceParam& param, ccglobal::Tracer* tracer = nullptr);

		void volume_slices(int index, ccglobal::VisualDebugger* debugger);
		void surfaces(int index, ccglobal::VisualDebugger* debugger);
	protected:
		CacheSliceImpl* m_impl;
	};
}
#endif  // CRSLICE_CACHE_SLICE_DEBUG_H_2
