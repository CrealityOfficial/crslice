#include "Debugger.hpp"

namespace Slic3r {
	int index_object(Print* print, PrintObject* object)
	{
		int index = 0;
		for (PrintObject* obj : print->objects_mutable())
		{
			if (obj == object)
			{
				return index;
			}
			++index;
		}
		return -1;
	}

	void bench_debug_objects_num(Print* print)
	{
#ifdef BENCH_DEBUG
		int size = (int)print->objects_mutable().size();
		print->debugger->resize_objects_num(size);
#endif
	}

	void bench_debug_volume_slices(Print* print, PrintObject* object, const std::vector<VolumeSlices>& slices)
	{
#ifdef BENCH_DEBUG
		print->debugger->cache_volume_slices(index_object(print, object), slices);
#endif
	}

	void bench_debug_slice_2_regions(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		print->debugger->cache_slice_2_regions(index_object(print, object), object);
#endif
	}

	void bench_debug_mm_segmentation(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		//print->debugger->cache(index_object(print, object), object);
#endif
	}

	void bench_debug_conical_overhang(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		//print->debugger->cache_slice_2_regions(index_object(print, object), object);
#endif
	}
}
