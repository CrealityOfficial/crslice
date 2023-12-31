// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include <cassert>

#include "ccglobal/log.h"

#include "Application.h"
#include "progress/Progress.h"
#include "utils/gettime.h"

namespace cura52
{

double Progress::times[] = {
    0.0,    // START   = 0, 
    5.269,  // SLICING = 1, 
    1.533,  // PARTS   = 2, 
    71.811, // INSET_SKIN = 3
    51.009, // SUPPORT = 4, 
    154.62, // EXPORT  = 5, 
    0.1     // FINISH  = 6
};
std::string Progress::names [] = 
{
    "start",
    "slice",
    "layerparts",
    "inset+skin",
    "support",
    "export",
    "process"
};

float Progress::calcOverallProgress(Stage stage, float stage_progress)
{
    assert(stage_progress <= 1.0);
    assert(stage_progress >= 0.0);
    return ( accumulated_times[(int)stage] + stage_progress * times[(int)stage] ) / total_timing;
}

Progress::Progress()
{
    accumulated_times[N_PROGRESS_STAGES] = { -1 };
    total_timing = -1.0;
}

Progress::~Progress()
{

}

void Progress::init()
{
    double accumulated_time = 0;
    for (int stage = 0; stage < N_PROGRESS_STAGES; stage++)
    {
        accumulated_times[(int)stage] = accumulated_time;
        accumulated_time += times[(int)stage];
    }
    total_timing = accumulated_time;
}

void Progress::messageProgress(Progress::Stage stage, int progress_in_stage, int progress_in_stage_max)
{
    float percentage = calcOverallProgress(stage, float(progress_in_stage) / float(progress_in_stage_max));
    application->sendProgress(percentage);

    // logProgress(names[(int)stage].c_str(), progress_in_stage, progress_in_stage_max, percentage); FIXME: use different sink
}

void Progress::messageProgressStage(Progress::Stage stage)
{
    TimeKeeper& time_keeper = application->processor.time_keeper;

    if ((int)stage > 0)
    {
        LOGI("Progress: { %s } accomplished in { %f }s", names[(int)stage - 1].c_str(), time_keeper.restart());
    }
    else
    {
        time_keeper.restart();
    }
    
    if ((int)stage < (int)Stage::FINISH)
    {
        LOGI("Starting { %s }...", names[(int)stage].c_str());
    }
}

}// namespace cura52