//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef CXUTIL_FAN_SPEED_LAYER_TIME_H
#define CXUTIL_FAN_SPEED_LAYER_TIME_H

#include "cxutil/settings/LayerIndex.h"
#include "cxutil/settings/Velocity.h"
#include "cxutil/settings/Duration.h"

namespace cxutil
{

    struct FanSpeedLayerTimeSettings
    {
    public:
        Duration cool_min_layer_time;
        Duration cool_min_layer_time_fan_speed_max;
        double cool_fan_speed_0;
        double cool_fan_speed_min;
        double cool_fan_speed_max;
        Velocity cool_min_speed;
        LayerIndex cool_fan_full_layer;
    };

} // namespace cxutil

#endif // CXUTIL_FAN_SPEED_LAYER_TIME_H