//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#include "cxutil/slicer/ExtruderTrain.h"
#include "raft.h"
#include "Slice.h"
#include "sliceDataStorage.h"
#include "support.h"
#include "cxutil/settings/EnumSettings.h" //For EPlatformAdhesion.

#include "cxutil/math/math.h"


namespace cxutil
{

void Raft::generate(SliceDataStorage& storage)
{
    assert(storage.raftOutline.size() == 0 && "Raft polygon isn't generated yet, so should be empty!");
    const Settings& settings = storage.scene->current_mesh_group->settings->get<ExtruderTrain&>("adhesion_extruder_nr").settings;
    const coord_t distance = settings.get<coord_t>("raft_margin");
    constexpr bool include_support = true;
    constexpr bool include_prime_tower = true;
    Polygons meshesShadow = storage.getLayerOutlines(0, include_support, include_prime_tower);
    storage.raftOutline = meshesShadow.offset(distance, ClipperLib::jtRound);
    const coord_t shield_line_width_layer0 = settings.get<coord_t>("skirt_brim_line_width");
    if (storage.draft_protection_shield.size() > 0)
    {
        Polygons draft_shield_raft = storage.draft_protection_shield.offset(shield_line_width_layer0) // start half a line width outside shield
                                        .difference(storage.draft_protection_shield.offset(-distance - shield_line_width_layer0 / 2, ClipperLib::jtRound)); // end distance inside shield
        storage.raftOutline = storage.raftOutline.unionPolygons(draft_shield_raft);
    }
    if (storage.oozeShield.size() > 0 && storage.oozeShield[0].size() > 0)
    {
        const Polygons& ooze_shield = storage.oozeShield[0];
        Polygons ooze_shield_raft = ooze_shield.offset(shield_line_width_layer0) // start half a line width outside shield
                                        .difference(ooze_shield.offset(-distance - shield_line_width_layer0 / 2, ClipperLib::jtRound)); // end distance inside shield
        storage.raftOutline = storage.raftOutline.unionPolygons(ooze_shield_raft);
    }
    const coord_t smoothing = settings.get<coord_t>("raft_smoothing");
    storage.raftOutline = storage.raftOutline.offset(smoothing, ClipperLib::jtRound).offset(-smoothing, ClipperLib::jtRound); // remove small holes and smooth inward corners
   //over
    bool meshOver = true;
    for (auto& point : meshesShadow[0])
    {
        if (point.X >= storage.machine_size.min.x && point.X <= storage.machine_size.max.x && point.Y >= storage.machine_size.min.y && point.Y <= storage.machine_size.max.y)
        {
            meshOver = false;
            break;
        }
    }
    if (!meshOver)
    {
        if (storage.scene->machine_center_is_zero)
        {
            Point3 bbox = storage.machine_size.getMiddle();
            for (auto& point : storage.raftOutline.paths[0])
            {
                if (point.X < -bbox.x) point.X = -bbox.x;
                if (point.X > bbox.x) point.X = bbox.x;
                if (point.Y < -bbox.y) point.Y = -bbox.y;
                if (point.Y > bbox.y) point.Y = bbox.y;
            }
        }
        else
        {
            for (auto& point : storage.raftOutline.paths[0])
            {
                if (point.X < storage.machine_size.min.x) point.X = storage.machine_size.min.x;
                if (point.X > storage.machine_size.max.x) point.X = storage.machine_size.max.x;
                if (point.Y < storage.machine_size.min.y) point.Y = storage.machine_size.min.y;
                if (point.Y > storage.machine_size.max.y) point.Y = storage.machine_size.max.y;
            }
        }
    }
}

coord_t Raft::getTotalThickness(Scene* scene)
{
    const ExtruderTrain& extruder = scene->current_mesh_group->settings->get<ExtruderTrain&>("adhesion_extruder_nr");
    return extruder.settings->get<coord_t>("raft_base_thickness")
        + extruder.settings->get<coord_t>("raft_interface_thickness")
        + extruder.settings->get<size_t>("raft_surface_layers") * extruder.settings->get<coord_t>("raft_surface_thickness");
}
coord_t Raft::getTotalSimpleRaftThickness(Scene* scene)
{
    if (nullptr == scene) return 0;
    const ExtruderTrain& extruder = scene->current_mesh_group->settings->get<ExtruderTrain&>("adhesion_extruder_nr");
    auto bs= extruder.settings->get<coord_t>("raft_base_thickness");
    auto rn = extruder.settings->get<size_t>("raft_init_layer_num");
    auto ri = extruder.settings->get<coord_t>("raft_interface_thickness");
    auto rth = extruder.settings->get<coord_t>("raft_surface_thickness");

    return extruder.settings->get<coord_t>("raft_base_thickness") * extruder.settings->get<size_t>("raft_init_layer_num")
        + extruder.settings->get<coord_t>("raft_interface_thickness")
        + extruder.settings->get<size_t>("raft_top_layer_num") * extruder.settings->get<coord_t>("raft_surface_thickness");
}
coord_t Raft::getZdiffBetweenRaftAndLayer1(Scene* scene)
{
    const Settings& mesh_group_settings = *scene->current_mesh_group->settings;
    const ExtruderTrain& extruder = mesh_group_settings.get<ExtruderTrain&>("adhesion_extruder_nr");
    if (mesh_group_settings.get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::RAFT && mesh_group_settings.get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::SIMPLERAFT)
    {
        return 0;
    }
    const coord_t airgap = std::max(coord_t(0), extruder.settings->get<coord_t>("raft_airgap"));
    const coord_t layer_0_overlap = mesh_group_settings.get<coord_t>("layer_0_z_overlap");

    const coord_t layer_height_0 = mesh_group_settings.get<coord_t>("layer_height_0");

    const coord_t z_diff_raft_to_bottom_of_layer_1 = std::max(coord_t(0), airgap + layer_height_0 - layer_0_overlap);
    return z_diff_raft_to_bottom_of_layer_1;
}

size_t Raft::getFillerLayerCount(Scene* scene)
{
    const coord_t normal_layer_height = scene->current_mesh_group->settings->get<coord_t>("layer_height");
    return round_divide(getZdiffBetweenRaftAndLayer1(scene), normal_layer_height);
}

coord_t Raft::getFillerLayerHeight(Scene* scene)
{
    const Settings& mesh_group_settings = *scene->current_mesh_group->settings;
    if (mesh_group_settings.get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::RAFT && mesh_group_settings.get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::SIMPLERAFT)
    {
        const coord_t normal_layer_height = mesh_group_settings.get<coord_t>("layer_height");
        return normal_layer_height;
    }
    return round_divide(getZdiffBetweenRaftAndLayer1(scene), getFillerLayerCount(scene));
}


size_t Raft::getTotalExtraLayers(Scene* scene)
{
    const ExtruderTrain& extruder = scene->current_mesh_group->settings->get<ExtruderTrain&>("adhesion_extruder_nr");
    if (extruder.settings->get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::RAFT && extruder.settings->get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::SIMPLERAFT)
    {
        return 0;
    }
    return 2 + extruder.settings->get<size_t>("raft_surface_layers") + getFillerLayerCount(scene);
}
size_t Raft::getTotalSimpleExtraLayers(Scene* scene)
{
    const ExtruderTrain& extruder = scene->current_mesh_group->settings->get<ExtruderTrain&>("adhesion_extruder_nr");
    if (extruder.settings->get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::SIMPLERAFT)
    {
        return 0;
    }
    //constexpr int raft_init_layer_num = mgParam->raft_init_layer_num;
   //constexpr int raft_top_layer_num = mgParam->raft_top_layer_num;
    return extruder.settings->get<size_t>("raft_init_layer_num") + extruder.settings->get<size_t>("raft_top_layer_num");
}

}//namespace cxutil
