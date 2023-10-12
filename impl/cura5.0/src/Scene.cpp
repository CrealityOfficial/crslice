// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include "Application.h"
#include "FffProcessor.h" //To start a slice.
#include "Scene.h"
#include "Weaver.h"
#include "Wireframe2gcode.h"
#include "progress/Progress.h"
#include "sliceDataStorage.h"

#include "ccglobal/log.h"

namespace cura52
{

    Scene::Scene(const size_t num_mesh_groups) : mesh_groups(num_mesh_groups), current_mesh_group(mesh_groups.begin())
    {
        for (MeshGroup& mesh_group : mesh_groups)
        {
            mesh_group.settings.setParent(&settings);
        }
    }

    const std::string Scene::getAllSettingsString() const
    {
        std::stringstream output;
        output << settings.getAllSettingsString(); // Global settings.

        // Per-extruder settings.
        for (size_t extruder_nr = 0; extruder_nr < extruders.size(); extruder_nr++)
        {
            output << " -e" << extruder_nr << extruders[extruder_nr].settings.getAllSettingsString();
        }

        for (size_t mesh_group_index = 0; mesh_group_index < mesh_groups.size(); mesh_group_index++)
        {
            if (mesh_group_index == 0)
            {
                output << " -g";
            }
            else
            {
                output << " --next";
            }

            // Per-mesh-group settings.
            const MeshGroup& mesh_group = mesh_groups[mesh_group_index];
            output << mesh_group.settings.getAllSettingsString();

            // Per-object settings.
            for (size_t mesh_index = 0; mesh_index < mesh_group.meshes.size(); mesh_index++)
            {
                const Mesh& mesh = mesh_group.meshes[mesh_index];
                output << " -e" << mesh.settings.get<size_t>("extruder_nr") << " -l \"" << mesh_index << "\"" << mesh.settings.getAllSettingsString();
            }
        }
        output << "\n";

        return output.str();
    }

    void Scene::processMeshGroup(MeshGroup& mesh_group)
    {
        application->progressor.restartTime();
        SAFE_MESSAGE("{0}");

        TimeKeeper time_keeper_total;

        bool empty = true;
        for (Mesh& mesh : mesh_group.meshes)
        {
            if (!mesh.settings.get<bool>("infill_mesh") && !mesh.settings.get<bool>("anti_overhang_mesh"))
            {
                empty = false;
                break;
            }
        }
        if (empty)
        {
            application->progressor.messageProgress(Progress::Stage::FINISH, 1, 1); // 100% on this meshgroup
            LOGI("Total time elapsed { %f }s.", time_keeper_total.restart());
            return;
        }

        FffProcessor& fff_processor = application->processor;
        if (mesh_group.settings.get<bool>("wireframe_enabled"))
        {
            LOGI("Starting Neith Weaver...");

            Weaver weaver;
            weaver.application = application;

            weaver.weave(&mesh_group);

            LOGI("Starting Neith Gcode generation...");
            Wireframe2gcode gcoder(weaver, fff_processor.gcode_writer.gcode);
            gcoder.writeGCode();
            LOGI("Finished Neith Gcode generation...");
        }
        else // Normal operation (not wireframe).
        {
            SliceDataStorage storage(application);
            if (!fff_processor.polygon_generator.generateAreas(storage, &mesh_group))
            {
                return;
            }

            application->progressor.messageProgressStage(Progress::Stage::EXPORT);

            CALLTICK("writeGCode 0");
            fff_processor.gcode_writer.writeGCode(storage);
            CALLTICK("writeGCode 1");
        }

        application->progressor.messageProgress(Progress::Stage::FINISH, 1, 1); // 100% on this meshgroup
        LOGI("Total time elapsed { %f }s.\n", time_keeper_total.restart());
    }
} // namespace cura52