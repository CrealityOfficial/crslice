#ifndef CURA52_TIMEESTIMATEKLIPPER_1685609041425_H
#define CURA52_TIMEESTIMATEKLIPPER_1685609041425_H
#include "timeEstimate.h"

namespace cura52
{
	class TimeEstimateKlipper : public TimeEstimateCalculator
	{
	public:
		TimeEstimateKlipper();
		virtual ~TimeEstimateKlipper();

	    void plan(Position newPos, Velocity feedRate, PrintFeatureType feature) override;
	};
}

#endif // CURA52_TIMEESTIMATEKLIPPER_1685609041425_H