// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include "ccglobal/log.h"

#include "ExtruderTrain.h"
#include "Slice.h"
#include "Application.h"

#include <fstream>      // std::ofstream

namespace cura52
{

    Slice::Slice(const size_t num_mesh_groups) : scene(num_mesh_groups)
    {
    }

    void Slice::compute()
    {
        if (scene.application->debugger)
            scene.application->debugger->startSlice((int)scene.mesh_groups.size());

        int index = 0;
        for (std::vector<MeshGroup>::iterator mesh_group = scene.mesh_groups.begin(); mesh_group != scene.mesh_groups.end(); mesh_group++)
        {
            scene.current_mesh_group = mesh_group;
            for (ExtruderTrain& extruder : scene.extruders)
            {
                extruder.settings.setParent(&scene.current_mesh_group->settings);
            }

            if (scene.application->debugger)
            {
                scene.application->debugger->startGroup(index);
                scene.application->debugger->groupBox(mesh_group->min(), mesh_group->max());
            }

            scene.processMeshGroup(*mesh_group);
            ++index;
        }
    }

    void Slice::reset()
    {
        scene.extruders.clear();
        scene.mesh_groups.clear();
        scene.settings = Settings();
    }

    void Slice::finalize()
    {
        scene.settings.application = scene.application;

        size_t numExtruder = scene.extruders.size();
        for (size_t i = 0; i < numExtruder; ++i)
        {
            ExtruderTrain& train = scene.extruders[i];
            train.extruder_nr = i;
            train.settings.application = scene.application;
            train.settings.add("extruder_nr", std::to_string(i));
        }

        for (MeshGroup& meshGroup : scene.mesh_groups)
        {
            meshGroup.settings.application = scene.application;
            for (Mesh& mesh : meshGroup.meshes)
            {
                mesh.settings.application = scene.application;
                if (mesh.settings.has("extruder_nr"))
                {
                    size_t i = mesh.settings.get<size_t>("extruder_nr");
                    if (i <= numExtruder)
                    {
                        mesh.settings.setParent(&scene.extruders[i].settings);
                        continue;
                    }
                }
     
                mesh.settings.setParent(&scene.extruders[0].settings);
            }
        }

        for (MeshGroup& meshGroup : scene.mesh_groups)
            meshGroup.finalize();
    }

} // namespace cura52