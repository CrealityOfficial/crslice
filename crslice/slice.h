#ifndef CRSLICE_SLICE_H
#define CRSLICE_SLICE_H
#include "crslice/interface.h"
#include "crslice/Settings.h"
namespace crslice
{

	class CRSLICE_API Slice
	{
	public:
		Slice();
		~Slice();
	private:
		Settings m_settingsCfg;
	public:
		void init( Settings *settingsPtr);
		void process();
	};
}
#endif  // MSIMPLIFY_SIMPLIFY_H
