#ifndef CX_DLPSLICER_1602646711722_H
#define CX_DLPSLICER_1602646711722_H
#include "cxutil/input/dlpdata.h"
#include "cxutil/input/dlpinput.h"

namespace cxutil
{
	class SliceTracer;
	class DLPSlicer
	{
	public:
		DLPSlicer();
		~DLPSlicer();

		DLPData* compute(DLPInput* input, SliceTracer* tracer);
	protected:
	};
}

#endif // CX_DLPSLICER_1602646711722_H