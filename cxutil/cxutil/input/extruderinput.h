#ifndef CX_EXTRUDERINPUT_1600399850073_H
#define CX_EXTRUDERINPUT_1600399850073_H
#include "cxutil/settings/Settings.h"
#include "cxutil/input/param.h"

namespace cxutil
{
	class ExtruderInput
	{
	public:
		ExtruderInput();
		~ExtruderInput();

		Settings* settings();
		ExtruderParam* param();

		void finalize();
	protected:
		Settings* m_settings;
		ExtruderParam* m_param;
	};

	typedef std::shared_ptr<ExtruderInput> ExtruderInputPtr;
}

#endif // CX_EXTRUDERINPUT_1600399850073_H