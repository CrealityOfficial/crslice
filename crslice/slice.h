#ifndef CRSLICE_SLICE_H
#define CRSLICE_SLICE_H
#include "crslice/interface.h"
#include "crcommon/Settings.h"

namespace crslice
{
	class CRSLICE_API Slice
	{
	public:
		Slice();
		~Slice();

		void sliceFromFakeArguments(int argc, const char* argv[]);
		void sliceFromScene();
	private:
		crcommon::Settings m_settingsCfg;
	public:
		void init(crcommon::Settings *settingsPtr);
		void process();
	};
}
#endif  // MSIMPLIFY_SIMPLIFY_H
