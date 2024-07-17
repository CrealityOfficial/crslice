#include "Debugger.hpp"

namespace Slic3r {
	void bench_debug_volume_slices(Print* print, PrintObject* object, const std::vector<VolumeSlices>& slices, const std::vector<float>& slice_zs)
	{
#ifdef BENCH_DEBUG
		print->debugger->volume_slices(print, object, slices, slice_zs);
#endif
	}
}
