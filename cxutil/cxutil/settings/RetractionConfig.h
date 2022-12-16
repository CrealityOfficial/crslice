//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef CXUTIL_RETRACTION_CONFIG_H
#define CXUTIL_RETRACTION_CONFIG_H

#include "cxutil/settings/Velocity.h"

namespace cxutil
{
    /*!
     * The retraction configuration used in the GCodePathConfig of each feature (and the travel config)
     */
    class RetractionConfig
    {
    public:
        double distance; //!< The distance retracted (in mm)
        Velocity speed; //!< The speed with which to retract (in mm/s)
        Velocity primeSpeed; //!< the speed with which to unretract (in mm/s)
        double prime_volume; //!< the amount of material primed after unretracting (in mm^3)
        coord_t zHop; //!< the amount with which to lift the head during a retraction-travel
        coord_t retraction_min_travel_distance; //!< Minimal distance traversed to even consider retracting (in micron)
        double retraction_extrusion_window; //!< Window of mm extruded filament in which to limit the amount of retractions
        size_t retraction_count_max; //!< The maximum amount of retractions allowed to occur in the RetractionConfig::retraction_extrusion_window
        
		bool travelFlowEnable;//whether the machine size is a super big,then set retraction min travel Flow
		coord_t travelFlow_distance;
		bool advanceEnable;//
        coord_t length_flow_advance; //!< the amount with which to lift the head during a retraction-travel
    };
}//namespace cxutil

#endif // CXUTIL_RETRACTION_CONFIG_H
