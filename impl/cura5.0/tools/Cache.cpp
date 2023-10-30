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

	void convertVectorVariableLines(const std::vector<VariableWidthLines>& vecLines, 
		std::vector<CrPolygons>& vecPolys)
	{
		vecPolys.clear();
		int size = (int)vecLines.size();
		if (size > 0)
		{
			vecPolys.resize(size);
			for (int i = 0; i < size; ++i)
				convertVariableLines(vecLines.at(i), vecPolys.at(i));
		}
	}

	void convertLayerPart(const SkinPart& skinPart,
		crslice::CrSkinPart& crSkin)
	{
		crslice::convertPolygonRaw(skinPart.outline, crSkin.outline);
		convertVectorVariableLines(skinPart.inset_paths, crSkin.inset_paths);
		crslice::convertPolygonRaw(skinPart.skin_fill, crSkin.skin_fill);
		crslice::convertPolygonRaw(skinPart.roofing_fill, crSkin.roofing_fill);
		crslice::convertPolygonRaw(skinPart.top_most_surface_fill, crSkin.top_most_surface_fill);
		crslice::convertPolygonRaw(skinPart.bottom_most_surface_fill, crSkin.bottom_most_surface_fill);
	}

	void convertLayerPart(const SliceLayerPart& layerPart,
		crslice::CrSliceLayerPart& crPart)
	{
		crslice::convertPolygonRaw(layerPart.outline, crPart.outline);
		crslice::convertPolygonRaw(layerPart.inner_area, crPart.inner_area);
		int count = (int)layerPart.skin_parts.size();
		if (count > 0)
		{
			crPart.skin_parts.resize(count);
			for (int i = 0; i < count; ++i)
			{
				convertLayerPart(layerPart.skin_parts.at(i), crPart.skin_parts.at(i));
			}
		}

		convertVectorVariableLines(layerPart.wall_toolpaths, crPart.wall_toolpaths);
		convertVectorVariableLines(layerPart.infill_wall_toolpaths, crPart.infill_wall_toolpaths);
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

	void Cache::cacheAll(const SliceDataStorage& storage)
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
				crslice::SerailCrSliceLayer serialLayer;
				int count = (int)layer.parts.size();
				if (count > 0)
				{
					serialLayer.layer.parts.resize(count);
					for (int w = 0; w < count; ++w)
					{
						convertLayerPart(layer.parts.at(w), serialLayer.layer.parts.at(w));
					}
				}

				std::string fileName = crslice::crslicelayer_name(m_root, i, j);
				ccglobal::cxndSave(serialLayer, fileName);
			}
		}
	}
} // namespace cura52
