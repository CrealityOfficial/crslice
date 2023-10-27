//Copyright (c) 2022 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef CACHE_H
#define CACHE_H
#include <string>
#include "slice/sliceddata.h"

namespace cura52 
{
	class SliceContext;
	class Cache 
	{
	public:
		Cache(const std::string& fileName, SliceContext* context);
		virtual ~Cache();

		void cacheSlicedData(const std::vector<SlicedData>& datas);
		void cacheProcessedSlicedData(const std::vector<SlicedData>& datas);
	protected:
		SliceContext* m_context;
		std::string m_root;

		int m_debugStep;
	};
} // namespace cura52

#define USE_CACHE 1
#endif // SVG_H
