// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include <sstream>
#include "Cache.h"

#include "communication/slicecontext.h"
#include "communication/sliceDataStorage.h"

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
	void convertExtrusionLine(const ExtrusionLine& line, CrPolygon& poly)
	{
		const std::vector<ExtrusionJunction>& jcs = line.junctions;
		for (const ExtrusionJunction& j : jcs)
		{
			poly.push_back(crslice::convert(j.p));
		}
	}

	void convertVariableLines(const VariableWidthLines& lines, CrPolygons& polys)
	{
		polys.clear();
		int size = (int)lines.size();
		if (size > 0)
		{
			polys.resize(size);
			for (int i = 0; i < size; ++i)
				convertExtrusionLine(lines.at(i), polys.at(i));
		}
	}

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
				crslice::convertPolygonRaw(layer.polygons, sslayer.polygons);
				crslice::convertPolygonRaw(layer.openPolylines, sslayer.open_polygons);

				ccglobal::cxndSave(sslayer, fileName);
			}
		}
	}

	void Cache::cacheProcessedSlicedData(const std::vector<SlicedData>& datas)
	{
		CACHE_CHECK_STEP(2);

		int meshSize = (int)datas.size();
		for (int i = 0; i < meshSize; ++i)
		{
			const SlicedData& data = datas.at(i);
			int layerCount = (int)data.layers.size();
			for (int j = 0; j < layerCount; ++j)
			{
				const SlicedLayer& layer = data.layers.at(j);
				std::string fileName = crslice::processed_sliced_layer_name(m_root, i, j);
				crslice::SerailSlicedLayer sslayer;
				sslayer.z = INT2MM(layer.z);
				crslice::convertPolygonRaw(layer.polygons, sslayer.polygons);
				crslice::convertPolygonRaw(layer.openPolylines, sslayer.open_polygons);

				ccglobal::cxndSave(sslayer, fileName);
			}
		}
	}

	void Cache::cacheLayerParts(const SliceDataStorage& storage)
	{
		CACHE_CHECK_STEP(3);

		int meshSize = (int)storage.meshes.size();
		for (int i = 0; i < meshSize; ++i)
		{
			const SliceMeshStorage& data = storage.meshes.at(i);
			int layerCount = (int)data.layers.size();
			for (int j = 0; j < layerCount; ++j)
			{
				const SliceLayer& layer = data.layers.at(j);
				int partsSize = (int)layer.parts.size();

				for (int k = 0; k < partsSize; ++k)
				{
					std::string fileName = crslice::mesh_layer_part_name(m_root, i, j, k);
					const SliceLayerPart& part = layer.parts.at(k);
					crslice::SerailPolygons spoly;
					crslice::convertPolygonRaw(part.outline, spoly.polygons);

					ccglobal::cxndSave(spoly, fileName);
				}
			}
		}
	}

	void Cache::cacheWalls(const SliceDataStorage& storage)
	{
		CACHE_CHECK_STEP(4);

		int meshSize = (int)storage.meshes.size();
		for (int i = 0; i < meshSize; ++i)
		{
			const SliceMeshStorage& data = storage.meshes.at(i);
			int layerCount = (int)data.layers.size();
			for (int j = 0; j < layerCount; ++j)
			{
				const SliceLayer& layer = data.layers.at(j);
				int partsSize = (int)layer.parts.size();

				for (int k = 0; k < partsSize; ++k)
				{
					std::string fileName = crslice::mesh_layer_part_wall_name(m_root, i, j, k);
					const SliceLayerPart& part = layer.parts.at(k);
					crslice::SerialWalls swalls;
					crslice::convertPolygonRaw(part.print_outline, swalls.print_outline);
					crslice::convertPolygonRaw(part.inner_area, swalls.inner_area);
					int count = (int)part.wall_toolpaths.size();
					if (count > 0)
					{
						swalls.walls.resize(count);
						for (int w = 0; w < count; ++w)
						{
							convertVariableLines(part.wall_toolpaths.at(w), swalls.walls.at(w));
						}
					}

					ccglobal::cxndSave(swalls, fileName);
				}
			}
		}
	}
} // namespace cura52
