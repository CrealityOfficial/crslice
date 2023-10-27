// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include <sstream>
#include "Cache.h"

#include "communication/slicecontext.h"

#include "crslice/load.h"
#include "conv.h"

#define CACHE_CHECK_STEP(n) \
if (m_debugStep < n)  \
{                     \
	m_context->setFailed();  \
	return; \
}

namespace cura52
{
	Cache::Cache(const std::string& fileName, SliceContext* context)
		: m_root(fileName)
		, m_context(context)
	{
		const Settings& settings = m_context->sceneSettings();

		m_debugStep = settings.get<int>("fdm_slice_debug_step");
		if (m_debugStep < 0)
			m_debugStep = 0;
	}

	Cache::~Cache()
	{

	}

	void Cache::cacheSlicedData(const std::vector<SlicedData>& datas)
	{
		CACHE_CHECK_STEP(1);

		int meshSize = (int)datas.size();
		for (int i = 0; i < meshSize; ++i)
		{
			const SlicedData& data = datas.at(i);
			int layerCount = (int)data.layers.size();
			for (int j = 0; j < layerCount; ++j)
			{
				const SlicedLayer& layer = data.layers.at(j);
				std::string fileName = crslice::sliced_layer_name(m_root, i, j);
				crslice::SerailSlicedLayer sslayer;
				sslayer.z = INT2MM(layer.z);
				crslice::convertPolygonRaw(layer.polygons, sslayer.polygon);
				crslice::convertPolygonRaw(layer.openPolylines, sslayer.open_polygon);

				ccglobal::cxndSave(sslayer, fileName);
			}
		}
	}

	void Cache::cacheProcessedSlicedData(const std::vector<SlicedData>& datas)
	{
		CACHE_CHECK_STEP(2);
	}
} // namespace cura52
