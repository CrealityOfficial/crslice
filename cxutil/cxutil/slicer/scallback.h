#ifndef CXUTIL_SCALLBACK_1607502965747_H
#define CXUTIL_SCALLBACK_1607502965747_H
#include "cxutil/settings/PrintFeature.h"

namespace cxutil
{
	class SGCodeCallback
	{
	public:
		virtual ~SGCodeCallback() {}

		virtual void onWrite(int x, int y, int z, double velocity, PrintFeatureType type) = 0;
		virtual void onLayerStart(int layer, float thickness) = 0;
		virtual void onExtruderChanged(int extruder) = 0;
	};
}

#endif // CXUTIL_SCALLBACK_1607502965747_H