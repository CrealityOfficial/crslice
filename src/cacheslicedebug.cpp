#include "crslice2/cacheslice.h"
#include "crslice2/crscene.h"
#include "../wrapper/orcaslicewrapper.h"
#include "libslic3r/I18N.hpp"
#include "libslic3r/BuildVolume.hpp"
#include "libslic3r/Debugger.hpp"

#include "conv.h"

#include <boost/format.hpp>
#include "crsliceexception.h"
#include "ccglobal/profile.h"

namespace crslice2
{
	struct ModelObjectCache
	{
		std::vector<Slic3r::VolumeSlices> volume_slices;
		std::vector<std::vector<Slic3r::SurfaceCollection>> slice_2_regions;

		Slic3r::ExPolygons polys;
	};
	
	class CacheSliceImpl : public Slic3r::Debugger
	{
	public:
		CacheSliceImpl() {
		}

		~CacheSliceImpl() {

		}

		void resize_objects_num(int num) override
		{
			m_object_caches.resize(num);
		}

		void cache_volume_slices(int index, const std::vector<Slic3r::VolumeSlices>& slices) override
		{
			m_object_caches.at(index).volume_slices = slices;
		}

		void cache_slice_2_regions(int index, Slic3r::PrintObject* object) override
		{
			ModelObjectCache& cache = m_object_caches.at(index);
			size_t lc = object->layer_count();
			if (lc > 0)
			{
				cache.slice_2_regions.resize(lc);
				for (int i = 0; i < lc; ++i)
				{
					Slic3r::Layer* l = object->get_layer(i);
					size_t rc = l->region_count();
					if (rc > 0)
					{
						cache.slice_2_regions.at(i).reserve(rc);
						for (int j = 0; j < rc; ++j)
							cache.slice_2_regions.at(i).push_back(l->get_region(j)->slices);
					}
				}
			}
		}


		std::vector<ModelObjectCache> m_object_caches;
	};

	CacheSlice::CacheSlice()
		:m_impl(new CacheSliceImpl())
	{

	}

	CacheSlice::~CacheSlice()
	{
		delete m_impl;
	}

	bool CacheSlice::slice(const CacheSliceParam& param, ccglobal::Tracer* tracer)
	{
		SYSTEM_TICK("load scene->");
		CrScenePtr scene(new crslice2::CrScene());
		scene->load(param.fileName);
		SYSTEM_TICK("load scene<-");

		if (scene->m_groups.size() == 0)
		{
			return false;
		}

		Slic3r::Print print;
		OrcaResult result;
		result.gcode_result.reset();

		print.debugger = m_impl;

		Slic3r::Model model;
		Slic3r::DynamicPrintConfig config;

		Slic3r::Calib_Params calibParams;
		calibParams.end = 0.0;
		calibParams.start = 0.0;
		calibParams.step = 0.0;
		calibParams.print_numbers = false;
		Slic3r::ThumbnailsList thumbnailData;

		SYSTEM_TICK("convert ->");
		convert_scene_2_orca(scene, model, config, calibParams, thumbnailData);
		SYSTEM_TICK("convert <-");

		OrcaResult orca_result;
		Slic3r::Calib_Params calib_params;
		auto f = [&](const Slic3r::ThumbnailsParams&) { return Slic3r::ThumbnailsList(); };
		Slic3r::ThumbnailsGeneratorCallback thumbnail_callback = f;

		try
		{
			orca_slice_impl_result(print, model, config, param.outName, param.tempDirectory, thumbnail_callback, calib_params, orca_result, tracer);
		}
		catch (const crslice2::CrSliceException& e)
		{
			_handle_slice_exception_ex(scene, e.sliceObjectId(), e.what(), tracer);
			return false;
		}

		(void)orca_result;
		return true;
	}

	void CacheSlice::volume_slices(ccglobal::VisualDebugger* debugger)
	{
		if (!debugger)
			return;

		for (const ModelObjectCache& moc : m_impl->m_object_caches)
		{
			int volume_count = (int)moc.volume_slices.size();
			for (int i = 0; i < volume_count; ++i)
			{
				const Slic3r::VolumeSlices& vs = moc.volume_slices.at(i);
				int layer_count = (int)vs.slices.size();
				for (int j = 0; j < layer_count; ++j)
				{
					std::string name = boost::str(boost::format("volume_slices_%d_%d")%i%j);

					CovertParam param;
					param.out = trimesh::vec3(1.0f, 0.0f, 0.0f);
					param.in = trimesh::vec3(0.0f, 1.0f, 0.0f);
					std::vector<trimesh::vec3> lines;
					std::vector<trimesh::vec4> colors;
					convert(vs.slices.at(j), lines, &colors, param);
					debugger->visual_color_polygon(name, lines, colors, 1.0f);
				}
			}
		}
	}

}