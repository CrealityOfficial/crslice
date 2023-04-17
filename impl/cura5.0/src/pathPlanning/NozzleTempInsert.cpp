//Copyright (c) 2022 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#include <assert.h>
#include "gcodeExport.h"
#include "pathPlanning/NozzleTempInsert.h"

namespace cura52
{

NozzleTempInsert::NozzleTempInsert(unsigned int path_idx, int extruder, double temperature, bool wait, double time_after_path_start)
: path_idx(path_idx)
, time_after_path_start(time_after_path_start)
, extruder(extruder)
, temperature(temperature)
, wait(wait)
{
    assert(temperature != 0 && temperature != -1 && "Temperature command must be set!");
}

void NozzleTempInsert::write(GCodeExport& gcode)
{
    //屏蔽喷嘴切换时的提前加热，保留单喷嘴自动调温预热功能
    if (gcode.getExtruderNum() == 1 )
    {
        gcode.writeTemperatureCommand(extruder, temperature, wait);
    }
}

}