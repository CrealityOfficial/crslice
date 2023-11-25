//Copyright (c) 2022 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#include <polyclipping/clipper.hpp>

#include "communication/ExtruderTrain.h"
#include "communication/sliceDataStorage.h"
#include "communication/slicecontext.h"

#include "raft.h"
#include "utils/math.h"

namespace cura52
{

    void Raft::generate(SliceDataStorage& storage)
    {
        assert(storage.raftOutline.size() == 0 && "Raft polygon isn't generated yet, so should be empty!");
        const Settings& settings = storage.application->currentGroup()->settings.get<ExtruderTrain&>("raft_base_extruder_nr").settings;
        const coord_t distance = settings.get<coord_t>("raft_margin");
        constexpr bool include_support = true;
        constexpr bool dont_include_prime_tower = false;  // Prime tower raft will be handled separately in 'storage.primeRaftOutline'; see below.
        storage.raftOutline = storage.getLayerOutlines(0, include_support, dont_include_prime_tower).offset(distance, ClipperLib::jtRound);
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

        if (settings.get<bool>("raft_remove_inside_corners"))
        {
            storage.raftOutline.makeConvex();
        }
        else
        {
            const coord_t smoothing = settings.get<coord_t>("raft_smoothing");
            storage.raftOutline = storage.raftOutline.offset(smoothing, ClipperLib::jtRound).offset(-smoothing, ClipperLib::jtRound); // remove small holes and smooth inward corners
        }

        Polygon machine_box;
        machine_box.add(Point(storage.machine_size.min.x, storage.machine_size.min.y));
        machine_box.add(Point(storage.machine_size.min.x, storage.machine_size.max.y));
        machine_box.add(Point(storage.machine_size.max.x, storage.machine_size.max.y));
        machine_box.add(Point(storage.machine_size.max.x, storage.machine_size.min.y));
        Polygons machine_boxs;
        machine_boxs.add(machine_box);
        storage.raftOutline = storage.raftOutline.intersection(machine_boxs);

        if (storage.primeTower.enabled && !storage.primeTower.would_have_actual_tower)
        {
            // Find out if the prime-tower part of the raft still needs to be printed, even if there is no actual tower.
            // This will only happen if the different raft layers are printed by different extruders.
            const Settings& mesh_group_settings = storage.application->currentGroup()->settings;
            const size_t base_extruder_nr = mesh_group_settings.get<ExtruderTrain&>("raft_base_extruder_nr").extruder_nr;
            const size_t interface_extruder_nr = mesh_group_settings.get<ExtruderTrain&>("raft_interface_extruder_nr").extruder_nr;
            const size_t surface_extruder_nr = mesh_group_settings.get<ExtruderTrain&>("raft_surface_extruder_nr").extruder_nr;
            if (base_extruder_nr == interface_extruder_nr && base_extruder_nr == surface_extruder_nr)
            {
                return;
            }
        }

        storage.primeRaftOutline = storage.primeTower.outer_poly_first_layer.offset(distance, ClipperLib::jtRound);
        if (settings.get<bool>("raft_remove_inside_corners"))
        {
            storage.primeRaftOutline = storage.primeRaftOutline.unionPolygons(storage.raftOutline);
            storage.primeRaftOutline.makeConvex();
        }
        storage.primeRaftOutline = storage.primeRaftOutline.difference(storage.raftOutline).intersection(machine_boxs); // In case of overlaps.
    }

    coord_t Raft::getTotalThickness(SliceContext* application)
    {
        const Settings& mesh_group_settings = application->currentGroup()->settings;
        const ExtruderTrain& base_train = mesh_group_settings.get<ExtruderTrain&>("raft_base_extruder_nr");
        const ExtruderTrain& interface_train = mesh_group_settings.get<ExtruderTrain&>("raft_interface_extruder_nr");
        const ExtruderTrain& surface_train = mesh_group_settings.get<ExtruderTrain&>("raft_surface_extruder_nr");
        return base_train.settings.get<coord_t>("raft_base_thickness")
            + interface_train.settings.get<size_t>("raft_interface_layers") * interface_train.settings.get<coord_t>("raft_interface_thickness")
            + surface_train.settings.get<size_t>("raft_surface_layers") * surface_train.settings.get<coord_t>("raft_surface_thickness");
    }

    coord_t Raft::getTotalSimpleRaftThickness(SliceContext* application)
    {
        if (nullptr == application) return 0;
        const Settings& mesh_group_settings = application->currentGroup()->settings;
        const ExtruderTrain& extruder = mesh_group_settings.get<ExtruderTrain&>("raft_base_extruder_nr");
        auto bs = extruder.settings.get<coord_t>("raft_base_thickness");
        auto rn = extruder.settings.get<size_t>("raft_init_layer_num");
        auto ri = extruder.settings.get<coord_t>("raft_interface_thickness");
        auto rth = extruder.settings.get<coord_t>("raft_surface_thickness");

        return extruder.settings.get<coord_t>("raft_base_thickness") * extruder.settings.get<size_t>("raft_init_layer_num")
            + extruder.settings.get<coord_t>("raft_interface_thickness")
            + extruder.settings.get<size_t>("raft_top_layer_num") * extruder.settings.get<coord_t>("raft_surface_thickness");
    }

    coord_t Raft::getZdiffBetweenRaftAndLayer1(SliceContext* application)
    {
        const Settings& mesh_group_settings = application->currentGroup()->settings;
        const ExtruderTrain& train = mesh_group_settings.get<ExtruderTrain&>("raft_surface_extruder_nr");
        if (mesh_group_settings.get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::RAFT && mesh_group_settings.get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::SIMPLERAFT)
        {
            return 0;
        }
        const coord_t airgap = std::max(coord_t(0), train.settings.get<coord_t>("raft_airgap"));
        const coord_t layer_0_overlap = mesh_group_settings.get<coord_t>("layer_0_z_overlap");

        const coord_t layer_height_0 = application->get_layer_height_0();

        const coord_t z_diff_raft_to_bottom_of_layer_1 = std::max(coord_t(0), airgap + layer_height_0 - layer_0_overlap);
        return z_diff_raft_to_bottom_of_layer_1;
    }

    size_t Raft::getFillerLayerCount(SliceContext* application)
    {
        const coord_t normal_layer_height = application->currentGroup()->settings.get<coord_t>("layer_height");
        return round_divide(getZdiffBetweenRaftAndLayer1(application), normal_layer_height);
    }

    coord_t Raft::getFillerLayerHeight(SliceContext* application)
    {
        const Settings& mesh_group_settings = application->currentGroup()->settings;
        if (mesh_group_settings.get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::RAFT && mesh_group_settings.get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::SIMPLERAFT)
        {
            const coord_t normal_layer_height = mesh_group_settings.get<coord_t>("layer_height");
            return normal_layer_height;
        }
        return round_divide(getZdiffBetweenRaftAndLayer1(application), getFillerLayerCount(application));
    }


    size_t Raft::getTotalExtraLayers(SliceContext* application)
    {
        const Settings& mesh_group_settings = application->currentGroup()->settings;
        const ExtruderTrain& base_train = mesh_group_settings.get<ExtruderTrain&>("raft_base_extruder_nr");
        const ExtruderTrain& interface_train = mesh_group_settings.get<ExtruderTrain&>("raft_interface_extruder_nr");
        const ExtruderTrain& surface_train = mesh_group_settings.get<ExtruderTrain&>("raft_surface_extruder_nr");
        if (base_train.settings.get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::RAFT && base_train.settings.get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::SIMPLERAFT)
        {
            return 0;
        }
        return 1 + interface_train.settings.get<size_t>("raft_interface_layers") + surface_train.settings.get<size_t>("raft_surface_layers") + getFillerLayerCount(application);
    }

    size_t Raft::getTotalSimpleExtraLayers(SliceContext* application)
    {
        const Settings& mesh_group_settings = application->currentGroup()->settings;
        const ExtruderTrain& base_train = mesh_group_settings.get<ExtruderTrain&>("raft_base_extruder_nr");
        if (base_train.settings.get<EPlatformAdhesion>("adhesion_type") != EPlatformAdhesion::SIMPLERAFT)
        {
            return 0;
        }
        //constexpr int raft_init_layer_num = mgParam->raft_init_layer_num;
       //constexpr int raft_top_layer_num = mgParam->raft_top_layer_num;
        return base_train.settings.get<size_t>("raft_init_layer_num") + base_train.settings.get<size_t>("raft_top_layer_num");
    }

}//namespace cura52
