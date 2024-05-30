#ifndef CRSLICE_CACHE_SLICE_H_2
#define CRSLICE_CACHE_SLICE_H_2
#include "crslice2/header.h"
#include "ccglobal/debugger.h"

namespace crslice2
{
	struct CacheSliceParam
	{
		std::string fileName;
		std::string outName;
	};

	class CacheSliceImpl;
	class CRSLICE2_API CacheSlice
	{
	public:
		CacheSlice();
		~CacheSlice();

		void slice(const CacheSliceParam& param, ccglobal::Tracer* tracer = nullptr);

		void visual_raw_slices(int layer, ccglobal::VisualDebugger* debugger);
	protected:
		CacheSliceImpl* m_impl;
	};
}
#endif  // MSIMPLIFY_SIMPLIFY_H
