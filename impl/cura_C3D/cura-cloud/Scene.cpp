//Copyright (c) 2019 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#include "FffProcessor.h" //To start a slice.
#include "Scene.h"
#include "sliceDataStorage.h"
#include "Weaver.h"
#include "Wireframe2gcode.h"
#include "communication/Communication.h" //To flush g-code and layer view when we're done.
#include "progress/Progress.h"

#include "cxutil/util/gettime.h"
#include "cxutil/util/logoutput.h"

namespace cxutil
{

    Scene::Scene(const size_t num_mesh_groups)
        : mesh_groups(num_mesh_groups)
        , current_mesh_group(mesh_groups.begin())
        , communication(nullptr)
        , callback(nullptr)
		, sCallback(nullptr)
        , debugCallback(nullptr)
    {
        for (MeshGroup& mesh_group : mesh_groups)
        {
            mesh_group.settings->setParent(&settings);
        }
    }

    const std::string Scene::getAllSettingsString() const
    {
        std::stringstream output;
        output << settings.getAllSettingsString(); //Global settings.

        //Per-extruder settings.
        for (size_t extruder_nr = 0; extruder_nr < extruders.size(); extruder_nr++)
        {
            output << " -e" << extruder_nr << extruders[extruder_nr].settings->getAllSettingsString();
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

            //Per-mesh-group settings.
            const MeshGroup& mesh_group = mesh_groups[mesh_group_index];
            output << mesh_group.settings->getAllSettingsString();

            //Per-object settings.
            for (size_t mesh_index = 0; mesh_index < mesh_group.meshes.size(); mesh_index++)
            {
                const Mesh* mesh = mesh_group.meshes[mesh_index];
                output << " -e" << mesh->settings->get<size_t>("extruder_nr") << " -l \"" << mesh_index << "\"" << mesh->settings->getAllSettingsString();
            }
        }
        output << "\n";

        return output.str();
    }

    void Scene::processMeshGroup(MeshGroup& mesh_group, std::vector<Polygons>& skirt_brims)
    {
        if (callback)
        {
            AABB3D box;
            box.min = mesh_group.min();
            box.max = mesh_group.max();
           // callback->onSceneBox(box);
        }

        if (callback && !mesh_group.settings->get<bool>("machine_is_belt"))
        {
            for (Mesh* mesh : mesh_group.meshes)
                callback->onCxutilMesh(mesh);
        }

        processor.time_keeper.restart();

        TimeKeeper time_keeper_total;

        bool empty = true;
        for (Mesh* mesh : mesh_group.meshes)
        {
            if (!mesh->settings->get<bool>("infill_mesh") && !mesh->settings->get<bool>("anti_overhang_mesh"))
            {
                empty = false;
                break;
            }
        }
        if (empty)
        {
            progress.messageProgress(Progress::Stage::FINISH, 1, 1); // 100% on this meshgroup
            LOGI("Total time elapsed %5.2fs.\n", time_keeper_total.restart());
            return;
        }

        if (mesh_group.settings->get<bool>("wireframe_enabled"))
        {
            LOGI("Starting Neith Weaver...\n");

            Weaver weaver;
            weaver.weave(&mesh_group, this);

            LOGI("Starting Neith Gcode generation...\n");
            Wireframe2gcode gcoder(weaver, processor.gcode_writer.gcode, this);
            gcoder.writeGCode();
            LOGI("Finished Neith Gcode generation...\n");
        }
        else //Normal operation (not wireframe).
        {
            SliceDataStorage storage(this);


            if (!processor.polygon_generator.generateAreas(storage, &mesh_group, &processor.time_keeper,skirt_brims))
            {
                LOGI("generateAreas failed...\n");
                return;
            }

            processor.gcode_writer.writeGCode(storage, &processor.time_keeper);
        }

        progress.messageProgress(Progress::Stage::FINISH, 1, 1); // 100% on this meshgroup
        communication->flushGCode();
        communication->sendOptimizedLayerData();
        LOGI("Total time elapsed %5.2fs.\n", time_keeper_total.restart());
    }

} //namespace cxutil