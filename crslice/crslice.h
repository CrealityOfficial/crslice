#ifndef CRSLICE_SLICE_H
#define CRSLICE_SLICE_H
#include "crslice/interface.h"
#include "crslice/crscene.h"

namespace crslice
{
	class CRSLICE_API CrSlice
	{
	public:
		CrSlice();
		~CrSlice();

		void sliceFromFakeArguments(int argc, const char* argv[]);
		void sliceFromScene(CrScenePtr scene);
	private:
		crcommon::Settings m_settingsCfg;
	public:
		void init(crcommon::Settings *settingsPtr);
		void process();
	};
}
#endif  // MSIMPLIFY_SIMPLIFY_H
