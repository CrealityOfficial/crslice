#ifndef CX_DLPSLICETRACER_1603960812874_H
#define CX_DLPSLICETRACER_1603960812874_H

namespace cxutil
{
	class SliceTracer
	{
	public:
		virtual ~SliceTracer() {}

		virtual void traceProgress(float progress) = 0;
		virtual bool traceInterrupt() = 0;
		virtual void traceFailed(const char* reason) = 0;
	};
}

#endif // CX_DLPSLICETRACER_1603960812874_H