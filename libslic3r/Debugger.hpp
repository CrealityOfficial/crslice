#ifndef slic3r_Debugger_hpp_
#define slic3r_Debugger_hpp_

#include "libslic3r/Print.hpp"

namespace Slic3r {

	class Debugger
	{
	public:
		virtual ~Debugger() {}

		virtual void resize_objects_num(int num) = 0;
		virtual void cache_volume_slices(int index, const std::vector<VolumeSlices>& slices) = 0;
		virtual void cache_slice_2_regions(int index, PrintObject* object) = 0;
	};

	void bench_debug_objects_num(Print* print);
	void bench_debug_volume_slices(Print* print, PrintObject* object, const std::vector<VolumeSlices>& slices);
	void bench_debug_slice_2_regions(Print* print, PrintObject* object);
	void bench_debug_mm_segmentation(Print* print, PrintObject* object);
	void bench_debug_conical_overhang(Print* print, PrintObject* object);
}

#endif
