//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#include "cxutil/slicer/ExtruderTrain.h"
#include "Slice.h"

#include "cxutil/util/logoutput.h"

namespace cxutil
{

Slice::Slice(const size_t num_mesh_groups)
	:scene(num_mesh_groups)
{
}

void Slice::compute()
{
    std::vector<Polygons> skirt_brims;
   // LOGW("%s", scene.getAllSettingsString().c_str());
    for (std::vector<MeshGroup>::iterator mesh_group = scene.mesh_groups.begin();
        mesh_group != scene.mesh_groups.end(); mesh_group++)
    {
        scene.current_mesh_group = mesh_group;
        for (ExtruderTrain& extruder : scene.extruders)
        {
            extruder.settings->setParent(scene.current_mesh_group->settings);
        }
        scene.processMeshGroup(*mesh_group, skirt_brims);
    }
}

void Slice::computeBelt(std::vector<Polygons>& skirt_brims)
{
   // LOGW("%s", scene.getAllSettingsString().c_str());
    for (std::vector<MeshGroup>::iterator mesh_group = scene.mesh_groups.begin();
        mesh_group != scene.mesh_groups.end(); mesh_group++)
    {
        float beltAngle = 45.0f * M_PIf / 180.0f;
        float layer_height = mesh_group[0].settings->get<double>("layer_height") / sinf(beltAngle);
        float layer_height0 = mesh_group[0].settings->get<double>("layer_height_0") / sinf(beltAngle);
        mesh_group[0].settings->add("layer_height", std::to_string(layer_height));
        mesh_group[0].settings->add("layer_height_0", std::to_string(layer_height0));

        scene.current_mesh_group = mesh_group;
        for (ExtruderTrain& extruder : scene.extruders)
        {
            extruder.settings->setParent(scene.current_mesh_group->settings);
        }
        scene.processMeshGroup(*mesh_group, skirt_brims);
    }
}

void Slice::reset()
{
	scene.processor.finalize();

    scene.extruders.clear();
    scene.mesh_groups.clear();
    scene.settings = Settings();
}
void Slice::finalize()
{
    size_t numExtruder = scene.extruders.size();
    for (size_t i = 0; i < numExtruder; ++i)
    {
        ExtruderTrain& train = scene.extruders[i];
        train.extruder_nr = i;
        train.settings->add("extruder_nr", std::to_string(i));
    }

    for (MeshGroup& meshGroup : scene.mesh_groups)
    {
        for (auto& mesh : meshGroup.meshes)
        {
            if (mesh->settings->has("extruder_nr"))
            {
                size_t i = mesh->settings->get<size_t>("extruder_nr");
                if (i <= numExtruder)
                {
                    mesh->settings->setParent(scene.extruders[i].settings);
                    continue;
                }
            }

            mesh->settings->setParent(scene.extruders[0].settings);
        }
    }

    for (MeshGroup& meshGroup : scene.mesh_groups)
        meshGroup.finalize();
}
}
