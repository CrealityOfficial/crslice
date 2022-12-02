// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include "ccglobal/log.h"

#include "ExtruderTrain.h"
#include "Slice.h"
#include <fstream>      // std::ofstream

namespace cura52
{

Slice::Slice(const size_t num_mesh_groups) : scene(num_mesh_groups)
{
}

void Slice::compute()
{
#if 0
    LOGD("All settings: {}", scene.getAllSettingsString());
    std::ofstream debugsetfile;
    debugsetfile.open(("ALL_Setting.txt"), std::ofstream::out);
    if (debugsetfile.is_open())
    {
        debugsetfile << scene.getAllSettingsString() << std::endl;
        debugsetfile.close();
    }
#endif
    for (std::vector<MeshGroup>::iterator mesh_group = scene.mesh_groups.begin(); mesh_group != scene.mesh_groups.end(); mesh_group++)
    {
        scene.current_mesh_group = mesh_group;
        for (ExtruderTrain& extruder : scene.extruders)
        {
            extruder.settings.setParent(&scene.current_mesh_group->settings);
        }
        scene.processMeshGroup(*mesh_group);
    }
}

void Slice::reset()
{
    scene.extruders.clear();
    scene.mesh_groups.clear();
    scene.settings = Settings();
}

} // namespace cura52