#ifndef slic3r_Debugger_hpp_
#define slic3r_Debugger_hpp_

#include "libslic3r/Print.hpp"

namespace Slic3r {

	class Debugger
	{
	public:
		virtual ~Debugger() {}

		virtual void volume_slices(Print* print, PrintObject* object, const std::vector<VolumeSlices>& slices, const std::vector<float>& slice_zs) = 0;
	};
}

#endif
