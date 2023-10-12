// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include <cstring>
#include <optional>

#include "ccglobal/log.h"

#include "Application.h" //To communicate layer view data.
#include "ExtruderTrain.h"
#include "LayerPlan.h"
#include "PathOrderMonotonic.h" //Monotonic ordering of skin lines.
#include "WipeScriptConfig.h"
#include "pathPlanning/Comb.h"
#include "pathPlanning/CombPaths.h"
#include "raft.h" // getTotalExtraLayers
#include "settings/types/Ratio.h"
#include "sliceDataStorage.h"
#include "utils/Simplify.h"
#include "utils/linearAlg2D.h"
#include "utils/polygonUtils.h"
#include "Slice3rBase/ArcFitter.hpp"

#include "settings/FlowTempGraph.h"

namespace cura52
{

constexpr int MINIMUM_LINE_LENGTH = 5; // in uM. Generated lines shorter than this may be discarded
constexpr int MINIMUM_SQUARED_LINE_LENGTH = MINIMUM_LINE_LENGTH * MINIMUM_LINE_LENGTH;

ExtruderPlan::ExtruderPlan(const size_t extruder,
                           const LayerIndex layer_nr,
                           const bool is_initial_layer,
                           const bool is_raft_layer,
                           const coord_t layer_thickness,
                           const FanSpeedLayerTimeSettings& fan_speed_layer_time_settings,
                           const RetractionConfig& retraction_config)
    : heated_pre_travel_time(0)
    , required_start_temperature(-1)
    , extruder_nr(extruder)
    , layer_nr(layer_nr)
    , is_initial_layer(is_initial_layer)
    , is_raft_layer(is_raft_layer)
    , layer_thickness(layer_thickness)
    , fan_speed_layer_time_settings(fan_speed_layer_time_settings)
    , retraction_config(retraction_config)
    , extrudeSpeedFactor(1.0)
    , extraTime(0.0)
    , totalPrintTime(0)
    , cool_min_layer_time_correct(0)
    , slowdown_level(0)
{
    min_layertime = fan_speed_layer_time_settings.cool_min_layer_time;
}

void ExtruderPlan::handleInserts(unsigned int& path_idx, GCodeExport& gcode)
{
    while (! inserts.empty() && path_idx >= inserts.front().path_idx)
    { // handle the Insert to be inserted before this path_idx (and all inserts not handled yet)
        inserts.front().write(gcode);
        inserts.pop_front();
    }
    while (!fan_inserts.empty() && path_idx >= fan_inserts.front().path_idx)
    { // handle the Insert to be inserted before this path_idx (and all inserts not handled yet)
        fan_inserts.front().write(gcode);
        fan_inserts.pop_front();
    }
}

void ExtruderPlan::handleAllRemainingInserts(GCodeExport& gcode)
{
    while (! inserts.empty())
    { // handle the Insert to be inserted before this path_idx (and all inserts not handled yet)
        NozzleTempInsert& insert = inserts.front();
        assert(insert.path_idx == paths.size());
        insert.write(gcode);
        inserts.pop_front();
    }
}

void ExtruderPlan::setExtrudeSpeedFactor(const Ratio speed_factor)
{
    extrudeSpeedFactor = speed_factor;
}

double ExtruderPlan::getExtrudeSpeedFactor()
{
    return extrudeSpeedFactor;
}

void ExtruderPlan::setFanSpeed(double _fan_speed)
{
    fan_speed = _fan_speed;
}
double ExtruderPlan::getFanSpeed()
{
    return fan_speed;
}

double ExtruderPlan::getTotalPrintTime()
{
    return totalPrintTime;
}

double ExtruderPlan::getFirstPrintPathFanSpeed()
{
    double first_Print_path_fan_speed = GCodePathConfig::FAN_SPEED_DEFAULT;
    for (unsigned int path_idx = 0; path_idx < paths.size(); path_idx++)
    {
        if (!paths[path_idx].isTravelPath())
        {
            first_Print_path_fan_speed = paths[path_idx].getFanSpeed();
            break;
        }
    }
    return first_Print_path_fan_speed;
}

void ExtruderPlan::applyBackPressureCompensation(const Ratio back_pressure_compensation, const Velocity max_feed_speed)
{
    constexpr double epsilon_speed_factor = 0.001; // Don't put on actual 'limit double minimum', because we don't want printers to stall.
    for (auto& path : paths)
    {
        const double nominal_width_for_path = static_cast<double>(path.config->getLineWidth());
        if (path.width_factor <= 0.0 || nominal_width_for_path <= 0.0 || path.config->isTravelPath() || path.config->isBridgePath())
        {
            continue;
        }
        const double line_width_for_path = path.width_factor * nominal_width_for_path;
        double max_speed_back_pressure_factor = max_feed_speed / path.config->getSpeed();
        path.speed_back_pressure_factor = std::min(max_speed_back_pressure_factor, std::max(epsilon_speed_factor, 1.0 + (nominal_width_for_path / line_width_for_path - 1.0) * back_pressure_compensation));
    }
}

GCodePath* LayerPlan::getLatestPathWithConfig(const GCodePathConfig& config, SpaceFillType space_fill_type, const Ratio flow, const Ratio width_factor, bool spiralize, const Ratio speed_factor)
{
    std::vector<GCodePath>& paths = extruder_plans.back().paths;
    if (paths.size() > 0 && paths.back().config == &config && ! paths.back().done && paths.back().flow == flow && paths.back().width_factor == width_factor && paths.back().speed_factor == speed_factor
        && paths.back().mesh_id == current_mesh) // spiralize can only change when a travel path is in between
    {
        return &paths.back();
    }
    paths.emplace_back(config, current_mesh, space_fill_type, flow, width_factor, spiralize, speed_factor);
    GCodePath* ret = &paths.back();
    if (config.type == PrintFeatureType::Infill || config.type == PrintFeatureType::Skin)
    {
        ret->width_factor = (config.getLineWidth() - fill_lineWidth_diff) / (Ratio)config.getLineWidth();
    }
    ret->skip_agressive_merge_hint = mode_skip_agressive_merge;
    return ret;
}

const Polygons* LayerPlan::getCombBoundaryInside() const
{
    return &comb_boundary_preferred;
}

void LayerPlan::forceNewPathStart()
{
    std::vector<GCodePath>& paths = extruder_plans.back().paths;
    if (paths.size() > 0)
        paths[paths.size() - 1].done = true;
}

LayerPlan::LayerPlan(const SliceDataStorage& storage,
                     LayerIndex layer_nr,
                     coord_t z,
                     coord_t layer_thickness,
                     size_t start_extruder,
                     const std::vector<FanSpeedLayerTimeSettings>& fan_speed_layer_time_settings_per_extruder,
                     coord_t comb_boundary_offset,
                     coord_t comb_move_inside_distance,
                     coord_t travel_avoid_distance)
    : configs_storage(storage, layer_nr, layer_thickness)
    , z(z)
    , final_travel_z(z)
    , mode_skip_agressive_merge(false)
    , application(storage.application)
    , storage(storage)
    , layer_nr(layer_nr)
    , is_initial_layer(layer_nr == 0 - static_cast<LayerIndex>(Raft::getTotalExtraLayers(storage.application)))
    , is_raft_layer(layer_nr < 0 - static_cast<LayerIndex>(Raft::getFillerLayerCount(storage.application)))
    , layer_thickness(layer_thickness)
    , has_prime_tower_planned_per_extruder(application->scene->extruders.size(), false)
    , current_mesh("")
    , last_extruder_previous_layer(start_extruder)
    , last_planned_extruder(&application->scene->extruders[start_extruder])
    , first_travel_destination_is_inside(false) // set properly when addTravel is called for the first time (otherwise not set properly)
    , comb_boundary_minimum(computeCombBoundary(CombBoundary::MINIMUM))
    , comb_boundary_preferred(computeCombBoundary(CombBoundary::PREFERRED))
    , comb_move_inside_distance(comb_move_inside_distance)
    , fan_speed_layer_time_settings_per_extruder(fan_speed_layer_time_settings_per_extruder)
    , fill_lineWidth_diff(0)
{
    size_t current_extruder = start_extruder;
    was_inside = true; // not used, because the first travel move is bogus
    is_inside = false; // assumes the next move will not be to inside a layer part (overwritten just before going into a layer part)
    if (application->scene->current_mesh_group->settings.get<CombingMode>("retraction_combing") != CombingMode::OFF)
    {
        comb = new Comb(storage, layer_nr, comb_boundary_minimum, comb_boundary_preferred, comb_boundary_offset, travel_avoid_distance, comb_move_inside_distance);
    }
    else
    {
        comb = nullptr;
    }
    for (const ExtruderTrain& extruder : application->scene->extruders)
    {
        layer_start_pos_per_extruder.emplace_back(extruder.settings.get<coord_t>("layer_start_x"), extruder.settings.get<coord_t>("layer_start_y"));
    }
    extruder_plans.reserve(application->scene->extruders.size());
    extruder_plans.emplace_back(current_extruder, layer_nr, is_initial_layer, is_raft_layer, layer_thickness, fan_speed_layer_time_settings_per_extruder[current_extruder], storage.retraction_config_per_extruder[current_extruder]);
    extruder_plans.back().slowdown_level = application->scene->extruders[current_extruder].settings.get<bool>("cool_infill_speed_slowdown_first") ? 1 : 0;

    for (size_t extruder_nr = 0; extruder_nr < application->scene->extruders.size(); extruder_nr++)
    { // Skirt and brim.
        skirt_brim_is_processed[extruder_nr] = false;
    }

    layerTemp = 0;
	maxvolumetricspeed = 0.0;
	pressureValue = -1;
    tmp_mesh_name = "";
    first_mesh_cancel = false;
    tmp_is_change_layer = 0;
    need_smart_brim = false;
	is_deceleration_speed = false;
}

LayerPlan::~LayerPlan()
{
    if (comb)
        delete comb;
}

ExtruderTrain* LayerPlan::getLastPlannedExtruderTrain()
{
    return last_planned_extruder;
}

Polygons LayerPlan::computeCombBoundary(const CombBoundary boundary_type)
{
    Polygons comb_boundary;
    const CombingMode mesh_combing_mode = application->scene->current_mesh_group->settings.get<CombingMode>("retraction_combing");
    if (mesh_combing_mode != CombingMode::OFF && (layer_nr >= 0 || mesh_combing_mode != CombingMode::NO_SKIN))
    {
        if (layer_nr < 0)
        {
            comb_boundary = storage.raftOutline.offset(MM2INT(0.1));
        }
        else
        {
            for (const SliceMeshStorage& mesh : storage.meshes)
            {
                const SliceLayer& layer = mesh.layers[static_cast<size_t>(layer_nr)];
                // don't process infill_mesh or anti_overhang_mesh
                if (mesh.settings.get<bool>("infill_mesh") && mesh.settings.get<bool>("anti_overhang_mesh"))
                {
                    continue;
                }
                coord_t offset;
                switch (boundary_type)
                {
                case CombBoundary::MINIMUM:
                    offset = -mesh.settings.get<coord_t>("machine_nozzle_size") / 2 - 0.1 - mesh.settings.get<coord_t>("wall_line_width_0") / 2;
                    break;
                case CombBoundary::PREFERRED:
                    offset = -mesh.settings.get<coord_t>("machine_nozzle_size") * 3 / 2 - mesh.settings.get<coord_t>("wall_line_width_0") / 2;
                    break;
                default:
                    offset = 0;
                    LOGW("Unknown combing boundary type. Did you forget to configure the comb offset for a new boundary type?");
                    break;
                }

                const CombingMode combing_mode = mesh.settings.get<CombingMode>("retraction_combing");
                for (const SliceLayerPart& part : layer.parts)
                {
                    if (combing_mode == CombingMode::ALL) // Add the increased outline offset (skin, infill and part of the inner walls)
                    {
                        comb_boundary.add(part.outline.offset(offset));
                    }
                    else if (combing_mode == CombingMode::NO_SKIN) // Add the increased outline offset, subtract skin (infill and part of the inner walls)
                    {
                        comb_boundary.add(part.outline.offset(offset).difference(part.inner_area.difference(part.infill_area)));
                    }
                    else if (combing_mode == CombingMode::NO_OUTER_SURFACES)
                    {
                        Polygons top_and_bottom_most_fill;
                        for (const SliceLayerPart& part : layer.parts)
                        {
                            for (const SkinPart& skin_part : part.skin_parts)
                            {
                                top_and_bottom_most_fill.add(skin_part.top_most_surface_fill);
                                top_and_bottom_most_fill.add(skin_part.bottom_most_surface_fill);
                            }
                        }
                        comb_boundary.add(part.outline.offset(offset).difference(top_and_bottom_most_fill));
                    }
                    else if (combing_mode == CombingMode::INFILL) // Add the infill (infill only)
                    {
                        comb_boundary.add(part.infill_area);
                    }
                }
            }
        }
    }

    return comb_boundary;
}

void LayerPlan::setIsInside(bool _is_inside)
{
    is_inside = _is_inside;
}

bool LayerPlan::bLayerBegin()
{
    return extruder_plans.back().paths.empty();
}

bool LayerPlan::setExtruder(const size_t extruder_nr)
{
    //if (extruder_nr == getExtruder())
    //{
    //    return false;
    //}
    setIsInside(false);
    { // handle end position of the prev extruder
        ExtruderTrain* extruder = getLastPlannedExtruderTrain();
        const bool end_pos_absolute = extruder->settings.get<bool>("machine_extruder_end_pos_abs");
        Point end_pos(extruder->settings.get<coord_t>("machine_extruder_end_pos_x"), extruder->settings.get<coord_t>("machine_extruder_end_pos_y"));
        if (! end_pos_absolute)
        {
            end_pos += getLastPlannedPositionOrStartingPosition();
        }
        else
        {
            const Point extruder_offset(extruder->settings.get<coord_t>("machine_nozzle_offset_x"), extruder->settings.get<coord_t>("machine_nozzle_offset_y"));
            end_pos += extruder_offset; // absolute end pos is given as a head position
        }
        if (end_pos_absolute || last_planned_position)
        {
            addTravel(end_pos); //  + extruder_offset cause it
        }
    }
    if (extruder_plans.back().paths.empty() && extruder_plans.back().inserts.empty())
    { // first extruder plan in a layer might be empty, cause it is made with the last extruder planned in the previous layer
        extruder_plans.back().extruder_nr = extruder_nr;
    }
    extruder_plans.emplace_back(extruder_nr, layer_nr, is_initial_layer, is_raft_layer, layer_thickness, fan_speed_layer_time_settings_per_extruder[extruder_nr], storage.retraction_config_per_extruder[extruder_nr]);
    //assert(extruder_plans.size() <= application->scene->extruders.size() && "Never use the same extruder twice on one layer!");
    last_planned_extruder = &application->scene->extruders[extruder_nr];

    { // handle starting pos of the new extruder
        ExtruderTrain* extruder = getLastPlannedExtruderTrain();
        const bool start_pos_absolute = extruder->settings.get<bool>("machine_extruder_start_pos_abs");
        Point start_pos(extruder->settings.get<coord_t>("machine_extruder_start_pos_x"), extruder->settings.get<coord_t>("machine_extruder_start_pos_y"));
        if (! start_pos_absolute)
        {
            start_pos += getLastPlannedPositionOrStartingPosition();
        }
        else
        {
            Point extruder_offset(extruder->settings.get<coord_t>("machine_nozzle_offset_x"), extruder->settings.get<coord_t>("machine_nozzle_offset_y"));
            start_pos += extruder_offset; // absolute start pos is given as a head position
        }
        if (start_pos_absolute || last_planned_position)
        {
            last_planned_position = start_pos;
        }
    }
    return true;
}
void LayerPlan::setMesh(const std::string mesh_id)
{
    current_mesh = mesh_id;
}
void LayerPlan::setMesh2(const std::string mesh_id)
{
    tmp_mesh_name = mesh_id;
}
void LayerPlan::moveInsideCombBoundary(const coord_t distance, const std::optional<SliceLayerPart>& part)
{
    constexpr coord_t max_dist2 = MM2INT(2.0) * MM2INT(2.0); // if we are further than this distance, we conclude we are not inside even though we thought we were.
    // this function is to be used to move from the boundary of a part to inside the part
    Point p = getLastPlannedPositionOrStartingPosition(); // copy, since we are going to move p
    if (PolygonUtils::moveInside(comb_boundary_preferred, p, distance, max_dist2) != NO_INDEX)
    {
        // Move inside again, so we move out of tight 90deg corners
        PolygonUtils::moveInside(comb_boundary_preferred, p, distance, max_dist2);
        if (comb_boundary_preferred.inside(p) && (part == std::nullopt || part->outline.inside(p)))
        {
            addTravel_simple(p);
            // Make sure the that any retraction happens after this move, not before it by starting a new move path.
            forceNewPathStart();
        }
    }
}

bool LayerPlan::getPrimeTowerIsPlanned(unsigned int extruder_nr) const
{
    return has_prime_tower_planned_per_extruder[extruder_nr];
}

bool LayerPlan::getTowerIsPlanned()
{
    bool isPlanned = false;
    for (std::vector<bool>::iterator it = has_prime_tower_planned_per_extruder.begin();it !=has_prime_tower_planned_per_extruder.end();it++)
    {
        if (*it)
        {
            isPlanned = *it;
        }
    }
    return isPlanned;
}

void LayerPlan::setPrimeTowerIsPlanned(unsigned int extruder_nr)
{
    has_prime_tower_planned_per_extruder[extruder_nr] = true;
}

std::optional<std::pair<Point, bool>> LayerPlan::getFirstTravelDestinationState() const
{
    std::optional<std::pair<Point, bool>> ret;
    if (first_travel_destination)
    {
        ret = std::make_pair(*first_travel_destination, first_travel_destination_is_inside);
    }
    return ret;
}

GCodePath& LayerPlan::addTravel(const Point p, const bool force_retract)
{
    const GCodePathConfig& travel_config = configs_storage.travel_config_per_extruder[getExtruder()];
    const RetractionConfig& retraction_config = storage.retraction_config_per_extruder[getExtruder()];

    GCodePath* path = getLatestPathWithConfig(travel_config, SpaceFillType::None);

    bool combed = false;

    const ExtruderTrain* extruder = getLastPlannedExtruderTrain();

    const bool is_first_travel_of_extruder_after_switch = extruder_plans.back().paths.size() == 1 && (extruder_plans.size() > 1 || last_extruder_previous_layer != getExtruder());
    bool bypass_combing = is_first_travel_of_extruder_after_switch && extruder->settings.get<bool>("retraction_hop_after_extruder_switch");

    const bool is_first_travel_of_layer = ! static_cast<bool>(last_planned_position);
    const bool retraction_enable = extruder->settings.get<bool>("retraction_enable");
    if (is_first_travel_of_layer)
    {
        bypass_combing = true; // first travel move is bogus; it is added after this and the previous layer have been planned in LayerPlanBuffer::addConnectingTravelMove
        first_travel_destination = p;
        first_travel_destination_is_inside = is_inside;
        if (layer_nr == 0 && retraction_enable && extruder->settings.get<bool>("retraction_hop_enabled"))
        {
            path->retract = true;
            path->perform_z_hop = true;
        }
        forceNewPathStart(); // force a new travel path after this first bogus move
    }
    else if (force_retract /*&& last_planned_position && ! shorterThen(*last_planned_position - p, retraction_config.retraction_min_travel_distance)*/)
    {//force retract is not limited to retraction_min_travel_distance
        // path is not shorter than min travel distance, force a retraction
        path->retract = true;
        if (comb == nullptr)
        {
            path->perform_z_hop = extruder->settings.get<bool>("retraction_hop_enabled");
        }
    }

    if (comb != nullptr && ! bypass_combing)
    {
        CombPaths combPaths;

        // Divide by 2 to get the radius
        // Multiply by 2 because if two lines start and end points places very close then will be applied combing with retractions. (Ex: for brim)
        const coord_t max_distance_ignored = extruder->settings.get<coord_t>("machine_nozzle_tip_outer_diameter") / 2 * 2;

        bool unretract_before_last_travel_move = false; // Decided when calculating the combing
        combed = comb->calc(*extruder, *last_planned_position, p, combPaths, was_inside, is_inside, max_distance_ignored, unretract_before_last_travel_move);
        if (combed)
        {
            bool retract = path->retract || (combPaths.size() > 1 && retraction_enable);
            if (! retract)
            { // check whether we want to retract
                if (combPaths.throughAir)
                {
                    retract = retraction_enable;
                }
                else
                {
                    for (CombPath& combPath : combPaths)
                    { // retract when path moves through a boundary
                        if (combPath.cross_boundary)
                        {
                            retract = retraction_enable;
                            break;
                        }
                    }
                }
                if (combPaths.size() == 1)
                {
                    CombPath comb_path = combPaths[0];
                    if (extruder->settings.get<bool>("limit_support_retractions") && combPaths.throughAir && ! comb_path.cross_boundary && comb_path.size() == 2 && comb_path[0] == *last_planned_position && comb_path[1] == p)
                    { // limit the retractions from support to support, which didn't cross anything
                        retract = false;
                    }
                }
            }

            const coord_t maximum_travel_resolution = extruder->settings.get<coord_t>("meshfix_maximum_travel_resolution");
            coord_t distance = 0;
            Point last_point((last_planned_position) ? *last_planned_position : Point(0, 0));
            for (CombPath& combPath : combPaths)
            { // add all comb paths (don't do anything special for paths which are moving through air)
                if (combPath.empty())
                {
                    continue;
                }
                for (Point& comb_point : combPath)
                {
                    if (comb_point.X < storage.machine_size.min.x) comb_point.X = storage.machine_size.min.x;
                    if (comb_point.X > storage.machine_size.max.x) comb_point.X = storage.machine_size.max.x;
                    if (comb_point.Y < storage.machine_size.min.y) comb_point.Y = storage.machine_size.min.y;
                    if (comb_point.Y > storage.machine_size.max.y) comb_point.Y = storage.machine_size.max.y;
                    if (path->points.empty() || vSize2(path->points.back() - comb_point) > maximum_travel_resolution * maximum_travel_resolution)
                    {
                        path->points.push_back(comb_point);
                        distance += vSize(last_point - comb_point);
                        last_point = comb_point;
                    }
                }
                distance += vSize(last_point - p);
                const coord_t retract_threshold = extruder->settings.get<coord_t>("retraction_combing_max_distance");
                path->retract = retract || (retract_threshold > 0 && distance > retract_threshold && retraction_enable);
                // don't perform a z-hop
                path->perform_z_hop = path->retract && extruder->settings.get<bool>("retraction_hop_enabled") && comb->combInside();//z-hop when combing inside a part
            }
            // Whether to unretract before the last travel move of the travel path, which comes before the wall to be printed.
            // This should be true when traveling towards an outer wall to make sure that the unretraction will happen before the
            // last travel move BEFORE going to that wall. This way, the nozzle doesn't sit still on top of the outer wall's
            // path while it is unretracting, avoiding possible blips.
            path->unretract_before_last_travel_move = path->retract && unretract_before_last_travel_move;
        }
    }

    // CURA-6675:
    // Retraction Minimal Travel Distance should work for all travel moves. If the travel move is shorter than the
    // Retraction Minimal Travel Distance, retraction should be disabled.
    if (!force_retract && ! is_first_travel_of_layer && last_planned_position && shorterThen(*last_planned_position - p, retraction_config.retraction_min_travel_distance))
    {
        path->retract = false;
        path->perform_z_hop = false;
    }

    // no combing? retract only when path is not shorter than minimum travel distance
    if (! combed && ! is_first_travel_of_layer && last_planned_position && ! shorterThen(*last_planned_position - p, retraction_config.retraction_min_travel_distance))
    {
        if (was_inside) // when the previous location was from printing something which is considered inside (not support or prime tower etc)
        { // then move inside the printed part, so that we don't ooze on the outer wall while retraction, but on the inside of the print.
            assert(extruder != nullptr);
            coord_t innermost_wall_line_width = extruder->settings.get<coord_t>((extruder->settings.get<size_t>("wall_line_count") > 1) ? "wall_line_width_x" : "wall_line_width_0");
            if (layer_nr == 0)
            {
                innermost_wall_line_width *= extruder->settings.get<Ratio>("initial_layer_line_width_factor");
            }
            moveInsideCombBoundary(innermost_wall_line_width);
        }
		path->retract = retraction_enable;
		path->perform_z_hop = retraction_enable && extruder->settings.get<bool>("retraction_hop_enabled");
    }

    // must start new travel path as retraction can be enabled or not depending on path length, etc.
    forceNewPathStart();

    GCodePath& ret = addTravel_simple(p, path);
    was_inside = is_inside;
    return ret;
}

GCodePath& LayerPlan::addTravel_simple(Point p, GCodePath* path)
{
    bool is_first_travel_of_layer = ! static_cast<bool>(last_planned_position);
    if (is_first_travel_of_layer)
    { // spiralize calls addTravel_simple directly as the first travel move in a layer
        first_travel_destination = p;
        first_travel_destination_is_inside = is_inside;
    }
    if (path == nullptr)
    {
        path = getLatestPathWithConfig(configs_storage.travel_config_per_extruder[getExtruder()], SpaceFillType::None);
    }
    path->points.push_back(p);
    last_planned_position = p;
    return *path;
}

void LayerPlan::planPrime(const float& prime_blob_wipe_length)
{
    forceNewPathStart();
    GCodePath& prime_travel = addTravel_simple(getLastPlannedPositionOrStartingPosition() + Point(0, MM2INT(prime_blob_wipe_length)));
    prime_travel.retract = false;
    prime_travel.perform_z_hop = false;
    prime_travel.perform_prime = true;
    forceNewPathStart();
}

void LayerPlan::addExtrusionMove(Point p, const GCodePathConfig& config, SpaceFillType space_fill_type, const Ratio& flow, const Ratio width_factor, bool spiralize, Ratio speed_factor, double fan_speed)
{
    GCodePath* path = getLatestPathWithConfig(config, space_fill_type, flow, width_factor, spiralize, speed_factor);
    path->points.push_back(p);
    path->setFanSpeed(fan_speed);
    if (! static_cast<bool>(first_extrusion_acc_jerk))
    {
        first_extrusion_acc_jerk = std::make_pair(path->config->getAcceleration(), path->config->getJerk());
    }
    last_planned_position = p;
}

void LayerPlan::addPolygon(ConstPolygonRef polygon, int start_idx, const bool backwards, const GCodePathConfig& config, coord_t wall_0_wipe_dist, bool spiralize, const Ratio& flow_ratio, bool always_retract)
{
    constexpr Ratio width_ratio = 1.0_r; // Not printed with variable line width.
    Point p0 = polygon[start_idx];
    addTravel(p0, always_retract);
    const int direction = backwards ? -1 : 1;
    for (size_t point_idx = 1; point_idx < polygon.size(); point_idx++)
    {
        Point p1 = polygon[(start_idx + point_idx * direction + polygon.size()) % polygon.size()];
        addExtrusionMove(p1, config, SpaceFillType::Polygons, flow_ratio, width_ratio, spiralize);
        p0 = p1;
    }
    if (polygon.size() > 2)
    {
        const Point& p1 = polygon[start_idx];
        addExtrusionMove(p1, config, SpaceFillType::Polygons, flow_ratio, width_ratio, spiralize);

        if (wall_0_wipe_dist > 0)
        { // apply outer wall wipe
            p0 = polygon[start_idx];
            int distance_traversed = 0;
            for (size_t point_idx = 1;; point_idx++)
            {
                Point p1 = polygon[(start_idx + point_idx * direction + polygon.size()) % polygon.size()];
                int p0p1_dist = vSize(p1 - p0);
                if (distance_traversed + p0p1_dist >= wall_0_wipe_dist)
                {
                    Point vector = p1 - p0;
                    Point half_way = p0 + normal(vector, wall_0_wipe_dist - distance_traversed);
                    addTravel_simple(half_way);
                    break;
                }
                else
                {
                    addTravel_simple(p1);
                    distance_traversed += p0p1_dist;
                }
                p0 = p1;
            }
            forceNewPathStart();
        }
    }
    else
    {
        LOGW("line added as polygon! (LayerPlan)");
    }
}

void LayerPlan::addPolygonsByOptimizer(const Polygons& polygons,
                                       const GCodePathConfig& config,
                                       const ZSeamConfig& z_seam_config,
                                       coord_t wall_0_wipe_dist,
                                       bool spiralize,
                                       const Ratio flow_ratio,
                                       bool always_retract,
                                       bool reverse_order,
                                       const std::optional<Point> start_near_location)
{
    if (polygons.empty())
    {
        return;
    }
    PathOrderOptimizer<ConstPolygonPointer> orderOptimizer(start_near_location ? *start_near_location: getLastPlannedPositionOrStartingPosition(), z_seam_config);
    for (size_t poly_idx = 0; poly_idx < polygons.size(); poly_idx++)
    {
        orderOptimizer.addPolygon(polygons[poly_idx]);
    }
    orderOptimizer.optimize();

#if 1
    if (need_smart_brim)
    {      
        for (int i = 2; i < orderOptimizer.paths.size(); i++)
        {
            const PathOrderPath<ConstPolygonPointer>& path = orderOptimizer.paths[i];
            PathOrderPath<ConstPolygonPointer> pre_path = orderOptimizer.paths[i - 1];
            coord_t dis = vSize(pre_path.converted->at(pre_path.start_vertex) - path.converted->at(path.start_vertex));
            if (i == orderOptimizer.paths.size() - 1)
            {
                orderOptimizer.paths[i - 1] = std::move(orderOptimizer.paths[i]);
                orderOptimizer.paths[i] = std::move(pre_path);
            }
            else if(dis > MM2INT(2.0))
            {
                orderOptimizer.paths[i - 1] = std::move(orderOptimizer.paths[i - 2]);
                orderOptimizer.paths[i - 2] = std::move(pre_path);
            }
        }
    }
#endif

    if (! reverse_order)
    {
        for (const PathOrderPath<ConstPolygonPointer>& path : orderOptimizer.paths)
        {
            addPolygon(*path.vertices, path.start_vertex, path.backwards, config, wall_0_wipe_dist, spiralize, flow_ratio, always_retract);
        }
    }
    else
    {
        for (int index = orderOptimizer.paths.size() - 1; index >= 0; --index)
        {
            const PathOrderPath<ConstPolygonPointer>& path = orderOptimizer.paths[index];
            addPolygon(**path.vertices, path.start_vertex, path.backwards, config, wall_0_wipe_dist, spiralize, flow_ratio, always_retract);
        }
    }
}

static constexpr float max_non_bridge_line_volume = MM2INT(100); // limit to accumulated "volume" of non-bridge lines which is proportional to distance x extrusion rate

void LayerPlan::addWallLine(const Point& p0,
                            const Point& p1,
                            const Settings& settings,
                            const GCodePathConfig& non_bridge_config,
                            const GCodePathConfig& bridge_config,
                            float flow,
                            const Ratio width_factor,
                            float& non_bridge_line_volume,
                            Ratio speed_factor,
                            double distance_to_bridge_start,
                            const float overhang_distance)
{
    const coord_t min_line_len = 5; // we ignore lines less than 5um long
    const double acceleration_segment_len = MM2INT(1); // accelerate using segments of this length
    const double acceleration_factor = 0.75; // must be < 1, the larger the value, the slower the acceleration
    const bool spiralize = false;

    const coord_t min_bridge_line_len = settings.get<coord_t>("bridge_wall_min_length");
    const Ratio bridge_wall_coast = settings.get<Ratio>("bridge_wall_coast");
    const bool overhang_bridge_force_cooling = settings.get<bool>("cool_overhang_bridge_force_cooling");
    const Ratio overhang_threshold = settings.get<Ratio>("cool_overhang_threshold");
    const Ratio overhang_fan_speed = settings.get<Ratio>("cool_overhang_fan_speed");

    std::vector<std::pair<float, float>> speed_sections = getOverhangSpeedSections();

    Point cur_point = p0;

    // helper function to add a single non-bridge line

    // If the line precedes a bridge line, it may be coasted to reduce the nozzle pressure before the bridge is reached

    // alternatively, if the line follows a bridge line, it may be segmented and the print speed gradually increased to reduce under-extrusion

    auto getSpeedFactor = [&](const Point& _p1)
    {
        if (overhang_distance < 0 || speed_sections.empty())
            return speed_factor;
        Ratio result_factor = getSpeedFactorP(_p1, overhang_distance, non_bridge_config.getSpeed(), speed_sections);
        return result_factor;
    };

    auto getFanSpeed = [&](const GCodePathConfig& config)
    {
        if (!overhang_bridge_force_cooling)
            return GCodePathConfig::FAN_SPEED_DEFAULT;
        if (config.isBridgePath() || overhang_distance > overhang_threshold * config.getLineWidth())
            return double(overhang_fan_speed * 100.);
        return GCodePathConfig::FAN_SPEED_DEFAULT;;
    };

    auto addNonBridgeLine = [&](const Point& line_end, bool changFlow = true)
    {
        coord_t distance_to_line_end = vSize(cur_point - line_end);

        while (distance_to_line_end > min_line_len)
        {
            // if we are accelerating after a bridge line, the segment length is less than the whole line length
            Point segment_end = (speed_factor == 1 || distance_to_line_end < acceleration_segment_len) ? line_end : cur_point + (line_end - cur_point) * acceleration_segment_len / distance_to_line_end;

            // flow required for the next line segment - when accelerating after a bridge segment, the flow is increased in inverse proportion to the speed_factor
            // so the slower the feedrate, the greater the flow - the idea is to get the extruder back to normal pressure as quickly as possible
            const float segment_flow = changFlow ? ((speed_factor < 1) ? flow * (1 / speed_factor) : flow) : flow;

            // if a bridge is present in this wall, this particular segment may need to be partially or wholely coasted
            if (distance_to_bridge_start > 0)
            {
                // speed_flow_factor approximates how the extrusion rate alters between the non-bridge wall line and the following bridge wall line
                // if the extrusion rates are the same, its value will be 1, if the bridge config extrusion rate is < the non-bridge config extrusion rate, the value is < 1

                const Ratio speed_flow_factor((bridge_config.getSpeed() * bridge_config.getFlowRatio()) / (non_bridge_config.getSpeed() * non_bridge_config.getFlowRatio()));

                // coast distance is proportional to distance, speed and flow of non-bridge segments just printed and is throttled by speed_flow_factor
                const double coast_dist = std::min(non_bridge_line_volume, max_non_bridge_line_volume) * (1 - speed_flow_factor) * bridge_wall_coast / 40;

                if ((distance_to_bridge_start - distance_to_line_end) <= coast_dist)
                {
                    // coast takes precedence over acceleration
                    segment_end = line_end;
                }

                const coord_t len = vSize(cur_point - segment_end);
                if (coast_dist > 0 && ((distance_to_bridge_start - len) <= coast_dist))
                {
                    if ((len - coast_dist) > min_line_len)
                    {
                        // segment is longer than coast distance so extrude using non-bridge config to start of coast
                        addExtrusionMove(segment_end + coast_dist * (cur_point - segment_end) / len, non_bridge_config, SpaceFillType::Polygons, segment_flow, width_factor, spiralize, speed_factor);
                    }
                    // then coast to start of bridge segment
                    constexpr Ratio flow = 0.0_r; // Coasting has no flow rate.
                    addExtrusionMove(segment_end, non_bridge_config, SpaceFillType::Polygons, flow, width_factor, spiralize, speed_factor);
                }
                else
                {
                    // no coasting required, just normal segment using non-bridge config
                    addExtrusionMove(segment_end,
                                     non_bridge_config,
                                     SpaceFillType::Polygons,
                                     segment_flow,
                                     width_factor,
                                     spiralize,
                                     std::min(getSpeedFactor(segment_end), speed_factor),
                                     getFanSpeed(non_bridge_config));
                }

                distance_to_bridge_start -= len;
            }
            else
            {
                // no coasting required, just normal segment using non-bridge config
                addExtrusionMove(segment_end,
                                 non_bridge_config,
                                 SpaceFillType::Polygons,
                                 segment_flow,
                                 width_factor,
                                 spiralize,
                                 std::min(getSpeedFactor(segment_end), speed_factor),
                                 getFanSpeed(non_bridge_config));
            }
            non_bridge_line_volume += vSize(cur_point - segment_end) * segment_flow * width_factor * speed_factor * non_bridge_config.getSpeed();
            cur_point = segment_end;
            speed_factor = 1 - (1 - speed_factor) * acceleration_factor;
            if (speed_factor >= 0.9)
            {
                speed_factor = 1;
            }
            distance_to_line_end = vSize(cur_point - line_end);
        }
    };

    if (bridge_wall_mask.empty())
    {
        // no bridges required
        addExtrusionMove(p1,
                        non_bridge_config,
                        SpaceFillType::Polygons,
                        flow,
                        width_factor, 
                        spiralize, 
                        std::min(getSpeedFactor(p1), speed_factor),
                        getFanSpeed(non_bridge_config));
    }
    else
    {
        // bridges may be required
        if (PolygonUtils::polygonCollidesWithLineSegment(bridge_wall_mask, p0, p1))
        {
            // the line crosses the boundary between supported and non-supported regions so one or more bridges are required

            // determine which segments of the line are bridges

            Polygons line_polys;
            line_polys.addLine(p0, p1);
            constexpr bool restitch = false; // only a single line doesn't need stitching
            line_polys = bridge_wall_mask.intersectionPolyLines(line_polys, restitch);

            // line_polys now contains the wall lines that need to be printed using bridge_config

            while (line_polys.size() > 0)
            {
                // find the bridge line segment that's nearest to the current point
                int nearest = 0;
                float smallest_dist2 = vSize2f(cur_point - line_polys[0][0]);
                for (unsigned i = 1; i < line_polys.size(); ++i)
                {
                    float dist2 = vSize2f(cur_point - line_polys[i][0]);
                    if (dist2 < smallest_dist2)
                    {
                        nearest = i;
                        smallest_dist2 = dist2;
                    }
                }
                ConstPolygonRef bridge = line_polys[nearest];

                // set b0 to the nearest vertex and b1 the furthest
                Point b0 = bridge[0];
                Point b1 = bridge[1];

                if (vSize2f(cur_point - b1) < vSize2f(cur_point - b0))
                {
                    // swap vertex order
                    b0 = bridge[1];
                    b1 = bridge[0];
                }

                const double bridge_line_len = vSize(b1 - b0);
                if (bridge_line_len < min_bridge_line_len)
                {
                    // treat the short bridge line just like a normal line
                    GCodePath path = extruder_plans.back().paths.back();
                    addExtrusionMove(b1,
                        non_bridge_config, 
                        SpaceFillType::Polygons,
                        flow,
                        path.width_factor, 
                        spiralize, 
                        std::min(getSpeedFactor(b1), path.speed_factor),
                        getFanSpeed(non_bridge_config));
                    cur_point = b1;
                }
                else
                {
                    // extrude using non_bridge_config to the start of the next bridge segment
                    addNonBridgeLine(b0);

                    // extrude using bridge_config to the end of the next bridge segment
                    if (bridge_line_len > min_line_len)
                    {
                        addExtrusionMove(b1, bridge_config, SpaceFillType::Polygons, flow, width_factor, false, 1.0_r, getFanSpeed(bridge_config));
                        non_bridge_line_volume = 0;
                        cur_point = b1;
                        // after a bridge segment, start slow and accelerate to avoid under-extrusion due to extruder lag
                        speed_factor = std::max(std::min(Ratio(bridge_config.getSpeed() / non_bridge_config.getSpeed()), 1.0_r), 0.8_r);
                    }
                }

                // finished with this segment
                line_polys.remove(nearest);
            }

            // if we haven't yet reached p1, fill the gap with non_bridge_config line
            addNonBridgeLine(p1);
        }
        else if (bridge_wall_mask.inside(p0, true) && vSize(p0 - p1) >= min_bridge_line_len && bridge_config.getSpeed() < non_bridge_config.getSpeed() * std::min(getSpeedFactor(p1), speed_factor))
        {
            // both p0 and p1 must be above air (the result will be ugly!)
            addExtrusionMove(p1, bridge_config, SpaceFillType::Polygons, flow, width_factor, false, 1.0_r, getFanSpeed(bridge_config));
            non_bridge_line_volume = 0;
        }
        else
        {
            // no part of the line is above air or the line is too short to print as a bridge line
            addNonBridgeLine(p1, false);
        }
    }
}

void LayerPlan::addWall(ConstPolygonRef wall, int start_idx, const Settings& settings, const GCodePathConfig& non_bridge_config, const GCodePathConfig& bridge_config, coord_t wall_0_wipe_dist, float flow_ratio, bool always_retract)
{
    // TODO: Deprecated in favor of ExtrusionJunction version below.
    if (wall.size() < 3)
    {
        LOGW("Point, or line added as (polygon) wall sequence! (LayerPlan)");
    }

    constexpr size_t dummy_perimeter_id = 0; // <-- Here, don't care about which perimeter any more.
    const coord_t nominal_line_width = non_bridge_config.getLineWidth(); // <-- The line width which it's 'supposed to' be will be used to adjust the flow ratio each time, this'll give a flow-ratio-multiplier of 1.

    ExtrusionLine ewall;
    std::for_each(wall.begin(), wall.end(), [&dummy_perimeter_id, &nominal_line_width, &ewall](const Point& p) { ewall.emplace_back(p, nominal_line_width, dummy_perimeter_id); });
    ewall.emplace_back(*wall.begin(), nominal_line_width, dummy_perimeter_id);
    constexpr bool is_closed = true;
    constexpr bool is_reversed = false;
    constexpr bool is_linked_path = false;
    addWall(ewall, start_idx, settings, non_bridge_config, bridge_config, wall_0_wipe_dist, flow_ratio, always_retract, is_closed, is_reversed, is_linked_path);
}

void LayerPlan::addWall(const ExtrusionLine& wall,
                        int start_idx,
                        const Settings& settings,
                        const GCodePathConfig& non_bridge_config,
                        const GCodePathConfig& bridge_config,
                        coord_t wall_0_wipe_dist,
                        float flow_ratio,
                        bool always_retract,
                        const bool is_closed,
                        const bool is_reversed,
                        const bool is_linked_path)
{
    if (wall.empty())
    {
        return;
    }
    if (is_closed)
    {
        // make sure wall start point is not above air!
        start_idx = locateFirstSupportedVertex(wall, start_idx);
    }

    float non_bridge_line_volume = max_non_bridge_line_volume; // assume extruder is fully pressurised before first non-bridge line is output
    double speed_factor = 1.0; // start first line at normal speed
    coord_t distance_to_bridge_start = 0; // will be updated before each line is processed

    const coord_t min_bridge_line_len = settings.get<coord_t>("bridge_wall_min_length");

    const Ratio nominal_line_width_multiplier = 1.0 / Ratio(non_bridge_config.getLineWidth()); // we multiply the flow with the actual wanted line width (for that junction), and then multiply with this

#define COVERBRIDGEDISTANCE 1
#if COVERBRIDGEDISTANCE
    const Ratio bridge_wall_coast = settings.get<Ratio>("bridge_wall_coast");
    const Ratio speed_flow_factor((bridge_config.getSpeed() * bridge_config.getFlowRatio()) / (non_bridge_config.getSpeed() * non_bridge_config.getFlowRatio()));
    const double coast_dist = max_non_bridge_line_volume * (1 - speed_flow_factor) * bridge_wall_coast / 40;
    std::vector<coord_t> distance_data;
    distance_data.resize(wall.size(), 0);
    auto computeDistanceToBridgeStartForAll = [&]()->void
    {
        if (bridge_wall_mask.empty())
            return ;
        const int direction = is_reversed ? -1 : 1;
        const size_t max_index = wall.size();
        for (size_t point_idx = 0; point_idx < max_index; point_idx += direction)
        {
            ExtrusionJunction p0 = wall[point_idx];
            const ExtrusionJunction& p1 = wall[(point_idx + 1) % wall.size()];     
            if (bridge_wall_mask.inside(p0.p, true))
            {
                continue;
            }
            coord_t p0_to_bridge_start_disteance = 0;
            bool findBridge = false;
            if (PolygonUtils::polygonCollidesWithLineSegment(bridge_wall_mask, p0.p, p1.p))
            {
                // the line crosses the boundary between supported and non-supported regions so it will contain one or more bridge segments

                // determine which segments of the line are bridges

                Polygons line_polys;
                line_polys.addLine(p0.p, p1.p);
                constexpr bool restitch = false; // only a single line doesn't need stitching
                line_polys = bridge_wall_mask.intersectionPolyLines(line_polys, restitch);
                
                while (line_polys.size() > 0)
                {
                    // find the bridge line segment that's nearest to p0
                    int nearest = 0;
                    float smallest_dist2 = vSize2f(p0.p - line_polys[0][0]);
                    for (unsigned i = 1; i < line_polys.size(); ++i)
                    {
                        float dist2 = vSize2f(p0.p - line_polys[i][0]);
                        if (dist2 < smallest_dist2)
                        {
                            nearest = i;
                            smallest_dist2 = dist2;
                        }
                    }
                    ConstPolygonRef bridge = line_polys[nearest];

                    // set b0 to the nearest vertex and b1 the furthest
                    Point b0 = bridge[0];
                    Point b1 = bridge[1];

                    if (vSize2f(p0.p - b1) < vSize2f(p0.p - b0))
                    {
                        // swap vertex order
                        b0 = bridge[1];
                        b1 = bridge[0];
                    }

                    p0_to_bridge_start_disteance += vSize(b0 - p0.p);

                    const double bridge_line_len = vSize(b1 - b0);

                    if (bridge_line_len >= min_bridge_line_len)
                    {
                        // job done, we have found the first bridge line
                        findBridge = true;
                        break;
                    }

                    p0_to_bridge_start_disteance += bridge_line_len;

                    // finished with this segment
                    line_polys.remove(nearest);
                }
            }
            
            if (findBridge && p0_to_bridge_start_disteance > 0)
            {
                int currentIdx = point_idx;
                coord_t dist_record = p0_to_bridge_start_disteance;
                if (distance_data[currentIdx] == 0)
                    distance_data[currentIdx] = dist_record;
                while (dist_record < coast_dist)
                {
                    int preIdx = (currentIdx - 1 + wall.size()) % wall.size();
                    const ExtrusionJunction& pre = wall[preIdx];
                    dist_record += vSize(pre.p - p0.p);
                    if (distance_data[preIdx] == 0)
                        distance_data[preIdx] = dist_record;
                    else
                        break;
                    p0 = pre;
                    currentIdx--;
                }
            }
        }
    };
    computeDistanceToBridgeStartForAll();
#endif

    // helper function to calculate the distance from the start of the current wall line to the first bridge segment

    auto computeDistanceToBridgeStart = [&](unsigned current_index)
    {
#if COVERBRIDGEDISTANCE
        distance_to_bridge_start = distance_data[current_index];

        return;
#endif

        distance_to_bridge_start = 0;

        if (! bridge_wall_mask.empty())
        {
            // there is air below the part so iterate through the lines that have not yet been output accumulating the total distance to the first bridge segment
            for (unsigned point_idx = current_index; point_idx < wall.size(); ++point_idx)
            {
                const ExtrusionJunction& p0 = wall[point_idx];
                const ExtrusionJunction& p1 = wall[(point_idx + 1) % wall.size()];

                if (PolygonUtils::polygonCollidesWithLineSegment(bridge_wall_mask, p0.p, p1.p))
                {
                    // the line crosses the boundary between supported and non-supported regions so it will contain one or more bridge segments

                    // determine which segments of the line are bridges

                    Polygons line_polys;
                    line_polys.addLine(p0.p, p1.p);
                    constexpr bool restitch = false; // only a single line doesn't need stitching
                    line_polys = bridge_wall_mask.intersectionPolyLines(line_polys, restitch);

                    while (line_polys.size() > 0)
                    {
                        // find the bridge line segment that's nearest to p0
                        int nearest = 0;
                        float smallest_dist2 = vSize2f(p0.p - line_polys[0][0]);
                        for (unsigned i = 1; i < line_polys.size(); ++i)
                        {
                            float dist2 = vSize2f(p0.p - line_polys[i][0]);
                            if (dist2 < smallest_dist2)
                            {
                                nearest = i;
                                smallest_dist2 = dist2;
                            }
                        }
                        ConstPolygonRef bridge = line_polys[nearest];

                        // set b0 to the nearest vertex and b1 the furthest
                        Point b0 = bridge[0];
                        Point b1 = bridge[1];

                        if (vSize2f(p0.p - b1) < vSize2f(p0.p - b0))
                        {
                            // swap vertex order
                            b0 = bridge[1];
                            b1 = bridge[0];
                        }

                        distance_to_bridge_start += vSize(b0 - p0.p);

                        const double bridge_line_len = vSize(b1 - b0);

                        if (bridge_line_len >= min_bridge_line_len)
                        {
                            // job done, we have found the first bridge line
                            return;
                        }

                        distance_to_bridge_start += bridge_line_len;

                        // finished with this segment
                        line_polys.remove(nearest);
                    }
                }
                else if (! bridge_wall_mask.inside(p0.p, true))
                {
                    // none of the line is over air
                    distance_to_bridge_start += vSize(p1.p - p0.p);
                }
            }

            // we have got all the way to the end of the wall without finding a bridge segment so disable coasting by setting distance_to_bridge_start back to 0

            distance_to_bridge_start = 0;
        }
    };

    bool first_line = true;
    const coord_t small_feature_max_length = settings.get<coord_t>("small_feature_max_length");
    Ratio small_feature_speed_factor = settings.get<Ratio>((layer_nr == 0) ? "small_feature_speed_factor_0" : "small_feature_speed_factor");
    if (small_feature_speed_factor == 0) small_feature_speed_factor = 1.0;
    bool is_small_feature = (small_feature_max_length > 0) && cura52::shorterThan(wall, small_feature_max_length);
    const Velocity min_speed = fan_speed_layer_time_settings_per_extruder[getLastPlannedExtruderTrain()->extruder_nr].cool_min_speed;
    const coord_t max_area_deviation = std::max(settings.get<int>("meshfix_maximum_extrusion_area_deviation"), 1); // Square micrometres!
    const coord_t max_resolution = std::max(settings.get<coord_t>("meshfix_maximum_resolution"), coord_t(1));
    const double small_feature_fan_speed = settings.get<double>("cool_small_feature_fan_speed_factor");

    Ratio smaller_level_factor = 0;
    coord_t big_length = small_feature_max_length;
    coord_t smaller_length = 0;
    int64_t wall_length = wall.getLength();
    if (settings.get<bool>("set_small_feature_grading") && wall_length > 0 && non_bridge_config.type != PrintFeatureType::Support)
    {
        int level = -1;
        if (wall_length < settings.get<coord_t>("special_small_feature_max_length_4"))
        {
            level = 4;
            small_feature_speed_factor = settings.get<Ratio>((layer_nr == 0) ? "special_small_feature_speed_factor_0_4" : "special_small_feature_speed_factor_4");
            extruder_plans.back().cool_min_layer_time_correct = settings.get<Duration>("special_cool_min_layer_time_4");
            is_small_feature = true;
        }
        else if (wall_length < settings.get<coord_t>("special_small_feature_max_length_3"))
        {
            level = 3;
            small_feature_speed_factor = settings.get<Ratio>((layer_nr == 0) ? "special_small_feature_speed_factor_0_3" : "special_small_feature_speed_factor_3");
            extruder_plans.back().cool_min_layer_time_correct = settings.get<Duration>("special_cool_min_layer_time_3");
            is_small_feature = true;
        }
        else if (wall_length < settings.get<coord_t>("special_small_feature_max_length_2"))
        {
            level = 2;
            small_feature_speed_factor = settings.get<Ratio>((layer_nr == 0) ? "special_small_feature_speed_factor_0_2" : "special_small_feature_speed_factor_2");
            extruder_plans.back().cool_min_layer_time_correct = settings.get<Duration>("special_cool_min_layer_time_2");
            is_small_feature = true;
        }
        else if (wall_length < settings.get<coord_t>("special_small_feature_max_length_1"))
        {
            level = 1;
            small_feature_speed_factor = settings.get<Ratio>((layer_nr == 0) ? "special_small_feature_speed_factor_0_1" : "special_small_feature_speed_factor_1");
            extruder_plans.back().cool_min_layer_time_correct = settings.get<Duration>("special_cool_min_layer_time_1");
            is_small_feature = true;
        }
        if (level > 0 && level < 4)
        {
            std::string factor_name = (layer_nr == 0) ? "special_small_feature_speed_factor_0_%d" : "special_small_feature_speed_factor_%d";
            factor_name.replace(factor_name.find("%d"), 2, std::to_string(level + 1));
            smaller_level_factor = settings.get<Ratio>(factor_name);
            std::string length_name = "special_small_feature_max_length_%d";
            length_name.replace(length_name.find("%d"), 2, std::to_string(level));
            big_length = settings.get<coord_t>(length_name);
            length_name.replace(length_name.find_last_of("_") + 1, 1, std::to_string(level + 1));
            smaller_length = settings.get<coord_t>(length_name);
        }
    }
    if (is_small_feature && wall_length > 0 && small_feature_speed_factor > 0 && small_feature_speed_factor < 1.0)
    {
        small_feature_speed_factor = ((wall_length - smaller_length) / (float)(big_length - smaller_length)) * (small_feature_speed_factor - smaller_level_factor) + smaller_level_factor;
        small_feature_speed_factor = std::max(small_feature_speed_factor, Ratio(min_speed / non_bridge_config.getSpeed()));
    }  
    
    
    ExtrusionJunction p0 = wall[start_idx];

    const int direction = is_reversed ? -1 : 1;
    const size_t max_index = is_closed ? wall.size() + 1 : wall.size();
    for (size_t point_idx = 1; point_idx < max_index; point_idx++)
    {
        const ExtrusionJunction& p1 = wall[(wall.size() + start_idx + point_idx * direction) % wall.size()];

        if (! bridge_wall_mask.empty())
        {
            computeDistanceToBridgeStart((wall.size() + start_idx + point_idx * direction - 1) % wall.size());
        }

        if (first_line)
        {
            addTravel(p0.p, always_retract);
            first_line = false;
        }

        /*
        If the line has variable width, break it up into pieces with the
        following constraints:
        - Each piece must be smaller than the Maximum Resolution setting.
        - The difference between the trapezoidal shape of the line and the
          rectangular shape of the line may not exceed the Maximum Extrusion
          Area Deviation setting, unless required by the first constraint.
        Since breaking up a line segment into N pieces (each with averaged
        width) divides the area deviation by N, we can simply check how many
        pieces we'd want to get low enough deviation, then check if each piece
        is not too short at the end.
        */
        const coord_t delta_line_width = p1.w - p0.w;
        const Point line_vector = p1.p - p0.p;
        const coord_t line_length = vSize(line_vector);
        /*
        Calculate how much the line would deviate from the trapezoidal shape if printed at average width.
        This formula is:
        - Half the length times half the delta width, for the rectangular shape of the deviating side.
        - Half of that because the ideal line width is trapezoidal, making the deviating part triangular.
        - Double of that because the deviation occurs on both sides of the idealised line width.
        This results in delta_line_width / 2 * line_length / 2 / 2 * 2 == delta_line_width * line_length / 4.
        */
        const coord_t line_area_deviation = std::abs(delta_line_width) * line_length / 4;
        const size_t pieces_limit_deviation = round_up_divide(line_area_deviation, max_area_deviation); // How many pieces we'd need to stay beneath the max area deviation.
        const size_t pieces_limit_resolution = line_length / max_resolution; // Round down this time, to not exceed the maximum resolution.
        const size_t pieces = std::max(size_t(1), std::min(pieces_limit_deviation, pieces_limit_resolution)); // Resolution overrides deviation, if resolution is a constraint.
        const coord_t piece_length = round_divide(line_length, pieces);

        for (size_t piece = 0; piece < pieces; ++piece)
        {
            const float average_progress = (float(piece) + 0.5) / pieces; // How far along this line to sample the line width in the middle of this piece.
            const coord_t line_width = p0.w + average_progress * delta_line_width;
            const Point destination = p0.p + normal(line_vector, piece_length * (piece + 1));
            if (is_small_feature)
            {
                std::vector<std::pair<float, float>> speed_sections = getOverhangSpeedSections();
                Ratio result_speed_factor = small_feature_speed_factor;
                if (p1.overhang_distance > 0 && !speed_sections.empty())
                    result_speed_factor = std::min(small_feature_speed_factor, getSpeedFactorP(destination, p1.overhang_distance, non_bridge_config.getSpeed(), speed_sections));
                constexpr bool spiralize = false;
                double fan_speed_s = getLayerNr() < settings.get<LayerIndex>("cool_fan_full_layer") ? non_bridge_config.getFanSpeed() : small_feature_fan_speed;
                addExtrusionMove(destination, non_bridge_config, SpaceFillType::Polygons, flow_ratio, line_width* nominal_line_width_multiplier, spiralize, result_speed_factor, fan_speed_s);
            }
            else
            {
                const Point origin = p0.p + normal(line_vector, piece_length * piece);
                addWallLine(origin, destination, settings, non_bridge_config, bridge_config, flow_ratio, line_width * nominal_line_width_multiplier, non_bridge_line_volume, speed_factor, distance_to_bridge_start, p1.overhang_distance);
            }
        }

        p0 = p1;
    }

    if (wall.size() >= 2)
    {
        if (! bridge_wall_mask.empty())
        {
            computeDistanceToBridgeStart((start_idx + wall.size() - 1) % wall.size());
        }

        if (wall_0_wipe_dist > 0 && ! is_linked_path)
        { // apply outer wall wipe
            p0 = wall[start_idx];
            int distance_traversed = 0;
            for (unsigned int point_idx = 1;; point_idx++)
            {
                if (point_idx > wall.size() && distance_traversed == 0) // Wall has a total circumference of 0. This loop would never end.
                {
                    break; // No wipe if the wall has no circumference.
                }
                ExtrusionJunction p1 = wall[(start_idx + point_idx) % wall.size()];
                int p0p1_dist = vSize(p1 - p0);
                if (distance_traversed + p0p1_dist >= wall_0_wipe_dist)
                {
                    Point vector = p1.p - p0.p;
                    Point half_way = p0.p + normal(vector, wall_0_wipe_dist - distance_traversed);
                    addTravel_simple(half_way);
                    break;
                }
                else
                {
                    addTravel_simple(p1.p);
                    distance_traversed += p0p1_dist;
                }
                p0 = p1;
            }
            forceNewPathStart();
        }
    }
    else
    {
        LOGW("Point added as wall sequence! (LayerPlan)");
    }
}

void LayerPlan::addInfillWall(const ExtrusionLine& wall, const GCodePathConfig& path_config, bool force_retract)
{
    assert(("All empty walls should have been filtered at this stage", ! wall.empty()));
    ExtrusionJunction junction{ *wall.begin() };
    addTravel(junction.p, force_retract);

    for (const auto& junction_n : wall)
    {
        const Ratio width_factor = junction_n.w / Ratio(path_config.getLineWidth());
        constexpr SpaceFillType space_fill_type = SpaceFillType::Polygons;
        constexpr Ratio flow = 1.0_r;
        addExtrusionMove(junction_n.p, path_config, space_fill_type, flow, width_factor);
        junction = junction_n;
    }
}

void LayerPlan::addWalls(const Polygons& walls,
                         const Settings& settings,
                         const GCodePathConfig& non_bridge_config,
                         const GCodePathConfig& bridge_config,
                         const ZSeamConfig& z_seam_config,
                         coord_t wall_0_wipe_dist,
                         float flow_ratio,
                         bool always_retract)
{
    // TODO: Deprecated in favor of ExtrusionJunction version below.
    PathOrderOptimizer<ConstPolygonPointer> orderOptimizer(getLastPlannedPositionOrStartingPosition(), z_seam_config);
    for (size_t poly_idx = 0; poly_idx < walls.size(); poly_idx++)
    {
        orderOptimizer.addPolygon(walls[poly_idx]);
    }
    orderOptimizer.optimize();
    for (const PathOrderPath<ConstPolygonPointer>& path : orderOptimizer.paths)
    {
        addWall(**path.vertices, path.start_vertex, settings, non_bridge_config, bridge_config, wall_0_wipe_dist, flow_ratio, always_retract);
    }
}


void LayerPlan::addLinesByOptimizer(const Polygons& polygons,
                                    const GCodePathConfig& config,
                                    const SpaceFillType space_fill_type,
                                    const bool enable_travel_optimization,
                                    const coord_t wipe_dist,
                                    const Ratio flow_ratio,
                                    const std::optional<Point> near_start_location,
                                    const double fan_speed,
                                    const bool reverse_print_direction)
{
    Polygons boundary;
    if (enable_travel_optimization && ! comb_boundary_minimum.empty())
    {
        // use the combing boundary inflated so that all infill lines are inside the boundary
        int dist = 0;
        if (layer_nr >= 0)
        {
            // determine how much the skin/infill lines overlap the combing boundary
            for (const SliceMeshStorage& mesh : storage.meshes)
            {
                const coord_t overlap = std::max(mesh.settings.get<coord_t>("skin_overlap_mm"), mesh.settings.get<coord_t>("infill_overlap_mm"));
                if (overlap > dist)
                {
                    dist = overlap;
                }
            }
            dist += 100; // ensure boundary is slightly outside all skin/infill lines
        }
        boundary.add(comb_boundary_minimum.offset(dist));
        // simplify boundary to cut down processing time
        boundary = Simplify(MM2INT(0.1), MM2INT(0.1), 0).polygon(boundary);
    }
    constexpr bool detect_loops = true;
    PathOrderOptimizer<ConstPolygonPointer> order_optimizer(near_start_location.value_or(getLastPlannedPositionOrStartingPosition()), ZSeamConfig(), detect_loops, &boundary, reverse_print_direction);
    for (size_t line_idx = 0; line_idx < polygons.size(); line_idx++)
    {
        order_optimizer.addPolyline(polygons[line_idx]);
    }
    order_optimizer.optimize();

    addLinesInGivenOrder(order_optimizer.paths, config, space_fill_type, wipe_dist, flow_ratio, fan_speed);
}


void LayerPlan::addLinesInGivenOrder(const std::vector<PathOrderPath<ConstPolygonPointer>>& paths, const GCodePathConfig& config, const SpaceFillType space_fill_type, const coord_t wipe_dist, const Ratio flow_ratio, const double fan_speed)
{
    coord_t min_travel_speed_length = storage.meshes.back().settings.get<coord_t>("speed_min_travel_speed_length");
    coord_t half_line_width = config.isBridgePath() ? 2.0 * config.getLineWidth() :
        (0.5 * config.getLineWidth() > min_travel_speed_length ? 0.5 * config.getLineWidth() : min_travel_speed_length);
    coord_t line_width_2 = half_line_width * half_line_width;
    for (size_t order_idx = 0; order_idx < paths.size(); order_idx++)
    {
        const PathOrderPath<ConstPolygonPointer>& path = paths[order_idx];
        ConstPolygonRef polyline = *path.vertices;
        const size_t start_idx = path.start_vertex;
        assert(start_idx == 0 || start_idx == polyline.size() - 1 || path.is_closed);
        const Point start = polyline[start_idx];

        if (vSize2(getLastPlannedPositionOrStartingPosition() - start) <= line_width_2)
        {
            // Instead of doing a small travel that is shorter than the line width (which is generally done at pretty high jerk & move) do a
            // "fake" extrusion move
            constexpr Ratio flow = 0.0_r;
            constexpr Ratio width_factor = 1.0_r;
            constexpr bool spiralize = false;
            constexpr Ratio speed_factor = 1.0_r;
            addExtrusionMove(start, config, space_fill_type, config.isBridgePath() ? flow_ratio : flow, width_factor, spiralize, speed_factor, fan_speed);
        }
        else
        {
            addTravel(start);
        }

        Point p0 = start;
        for (size_t idx = 0; idx < polyline.size(); idx++)
        {
            size_t point_idx;
            if (path.is_closed)
            {
                point_idx = (start_idx + idx + 1) % polyline.size();
            }
            else if (start_idx == 0)
            {
                point_idx = idx;
            }
            else
            {
                assert(start_idx == polyline.size() - 1);
                point_idx = start_idx - idx;
            }
            Point p1 = polyline[point_idx];

            // ignore line segments that are less than 5uM long
            if (vSize2(p1 - p0) >= MINIMUM_SQUARED_LINE_LENGTH)
            {
                constexpr Ratio width_factor = 1.0_r;
                constexpr bool spiralize = false;
                constexpr Ratio speed_factor = 1.0_r;
                addExtrusionMove(p1, config, space_fill_type, flow_ratio, width_factor, spiralize, speed_factor, fan_speed);
                p0 = p1;
            }
        }

        Point p1 = polyline[(start_idx == 0) ? polyline.size() - 1 : 0];
        p0 = (polyline.size() <= 1) ? p1 : polyline[(start_idx == 0) ? polyline.size() - 2 : 1];

        // Wipe
        if (wipe_dist != 0)
        {
            bool wipe = true;
            int line_width = config.getLineWidth();

            // Don't wipe if current extrusion is too small
            if (polyline.polylineLength() <= line_width * 2)
            {
                wipe = false;
            }

            // Don't wipe if next starting point is very near
            if (wipe && (order_idx < paths.size() - 1))
            {
                const PathOrderPath<ConstPolygonPointer>& next_path = paths[order_idx + 1];
                ConstPolygonRef next_polygon = *next_path.vertices;
                const size_t next_start = next_path.start_vertex;
                const Point& next_p0 = next_polygon[next_start];
                if (vSize2(next_p0 - p1) <= line_width * line_width * 4)
                {
                    wipe = false;
                }
            }

            if (wipe)
            {
                constexpr Ratio flow = 0.0_r;
                constexpr Ratio width_factor = 1.0_r;
                constexpr bool spiralize = false;
                constexpr Ratio speed_factor = 1.0_r;
                addExtrusionMove(p1 + normal(p1 - p0, wipe_dist), config, space_fill_type, flow, width_factor, spiralize, speed_factor, fan_speed);
            }
        }
    }
}

void LayerPlan::addLinesMonotonic(const Polygons& area,
                                  const Polygons& polygons,
                                  const GCodePathConfig& config,
                                  const SpaceFillType space_fill_type,
                                  const AngleRadians monotonic_direction,
                                  const coord_t max_adjacent_distance,
                                  const coord_t exclude_distance,
                                  const coord_t wipe_dist,
                                  const Ratio flow_ratio,
                                  const double fan_speed)
{
    const Polygons exclude_areas = area.tubeShape(exclude_distance, exclude_distance);
    const coord_t exclude_dist2 = exclude_distance * exclude_distance;
    const Point last_position = getLastPlannedPositionOrStartingPosition();

    // First lay all adjacent lines next to each other, to have a sensible input to the monotonic part of the algorithm.
    PathOrderOptimizer<ConstPolygonPointer> line_order(last_position);
    for (const ConstPolygonRef polyline : polygons)
    {
        line_order.addPolyline(polyline);
    }
    line_order.optimize();

    const auto is_inside_exclusion = [&exclude_areas, &exclude_dist2](ConstPolygonRef path) { return vSize2(path[1] - path[0]) < exclude_dist2 && exclude_areas.inside((path[0] + path[1]) / 2); };

    // Order monotonically, except for line-segments which stay in the excluded areas (read: close to the walls) consecutively.
    PathOrderMonotonic<ConstPolygonPointer> order(monotonic_direction, max_adjacent_distance, last_position);
    Polygons left_over;
    bool last_would_have_been_excluded = false;
    for (size_t line_idx = 0; line_idx < line_order.paths.size(); ++line_idx)
    {
        const ConstPolygonRef polyline = *line_order.paths[line_idx].vertices;
        const bool inside_exclusion = is_inside_exclusion(polyline);
        const bool next_would_have_been_included = inside_exclusion && (line_idx < line_order.paths.size() - 1 && is_inside_exclusion(*line_order.paths[line_idx + 1].vertices));
        if (inside_exclusion && last_would_have_been_excluded && next_would_have_been_included)
        {
            left_over.add(polyline);
        }
        else
        {
            order.addPolyline(polyline);
        }
        last_would_have_been_excluded = inside_exclusion;
    }
    order.optimize();

    // Read out and process the monotonically ordered lines.
    addLinesInGivenOrder(order.paths, config, space_fill_type, wipe_dist, flow_ratio, fan_speed);

    // Add all lines in the excluded areas the 'normal' way.
    addLinesByOptimizer(left_over, config, space_fill_type, true, wipe_dist, flow_ratio, getLastPlannedPositionOrStartingPosition(), fan_speed);
}

void LayerPlan::spiralizeWallSlice(const GCodePathConfig& config, ConstPolygonRef wall, ConstPolygonRef last_wall, const int seam_vertex_idx, const int last_seam_vertex_idx, const bool is_top_layer, const bool is_bottom_layer)
{
    const bool smooth_contours = application->scene->current_mesh_group->settings.get<bool>("smooth_spiralized_contours");
    const bool slope_slice = application->scene->current_mesh_group->settings.get<coord_t>("special_slope_slice_angle") != 0;
    constexpr bool spiralize = true; // In addExtrusionMove calls, enable spiralize and use nominal line width.
    constexpr Ratio width_factor = 1.0_r;

    // once we are into the spiral we always start at the end point of the last layer (if any)
    const Point origin = (last_seam_vertex_idx >= 0 && ! is_bottom_layer) ? last_wall[last_seam_vertex_idx] : wall[seam_vertex_idx];
    // NOTE: this used to use addTravel_simple() but if support is being generated then combed travel is required to avoid
    // the nozzle crossing the model on its return from printing the support.
    addTravel(origin);

    if ((!smooth_contours || slope_slice) && last_seam_vertex_idx >= 0)
    {
        // when not smoothing, we get to the (unchanged) outline for this layer as quickly as possible so that the remainder of the
        // outline wall has the correct direction - although this creates a little step, the end result is generally better because when the first
        // outline wall has the wrong direction (due to it starting from the finish point of the last layer) the visual effect is very noticeable
        Point join_first_wall_at = LinearAlg2D::getClosestOnLineSegment(origin, wall[seam_vertex_idx % wall.size()], wall[(seam_vertex_idx + 1) % wall.size()]);
        if (vSize(join_first_wall_at - origin) > 10)
        {
            constexpr Ratio flow = 1.0_r;
            addExtrusionMove(join_first_wall_at, config, SpaceFillType::Polygons, flow, width_factor, spiralize);
        }
    }

    const int n_points = wall.size();
    Polygons last_wall_polygons;
    last_wall_polygons.add(last_wall);
    const int max_dist2 = config.getLineWidth() * config.getLineWidth() * 4; // (2 * lineWidth)^2;

    double total_length = 0.0; // determine the length of the complete wall
    Point p0 = origin;
    for (int wall_point_idx = 1; wall_point_idx <= n_points; ++wall_point_idx)
    {
        const Point& p1 = wall[(seam_vertex_idx + wall_point_idx) % n_points];
        total_length += vSizeMM(p1 - p0);
        p0 = p1;
    }

    if (total_length == 0.0)
    {
        // nothing to do
        return;
    }

    // if this is the bottom layer, avoid creating a big elephants foot by starting with a reduced flow and increasing the flow
    // so that by the time the end of the first spiral is complete the flow is 100% - note that immediately before the spiral
    // is output, the extruder will be printing a normal wall line and so will be fully pressurised so that will tend to keep the
    // flow going

    // if this is the top layer, avoid an abrupt end by printing the same outline again but this time taper the spiral by reducing
    // the flow whilst keeping the same height - once the flow is down to a minimum allowed value, coast a little further

    const double min_bottom_layer_flow = 0.25; // start the bottom spiral at this flow rate
    const double min_top_layer_flow = 0.25; // lowest allowed flow while tapering the last spiral

    double speed_factor = 1; // may be reduced when printing the top layer so as to avoid a jump in extrusion rate as the layer starts

    if (is_top_layer)
    {
        // HACK ALERT - the last layer is approx 50% longer than the previous layer so it should take longer to print but the
        // normal slow down for quick layers mechanism can kick in and speed this layer up (because it is longer) but we prefer
        // the layer to be printed at a similar speed to the previous layer to avoid abrupt changes in extrusion rate so we slow it down

        const FanSpeedLayerTimeSettings& layer_time_settings = extruder_plans.back().fan_speed_layer_time_settings;
        double min_time = layer_time_settings.cool_min_layer_time;
        if (extruder_plans.back().cool_min_layer_time_correct != 0.)
        {
            min_time = extruder_plans.back().cool_min_layer_time_correct;
        }
        const double normal_layer_time = total_length / config.getSpeed();

        // would this layer's speed normally get reduced to satisfy the min layer time?
        if (normal_layer_time < min_time)
        {
            // yes, so the extended version will not get slowed down so much and we want to compensate for that
            const double extended_layer_time = (total_length * (2 - min_top_layer_flow)) / config.getSpeed();

            // modify the speed factor to cancel out the speed increase that would normally happen due to the longer layer time
            speed_factor = normal_layer_time / std::min(extended_layer_time, min_time);
        }
    }

    // extrude to the points following the seam vertex
    // the last point is the seam vertex as the polygon is a loop
    double wall_length = 0.0;
    p0 = origin;
    for (int wall_point_idx = 1; wall_point_idx <= n_points; ++wall_point_idx)
    {
        // p is a point from the current wall polygon
        const Point& p = wall[(seam_vertex_idx + wall_point_idx) % n_points];
        wall_length += vSizeMM(p - p0);
        p0 = p;

        const double flow = (is_bottom_layer) ? (min_bottom_layer_flow + ((1 - min_bottom_layer_flow) * wall_length / total_length)) : 1.0;

        // if required, use interpolation to smooth the x/y coordinates between layers but not for the first spiralized layer
        // as that lies directly on top of a non-spiralized wall with exactly the same outline and not for the last point in each layer
        // because we want that to be numerically exactly the same as the starting point on the next layer (not subject to any rounding)
        if ((smooth_contours && !slope_slice) && ! is_bottom_layer && wall_point_idx < n_points)
        {
            // now find the point on the last wall that is closest to p
            ClosestPolygonPoint cpp = PolygonUtils::findClosest(p, last_wall_polygons);

            // if we found a point and it's not further away than max_dist2, use it
            if (cpp.isValid() && vSize2(cpp.location - p) <= max_dist2)
            {
                // interpolate between cpp.location and p depending on how far we have progressed along wall
                addExtrusionMove(cpp.location + (p - cpp.location) * (wall_length / total_length), config, SpaceFillType::Polygons, flow, width_factor, spiralize, speed_factor);
            }
            else
            {
                // no point in the last wall was found close enough to the current wall point so don't interpolate
                addExtrusionMove(p, config, SpaceFillType::Polygons, flow, width_factor, spiralize, speed_factor);
            }
        }
        else
        {
            // no smoothing, use point verbatim
            addExtrusionMove(p, config, SpaceFillType::Polygons, flow, width_factor, spiralize, speed_factor);
        }
    }

    if (is_top_layer)
    {
        // add the tapering spiral
        const double min_spiral_coast_dist = 10; // mm
        double distance_coasted = 0;
        wall_length = 0;
        for (int wall_point_idx = 1; wall_point_idx <= n_points && distance_coasted < min_spiral_coast_dist; wall_point_idx++)
        {
            const Point& p = wall[(seam_vertex_idx + wall_point_idx) % n_points];
            const double seg_length = vSizeMM(p - p0);
            wall_length += seg_length;
            p0 = p;
            // flow is reduced in step with the distance travelled so the wall width should remain roughly constant
            double flow = 1 - (wall_length / total_length);
            if (flow < min_top_layer_flow)
            {
                flow = 0;
                distance_coasted += seg_length;
            }
            // reduce number of paths created when polygon has many points by limiting precision of flow
            constexpr bool no_spiralize = false;
            addExtrusionMove(p, config, SpaceFillType::Polygons, ((int)(flow * 20)) / 20.0, width_factor, no_spiralize, speed_factor);
        }
    }
}

bool ExtruderPlan::forceMinimalLayerTime(double minTime, double minimalSpeed, double travelTime, double extrudeTime,Point starting_position)
{
    double totalTime = travelTime + extrudeTime;
    if (totalTime < minTime && extrudeTime > 0.0)
    {
        double minExtrudeTime = minTime - travelTime;
        if (minExtrudeTime < 1)
            minExtrudeTime = 1;
        double factor = extrudeTime / minExtrudeTime;

        if (slowdown_level > 0)
        {
            bool add_level = false;
            for (GCodePath& path : paths)
            {
                if (slowdown_level > 1 ? path.needSlowdown(slowdown_level) && !path.needSlowdown(slowdown_level - 1) : path.needSlowdown(slowdown_level))
                {
                    double speed = path.config->getSpeed() * path.speed_factor * path.speed_back_pressure_factor * factor;
                    if (speed < minimalSpeed)
                    {
                        path.speed_factor = minimalSpeed / path.config->getSpeed() / path.speed_back_pressure_factor;
                        add_level = true;
                    }
                }
            }
            if (add_level && slowdown_level < 2)
            {
                slowdown_level++;
                return false;
            }             
        }

        for (GCodePath& path : paths)
        {
            if (!path.needSlowdown(slowdown_level))
                continue;
            double speed = path.config->getSpeed() * path.speed_factor * path.speed_back_pressure_factor;
            if (speed < minimalSpeed)
                path.speed_factor = speed / path.config->getSpeed() / path.speed_back_pressure_factor / factor;
            else if (speed * factor < minimalSpeed)
                path.speed_factor = minimalSpeed / path.config->getSpeed() / path.speed_back_pressure_factor / factor;
        }

        // Only slow down for the minimal time if that will be slower.
        assert(getExtrudeSpeedFactor() == 1.0); // The extrude speed factor is assumed not to be changed yet
        if (factor < 1.0)
        {
            setExtrudeSpeedFactor(factor);
        }
        else
        {
            factor = 1.0;
        }

        double inv_factor = 1.0 / factor; // cause multiplication is faster than division
        computeNaiveTimeEstimates(starting_position, 0);
        // Adjust stored naive time estimates
        estimates.extrude_time *= inv_factor;
        for (GCodePath& path : paths)
        {
            path.estimates.extrude_time *= inv_factor;
        }

        if (minTime - estimates.getExtrudeTime() - estimates.getTravelTime() > 0.1)
        {
            extraTime = minTime - estimates.getExtrudeTime() - estimates.getTravelTime();
        }
        totalPrintTime = estimates.getExtrudeTime() + estimates.getTravelTime();
    }

    return true;
}

TimeMaterialEstimates ExtruderPlan::computeNaiveTimeEstimates(Point starting_position, int level)
{
    estimates.reset();
    TimeMaterialEstimates ret;
    TimeEstimateCalculator travel_time_calculator;
    travel_time_calculator.currentPosition = TimeEstimateCalculator::Position(INT2MM(starting_position.X), INT2MM(starting_position.Y), INT2MM(layer_nr * layer_thickness), 0);

    Point p0 = starting_position;

    bool was_retracted = false; // wrong assumption; won't matter that much. (TODO)
    double fitting = 0.024517 * pow(min_layertime, 4) - 1.0104 * pow(min_layertime, 3) + 15.453 * pow(min_layertime, 2) - 105.71 * min_layertime + 323.81;
    int count = 0;
    for (GCodePath& path : paths)
    {
        travel_time_calculator.max_xy_jerk = path.config->getJerk();
        travel_time_calculator.acceleration = path.config->getAcceleration();
        path.estimates.reset();
        bool is_extrusion_path = false;
        double* path_time_estimate;
        double& material_estimate = path.estimates.material;

        if (!path.isTravelPath())
        {
            is_extrusion_path = true;
        }
        if (level > 1 ? path.needSlowdown(level) && !path.needSlowdown(level - 1) : path.needSlowdown(level))
        {
            path_time_estimate = &path.estimates.extrude_time;
        }
        else
        {
            if (path.retract)
            {
                path_time_estimate = &path.estimates.retracted_travel_time;
            }
            else
            {
                path_time_estimate = &path.estimates.unretracted_travel_time;
            }
            if (path.retract != was_retracted)
            { // handle retraction times
                double retract_unretract_time;
                if (path.retract)
                {
                    retract_unretract_time = retraction_config.distance / retraction_config.speed;
                }
                else
                {
                    retract_unretract_time = retraction_config.distance / retraction_config.primeSpeed;
                }
                path.estimates.retracted_travel_time += 0.5 * retract_unretract_time;
                path.estimates.unretracted_travel_time += 0.5 * retract_unretract_time;
            }
        }
        for (Point& p1 : path.points)
        {
            count+=1;
            travel_time_calculator.plan(TimeEstimateCalculator::Position(INT2MM(p1.X), INT2MM(p1.Y), INT2MM(layer_nr * layer_thickness), 0), (path.config->getSpeed() * path.speed_factor), path.config->type);

            p0 = p1;
        }

    }
    if (level == 0 && count < 200)
        estimates.extrude_time -= count/fitting;
    std::vector<Duration> estimates_aaa = travel_time_calculator.calculate();
    for (int i = 0; i < estimates_aaa.size(); i++)
    {
        estimates.extrude_time += estimates_aaa[i];
    }

    return estimates;
}

void ExtruderPlan::processFanSpeedAndMinimalLayerTime(bool force_minimal_layer_time, Point starting_position, double max_cds_fan_speed)
{
    Duration cool_min_layer_time = fan_speed_layer_time_settings.cool_min_layer_time;
    if (cool_min_layer_time_correct != 0.)
    {
        cool_min_layer_time = cool_min_layer_time_correct;
    }
    TimeMaterialEstimates estimates = computeNaiveTimeEstimates(starting_position, slowdown_level);
    if (estimates.getExtrudeTime() == 0 && slowdown_level)
    {
        slowdown_level = 0;
        estimates = computeNaiveTimeEstimates(starting_position, slowdown_level);
    }
    totalPrintTime = estimates.getTotalTime();
    if (force_minimal_layer_time)
    {
        bool done = forceMinimalLayerTime(cool_min_layer_time, fan_speed_layer_time_settings.cool_min_speed, estimates.getTravelTime(), estimates.getExtrudeTime(), starting_position);
        while (!done)
        {
            estimates = computeNaiveTimeEstimates(starting_position, slowdown_level);
            totalPrintTime = estimates.getTotalTime();
            done = forceMinimalLayerTime(cool_min_layer_time, fan_speed_layer_time_settings.cool_min_speed, estimates.getTravelTime(), estimates.getExtrudeTime(), starting_position);
        }
    }

    /*
                   min layer time
                   :
                   :  min layer time fan speed min
                |  :  :
      ^    max..|__:  :
                |  \  :
     fan        |   \ :
    speed  min..|... \:___________
                |________________
                  layer time >


    */
    // interpolate fan speed (for cool_fan_full_layer and for cool_min_layer_time_fan_speed_max)
    fan_speed = fan_speed_layer_time_settings.cool_fan_speed_min;
    cds_fan_speed = fan_speed_layer_time_settings.cds_fan_speed;
    double totalLayerTime = estimates.unretracted_travel_time + estimates.extrude_time;
    if (force_minimal_layer_time && totalLayerTime < fan_speed_layer_time_settings.cool_min_layer_time)
    {
        fan_speed = fan_speed_layer_time_settings.cool_fan_speed_max;
    }
    else if (fan_speed_layer_time_settings.cool_min_layer_time >= fan_speed_layer_time_settings.cool_min_layer_time_fan_speed_max)
    {
        fan_speed = fan_speed_layer_time_settings.cool_fan_speed_min;
    }
    else if (force_minimal_layer_time && totalLayerTime < fan_speed_layer_time_settings.cool_min_layer_time_fan_speed_max)
    {
        // when forceMinimalLayerTime didn't change the extrusionSpeedFactor, we adjust the fan speed
        double fan_speed_diff = fan_speed_layer_time_settings.cool_fan_speed_max - fan_speed_layer_time_settings.cool_fan_speed_min;
        double layer_time_diff = fan_speed_layer_time_settings.cool_min_layer_time_fan_speed_max - fan_speed_layer_time_settings.cool_min_layer_time;
        double fraction_of_slope = (totalLayerTime - fan_speed_layer_time_settings.cool_min_layer_time) / layer_time_diff;
        fan_speed = fan_speed_layer_time_settings.cool_fan_speed_max - fan_speed_diff * fraction_of_slope;
    }
    /*
    Supposing no influence of minimal layer time;
    i.e. layer time > min layer time fan speed min:

              max..   fan 'full' on layer
                   |  :
                   |  :
      ^       min..|..:________________
     fan           |  /
    speed          | /
          speed_0..|/
                   |
                   |__________________
                     layer nr >

    */
    if (layer_nr < fan_speed_layer_time_settings.cool_fan_full_layer && fan_speed_layer_time_settings.cool_fan_full_layer > 0 // don't apply initial layer fan speed speedup if disabled.
        && ! is_raft_layer // don't apply initial layer fan speed speedup to raft, but to model layers
    )
    {
        // Slow down the fan on the layers below the [cool_fan_full_layer], where layer 0 is speed 0.
        fan_speed = fan_speed_layer_time_settings.cool_fan_speed_0 + (fan_speed - fan_speed_layer_time_settings.cool_fan_speed_0) * std::max(LayerIndex(0), layer_nr) / fan_speed_layer_time_settings.cool_fan_full_layer;
    }

    bool full_fan_layer = (layer_nr >= fan_speed_layer_time_settings.cool_fan_full_layer) && getExtrudeSpeedFactor() < 1.0;
    double low_speed_print_time = 0;
    for (GCodePath& path : paths)
    {
        if (full_fan_layer && !path.isTravelPath())
            path.fan_speed = 100.0;
        if (cds_fan_speed > 0 && (path.speed_factor < 1.0 || path.config->isBridgePath()))
        {
            low_speed_print_time += path.estimates.getTotalTime();
        }
    }

    if (cds_fan_speed > 0 && (totalLayerTime < 6 || low_speed_print_time > 1))
    {
        cds_fan_speed = max_cds_fan_speed;
    }
}

void LayerPlan::processFanSpeedAndMinimalLayerTime(Point starting_position)
{
    for (unsigned int extr_plan_idx = 0; extr_plan_idx < extruder_plans.size(); extr_plan_idx++)
    {
        ExtruderPlan& extruder_plan = extruder_plans[extr_plan_idx];
        const Settings& extruder_settings = application->scene->extruders[extruder_plan.extruder_nr].settings;
        double max_cds_fan_speed = extruder_settings.get<double>("cool_special_cds_fan_speed");
        bool force_minimal_layer_time = extr_plan_idx == extruder_plans.size() - 1;
        extruder_plan.processFanSpeedAndMinimalLayerTime(force_minimal_layer_time, starting_position, max_cds_fan_speed);
        if (! extruder_plan.paths.empty() && ! extruder_plan.paths.back().points.empty())
        {
            starting_position = extruder_plan.paths.back().points.back();
        }
    }
}


void LayerPlan::writeGCode(GCodeExport& gcode)
{
    bool infill_slow_OK = false;
    bool wall_slow_OK = false;
    gcode.setLayerNr(layer_nr);

    gcode.writeLayerComment(layer_nr);

    if (layerTemp >0)
    {
        gcode.writeTemperatureCommand(gcode.getExtruderNr(), layerTemp);
    }
	else if (pressureValue >= 0)
	{
		gcode.writePressureComment(pressureValue);
	}
	else if (maxvolumetricspeed > 0)
	{
		gcode.setMaxVolumetricSpeed(maxvolumetricspeed);
	}
   
    // flow-rate compensation
    size_t extruder_nr = gcode.getExtruderNr();
    const Settings& mesh_group_settings = application->scene->current_mesh_group->settings;
    const Settings& extruder_settings = application->scene->extruders[extruder_nr].settings;
    gcode.setFlowRateExtrusionSettings(mesh_group_settings.get<double>("flow_rate_max_extrusion_offset"), mesh_group_settings.get<Ratio>("flow_rate_extrusion_offset_factor")); // Offset is in mm.


	double material_diameter = mesh_group_settings.get<double>("material_diameter");   
	double material_density = mesh_group_settings.get<double>("material_density");
	gcode.setDensity(material_density);
	gcode.setDiameter(material_diameter);
	//std::string machine_name = mesh_group_settings.get<std::string>("machine_name");

    coord_t cds_fan_start_layer = extruder_settings.get<coord_t>("cool_cds_fan_start_at_height") / layer_thickness;

    static LayerIndex layer_1{ 1 - static_cast<LayerIndex>(Raft::getTotalExtraLayers(gcode.application)) };
    if (layer_nr == layer_1 && mesh_group_settings.get<bool>("machine_heated_bed"))
    {
		Temperature max_bed_temperature;
		Temperature max_bed_temperature_layer_0;
		for (ExtruderTrain& aTrain : application->scene->extruders)
		{
			if (max_bed_temperature < aTrain.settings.get<Temperature>("material_bed_temperature"))
			{
				max_bed_temperature = aTrain.settings.get<Temperature>("material_bed_temperature");
			}
			if (max_bed_temperature_layer_0 < aTrain.settings.get<Temperature>("material_bed_temperature_layer_0"))
			{
				max_bed_temperature_layer_0 = aTrain.settings.get<Temperature>("material_bed_temperature_layer_0");
			}
		}
		if (max_bed_temperature != max_bed_temperature_layer_0)
		{
			constexpr bool wait = false;
			gcode.writeBedTemperatureCommand(max_bed_temperature, wait);
		}
    }

    gcode.setZ(z);

    bool speed_limit_to_height_enable = mesh_group_settings.get<bool>("speed_limit_to_height_enable");
    FlowTempGraph speed_limit_to_height = mesh_group_settings.get<FlowTempGraph>("speed_limit_to_height");
    if (speed_limit_to_height_enable)
    {
        gcode.calculatMaxSpeedLimitToHeight(speed_limit_to_height);
    }

    const GCodePathConfig* last_extrusion_config = nullptr; // used to check whether we need to insert a TYPE comment in the gcode.
    const bool acceleration_enabled = mesh_group_settings.get<bool>("acceleration_enabled");
    const bool acceleration_travel_enabled = mesh_group_settings.get<bool>("acceleration_travel_enabled");
    const bool acceleration_breaking_enabled = mesh_group_settings.get<bool>("acceleration_breaking_enable");
    const float acceleration_breaking = mesh_group_settings.get<double>("acceleration_breaking");
    const bool jerk_enabled = mesh_group_settings.get<bool>("jerk_enabled");
    const bool jerk_travel_enabled = mesh_group_settings.get<bool>("jerk_travel_enabled");
    std::string current_mesh = "";

    for (size_t extruder_plan_idx = 0; extruder_plan_idx < extruder_plans.size(); extruder_plan_idx++)
    {
        ExtruderPlan& extruder_plan = extruder_plans[extruder_plan_idx];
        const RetractionConfig& retraction_config = storage.retraction_config_per_extruder[extruder_plan.extruder_nr];
        coord_t z_hop_height = retraction_config.zHop;

        double cds_fan_speed = extruder_plan.cds_fan_speed;
        if (cds_fan_speed > 0 && layer_nr < cds_fan_start_layer)
            cds_fan_speed = -1;

        if (extruder_nr != extruder_plan.extruder_nr)
        {
            int prev_extruder = extruder_nr;
            extruder_nr = extruder_plan.extruder_nr;

            gcode.ResetLastEValueAfterWipe(prev_extruder);

            const ExtruderTrain& prev_extruder_train = application->scene->extruders[prev_extruder];
            if (prev_extruder_train.settings.get<bool>("retraction_hop_after_extruder_switch"))
            {
                z_hop_height = storage.extruder_switch_retraction_config_per_extruder[prev_extruder].zHop;
                gcode.switchExtruder(extruder_nr, storage.extruder_switch_retraction_config_per_extruder[prev_extruder], z_hop_height);
            }
            else
            {
                gcode.switchExtruder(extruder_nr, storage.extruder_switch_retraction_config_per_extruder[prev_extruder]);
            }

            const ExtruderTrain& extruder = application->scene->extruders[extruder_nr];

            if (extruder_plan.prev_extruder_standby_temp)
            { // turn off previous extruder
                constexpr bool wait = false;
                Temperature prev_extruder_temp = *extruder_plan.prev_extruder_standby_temp;
                const LayerIndex prev_layer_nr = (extruder_plan_idx == 0) ? layer_nr - 1 : layer_nr;
                if (prev_layer_nr == storage.max_print_height_per_extruder[prev_extruder])
                {
                    prev_extruder_temp = 0; // TODO ? should there be a setting for extruder_off_temperature ?
                }
                gcode.writeTemperatureCommand(prev_extruder, prev_extruder_temp, wait);
            }

            { // require printing temperature to be met
                constexpr bool wait = true;
                gcode.writeTemperatureCommand(extruder_nr, extruder_plan.required_start_temperature, wait);
            }

            const double extra_prime_amount = extruder.settings.get<bool>("retraction_enable") ? extruder.settings.get<double>("switch_extruder_extra_prime_amount") : 0;
            gcode.addExtraPrimeAmount(extra_prime_amount);
        }
        else if (extruder_plan_idx == 0)
        {
            const WipeScriptConfig& wipe_config = storage.wipe_config_per_extruder[extruder_plan.extruder_nr];
            if (wipe_config.clean_between_layers && gcode.getExtrudedVolumeAfterLastWipe(extruder_nr) > wipe_config.max_extrusion_mm3)
            {
                gcode.insertWipeScript(wipe_config);
                gcode.ResetLastEValueAfterWipe(extruder_nr);
            }
            else if (layer_nr != 0 && application->scene->extruders[extruder_nr].settings.get<bool>("retract_at_layer_change"))
            {
                // only do the retract if the paths are not spiralized
                if (! mesh_group_settings.get<bool>("magic_spiralize"))
                {
                    gcode.writeRetraction(retraction_config);
                }
            }
        }

        double first_fan_speed = extruder_plan.getFirstPrintPathFanSpeed();
        gcode.writeFanCommand(first_fan_speed == GCodePathConfig::FAN_SPEED_DEFAULT ? extruder_plan.getFanSpeed() : first_fan_speed, cds_fan_speed);
        if (extruder_settings.get<bool>("cool_chamber_fan_enable"))
        {
            gcode.writeChamberFanCommand(100.);
        }
        else
        {
            gcode.writeChamberFanCommand(0);
        }

        std::vector<GCodePath>& paths = extruder_plan.paths;

        extruder_plan.inserts.sort([](const NozzleTempInsert& a, const NozzleTempInsert& b) -> bool { return a.path_idx < b.path_idx; });

        const ExtruderTrain& extruder = application->scene->extruders[extruder_nr];

        bool update_extrusion_offset = true;

        bool isAvoidPoint = false;//统计被过滤的点 用于G2G3的判断
        bool  entireLayerSlowdown = false;
        /*float*/Acceleration speed_slowtofast_slowdown_revise_acceleration = application->scene->settings.get<Acceleration>("speed_slowtofast_slowdown_revise_acceleration");
        if (application->scene->settings.get<bool>("speed_slowtofast_slowdown"))
        {//speedSlowDownPath 重新新建的一个变量，标识此段path是否有速度突变
            for (unsigned int path_idx = 0; path_idx < paths.size(); path_idx++)
            {
                double speed = paths[path_idx].config->getSpeed();
                if (paths[path_idx].needSlowdown(extruder_plan.slowdown_level))
                {
                    speed *= extruder_plan.getExtrudeSpeedFactor();
                }
                speed *= paths[path_idx].speed_factor;

                if (!paths[path_idx].config->isTravelPath())
                {
                    paths[path_idx].speedSlowDownPath = speed;
                }
                else
                {
                    paths[path_idx].speedSlowDownPath = -111.0;
                }
            }
            for (unsigned int path_idx = 0; path_idx < paths.size(); path_idx++)
            {
                for (int i = 1; i <= path_idx; i++)
                {
                    if (path_idx - i >= 0 && 
                        (paths[path_idx].config->type == PrintFeatureType::Infill || paths[path_idx].config->type == PrintFeatureType::InnerWall ||
                            paths[path_idx].config->type == PrintFeatureType::OuterWall)
                        && (paths[path_idx - i].config->type == PrintFeatureType::Infill ||
                            paths[path_idx - i].config->type == PrintFeatureType::InnerWall || paths[path_idx - i].config->type == PrintFeatureType::OuterWall)
                        &&(paths[path_idx].speedSlowDownPath / paths[path_idx - i].speedSlowDownPath) > 1.1f
                        && paths[path_idx - i].speedSlowDownPath < 50.0 && this->layer_nr > 0)
                    {
                        entireLayerSlowdown = true;
                        break;
                        break;
                    }
                }
            }

        }

		float realLength = 0.0;
		int iflag = 0;
        for (unsigned int path_idx = 0; path_idx < paths.size(); path_idx++)
        {
            extruder_plan.handleInserts(path_idx, gcode);

            GCodePath& path = paths[path_idx];

            if (path.perform_prime)
            {
                gcode.writePrimeTrain(extruder.settings.get<Velocity>("speed_travel"));
                gcode.writeRetraction(retraction_config);
            }

            if (! path.retract && path.config->isTravelPath() && path.points.size() == 1 && path.points[0] == gcode.getPositionXY() && z == gcode.getPositionZ())
            {
                // ignore travel moves to the current location to avoid needless change of acceleration/jerk
                isAvoidPoint = false;//统计被过滤的点 用于G2G3的判断
                continue;
            }
            else
            {
                isAvoidPoint = true;//统计被过滤的点 用于G2G3的判断
            }

            // In some cases we want to find the next non-travel move.
            size_t next_extrusion_idx = path_idx + 1;
            if ((acceleration_enabled && ! acceleration_travel_enabled) || (jerk_enabled && ! jerk_travel_enabled))
            {
                while (next_extrusion_idx < paths.size() && paths[next_extrusion_idx].config->isTravelPath())
                {
                    ++next_extrusion_idx;
                }
            }

            if (acceleration_enabled)
            {
                if (path.config->isTravelPath())
                {
                    if (acceleration_travel_enabled)
                    {
                        if (entireLayerSlowdown)
                            gcode.writeTravelAcceleration(speed_slowtofast_slowdown_revise_acceleration, acceleration_breaking_enabled, acceleration_breaking);
                        else
                            gcode.writeTravelAcceleration(path.config->getAcceleration(), acceleration_breaking_enabled, acceleration_breaking);
                    }
                    else
                    {
                        // Use the acceleration of the first non-travel move *after* the travel.
                        if (next_extrusion_idx >= paths.size()) // Only travel moves for the remainder of the layer.
                        {
                            if (static_cast<bool>(next_layer_acc_jerk))
                            {
                                if (entireLayerSlowdown)
                                    gcode.writeTravelAcceleration(speed_slowtofast_slowdown_revise_acceleration, acceleration_breaking_enabled, acceleration_breaking);
                                else
                                    gcode.writeTravelAcceleration(next_layer_acc_jerk->first, acceleration_breaking_enabled, acceleration_breaking);
                            } // If the next layer has no extruded move, just keep the old acceleration. Should be very rare to have an empty layer.
                        }
                        else
                        {
                            if (entireLayerSlowdown)
                                gcode.writeTravelAcceleration(speed_slowtofast_slowdown_revise_acceleration, acceleration_breaking_enabled, acceleration_breaking);
                            else
                                gcode.writeTravelAcceleration(paths[next_extrusion_idx].config->getAcceleration(), acceleration_breaking_enabled, acceleration_breaking);
                        }
                    }
                }
                else
                {
                    if (entireLayerSlowdown)
                        gcode.writePrintAcceleration(speed_slowtofast_slowdown_revise_acceleration, acceleration_breaking_enabled, acceleration_breaking);
                    else
                        gcode.writePrintAcceleration(path.config->getAcceleration() * extruder_plan.getExtrudeSpeedFactor(), acceleration_breaking_enabled, acceleration_breaking);
                }
            }
            if (jerk_enabled)
            {
                if (jerk_travel_enabled)
                {
                    gcode.writeJerk(path.config->getJerk());
                }
                else
                {
                    // Use the jerk of the first non-travel move *after* the travel.
                    if (next_extrusion_idx >= paths.size()) // Only travel moves for the remainder of the layer.
                    {
                        if (static_cast<bool>(next_layer_acc_jerk))
                        {
                            gcode.writeJerk(next_layer_acc_jerk->second);
                        } // If the next layer has no extruded move, just keep the old jerk. Should be very rare to have an empty layer.
                    }
                    else
                    {
                        gcode.writeJerk(paths[next_extrusion_idx].config->getJerk());
                    }
                }
            }
            bool newway_zhop = false;
            auto getLastExtrusionPath = [paths](std::vector<Point>& ExtrusionPath, int curIdx)
            {
                ExtrusionPath.clear();
                Point start_point;
                if (curIdx < 1)
                    return true;
                bool start = false;
                for (int i = curIdx - 1; i >= 0; i--)
                {
                    GCodePath path = paths[i];
                    if (!path.isTravelPath())
                    {
                        ClipperLib::ReversePath(path.points);
                        ExtrusionPath.insert(ExtrusionPath.end(), path.points.begin(), path.points.end());
                        start = true;
                    }
                    else if (start)
                    {
                        start_point = path.points.back();
                        break;
                    }
                }
                if (!ExtrusionPath.empty() && start_point == ExtrusionPath[0])//closePolygon
                {
                    ExtrusionPath.pop_back();
                    ClipperLib::ReversePath(ExtrusionPath);
                    return false;
                }

                return true;
            };
            if (path.retract && !newway_zhop)
            {
                gcode.writeLine(";;retract;;");
                //if(extruder.settings.get<RetractionType>("retraction_type") == RetractionType::BAMBOO)
                if (extruder.settings.get<bool>("retraction_wipe"))
                {
                    RetractionConfig retraction_config_1 = retraction_config;
                    retraction_config_1.distance = extruder.settings.get<Ratio>("before_wipe_retraction_amount_percent") * retraction_config_1.distance;
                    gcode.writeRetraction(retraction_config_1);
                    std::vector<Point> last_path;
                    std::vector<std::pair<Point, double>> wipe_path_record;
                    bool openPolygen = getLastExtrusionPath(last_path, path_idx);
                    double wipe_length = MM2INT(extruder.settings.get<double>("wipe_length"));
                    double cur_dis_path = 0;
                    double dis_Extru = (1 - extruder.settings.get<Ratio>("before_wipe_retraction_amount_percent") - 0.015) * retraction_config.distance;
                    double speed = path.config->getSpeed() * 0.5; //wipe speed
                    for (unsigned int point_idx = 0; point_idx < last_path.size(); point_idx++)
                    {
                        Point src_pos = gcode.getPositionXY();
                        Point cur_pos = last_path[point_idx];
                        double len = vSize(cur_pos - src_pos);
                        if (len == 0.) continue;
                        double theta = 1.0f;
                        if (len < wipe_length - cur_dis_path)
                        {
                            theta = len / wipe_length;
                        }
                        else
                        {
                            theta = (wipe_length - cur_dis_path) / wipe_length;
                            cur_pos = src_pos + ((wipe_length - cur_dis_path) / len) * (cur_pos - src_pos);
                        }
                        wipe_path_record.push_back(std::make_pair(src_pos, theta));
                        gcode.writeExtrusionG1(speed, cur_pos, openPolygen ? 0 : -dis_Extru * theta, path.config->type);
                        cur_dis_path += len;
                        if (cur_dis_path > wipe_length) break;
                    }
                    if (openPolygen && !wipe_path_record.empty())
                    {
                        std::reverse(wipe_path_record.begin(), wipe_path_record.end());
                        for (const std::pair<Point, double>& wipe_step : wipe_path_record)
                        {
                            gcode.writeExtrusionG1(speed, wipe_step.first, -dis_Extru * wipe_step.second, path.config->type);
                        }
                    }
                    //gcode.writeRetraction(retraction_config);
                    //if (path.perform_z_hop)
                    //{
                    //    gcode.writeCircle(speed, path.points[0], z_hop_height);
                    //    z_hop_height = retraction_config.zHop; // back to normal z hop
                    //}
                    //else
                    //{
                    //    gcode.writeZhopEnd();
                    //}
                }
                //else//default
                {
                    gcode.writeRetraction(retraction_config);
                    if (path.perform_z_hop)
                    {
                        RetractionHopType hop_type = extruder.settings.get<RetractionHopType>("retraction_hop_type");
                        switch (hop_type)
                        {
                        case cura52::RetractionHopType::DIRECTLIFT:
                        {
                            gcode.writeZhopStart(z_hop_height);
                            z_hop_height = retraction_config.zHop; // back to normal z hop
                            break;
                        }
                        case cura52::RetractionHopType::SPIRALLIFT:
                        {
                            double speed = path.config->getSpeed() * path.speed_factor;
                            gcode.writeCircle(speed, path.points[0], z_hop_height);
                            z_hop_height = retraction_config.zHop; // back to normal z hop
                            break;
                        }
                        case cura52::RetractionHopType::TRAPEZOIDALLEFT:
                        {
                            double speed = path.config->getSpeed() * path.speed_factor;
                            gcode.writeTrapezoidalLeft(speed, path.points[0], z_hop_height);
                            z_hop_height = retraction_config.zHop;
                            break;
                        }
                        default:
                            break;
                        }
                    }
                    else
                    {
                        gcode.writeZhopEnd();
                    }
                }
            }
            else if (newway_zhop)
            {
                gcode.writeLine(";;retract;;");
                //if(extruder.settings.get<RetractionType>("retraction_type") == RetractionType::BAMBOO)
                if (extruder.settings.get<bool>("retraction_wipe"))
                {
                    RetractionConfig retraction_config_1 = retraction_config;
                    retraction_config_1.distance = extruder.settings.get<Ratio>("before_wipe_retraction_amount_percent") * retraction_config_1.distance;
                    gcode.writeRetraction(retraction_config_1);

                    std::vector<Point> last_path;
                    std::vector<std::pair<Point, double>> wipe_path_record;
                    bool openPolygen = getLastExtrusionPath(last_path, path_idx);
                    double wipe_length = MM2INT(extruder.settings.get<double>("wipe_length"));
                    double cur_dis_path = 0;
                    double dis_Extru = (1 - extruder.settings.get<Ratio>("before_wipe_retraction_amount_percent") - 0.015) * retraction_config.distance;
                    double speed = path.config->getSpeed() * 0.5; //wipe speed
                    for (unsigned int point_idx = 0; point_idx < last_path.size(); point_idx++)
                    {
                        Point src_pos = gcode.getPositionXY();
                        Point cur_pos = last_path[point_idx];
                        double len = vSize(cur_pos - src_pos);
                        if (len == 0.) continue;
                        double theta = 1.0f;
                        if (len < wipe_length - cur_dis_path)
                        {
                            theta = len / wipe_length;
                        }
                        else
                        {
                            theta = (wipe_length - cur_dis_path) / wipe_length;
                            cur_pos = src_pos + ((wipe_length - cur_dis_path) / len) * (cur_pos - src_pos);
                        }
                        wipe_path_record.push_back(std::make_pair(src_pos, theta));
                        gcode.writeExtrusionG1(speed, cur_pos, openPolygen ? 0 : -dis_Extru * theta, path.config->type);
                        cur_dis_path += len;
                        if (cur_dis_path > wipe_length) break;
                    }
                    if (openPolygen && !wipe_path_record.empty())
                    {
                        std::reverse(wipe_path_record.begin(), wipe_path_record.end());
                        for (const std::pair<Point, double>& wipe_step : wipe_path_record)
                        {
                            gcode.writeExtrusionG1(speed, wipe_step.first, -dis_Extru * wipe_step.second, path.config->type);
                        }
                    }
                }
                SliceLayer below_slice_layer;
                if (this->layer_nr >0) below_slice_layer = storage.meshes.at(0).layers[this->layer_nr-1];
                Polygons bp = below_slice_layer.getOutlines();
                {
                    gcode.writeRetraction(retraction_config);
                    RetractionHopType hop_type;
                    if (needs_retraction(path, hop_type,bp))
                    {
                        //z_hop_height = 0.4;
                        switch (hop_type)
                        {
                        case cura52::RetractionHopType::SPIRALLIFT:
                        {
                            double speed = path.config->getSpeed() * path.speed_factor;
                            gcode.writeCircle(speed, path.points[0], z_hop_height);
                            z_hop_height = retraction_config.zHop; // back to normal z hop
                            break;
                        }
                        case cura52::RetractionHopType::TRAPEZOIDALLEFT:
                        {
                            double speed = path.config->getSpeed() * path.speed_factor;
                            gcode.writeTrapezoidalLeft(speed, path.points[0], z_hop_height);
                            z_hop_height = retraction_config.zHop;
                            break;
                        }
                        default:
                            break;
                        }
                    }
                    else
                    {
                        gcode.writeZhopEnd();
                    }
                }
            }
            if (! path.config->isTravelPath() && last_extrusion_config != path.config)
            {
                gcode.writeTypeComment(path.config->type);
                if (path.config->isBridgePath())
                {
                    gcode.writeComment("BRIDGE");
                }
                last_extrusion_config = path.config;
                update_extrusion_offset = true;
            }
            else
            {
                update_extrusion_offset = false;
            }

            double speed = path.config->getSpeed();

            // for some movements such as prime tower purge, the speed may get changed by this factor
            speed *= path.speed_factor;

            // Apply the extrusion speed factor if it's an extrusion move.
            if (path.needSlowdown(extruder_plan.slowdown_level))
            {
                speed *= extruder_plan.getExtrudeSpeedFactor();
            }

            // This seems to be the best location to place this, but still not ideal.
            if (path.mesh_id != current_mesh &&  (application->scene->settings.get<bool>("special_object_cancel")))
            {
                current_mesh = path.mesh_id;
                std::stringstream ss;
                if (current_mesh == "")
                    current_mesh = "NONMESH";
                std::stringstream ss_exclude_end;
                std::stringstream ss_exclude_start;
                //change layer cura  only start 
                if (tmp_is_change_layer == 0 && !first_mesh_cancel && path.config->type != PrintFeatureType::SkirtBrim)
                {
                    ss << "MESH:" << current_mesh;
                    gcode.writeComment(ss.str());
                    ss_exclude_start << "EXCLUDE_OBJECT_START NAME=" << path.mesh_id;
                    gcode.writeComment2(ss_exclude_start.str());
                    first_mesh_cancel = true;
                }
                else if (tmp_is_change_layer != this->layer_nr)
                {
                    ss << "MESH:" << current_mesh;
                    gcode.writeComment(ss.str());
                    ss_exclude_start << "EXCLUDE_OBJECT_START NAME=" << path.mesh_id;
                    gcode.writeComment2(ss_exclude_start.str());
                }
                else 
                {
                    ss << "MESH:" << current_mesh;
                    gcode.writeComment(ss.str());
                    ss_exclude_end << "EXCLUDE_OBJECT_END NAME=" << tmp_mesh_name;
                    gcode.writeComment2(ss_exclude_end.str());
                    if (current_mesh != "NONMESH")
                    {
                        ss_exclude_start << "EXCLUDE_OBJECT_START NAME=" << path.mesh_id;
                        gcode.writeComment2(ss_exclude_start.str());
                    }

                }
                tmp_mesh_name = path.mesh_id;
                tmp_is_change_layer = this->layer_nr;
            }
            if (path.mesh_id != current_mesh)
            {
                current_mesh = path.mesh_id;
                if (current_mesh == "") current_mesh = "NONMESH";
                std::stringstream ss;
                ss << "MESH:" << current_mesh;
                gcode.writeComment(ss.str());
            }
            if (path.config->isTravelPath())
            { // early comp for travel paths, which are handled more simply
                if (! path.perform_z_hop && final_travel_z != z && extruder_plan_idx == (extruder_plans.size() - 1) && path_idx == (paths.size() - 1))
                {
                    // Before the final travel, move up to the next layer height, on the current spot, with a sensible speed.
                    Point3 current_position = gcode.getPosition();
                    current_position.z = final_travel_z;
                    gcode.writeTravel(current_position, extruder.settings.get<Velocity>("speed_z_hop"));

                    // Prevent the final travel(s) from resetting to the 'previous' layer height.
                    gcode.setZ(final_travel_z);
                }
                for (unsigned int point_idx = 0; point_idx < path.points.size() - 1; point_idx++)
                {
                    gcode.writeTravel(path.points[point_idx], speed);
                }
                if (path.unretract_before_last_travel_move)
                {
                    // We need to unretract before the last travel move of the path if the next path is an outer wall.
                    gcode.writeUnretractionAndPrime();
                }
                gcode.writeTravel(path.points.back(), speed);
                continue;
            }

            bool judgeVelocityDip = false;
            float speed_quarter;
            if (application->scene->settings.get<bool>("speed_slowtofast_slowdown"))
            {
                for (int i = 1; i <= path_idx; i++)
                {   
                    float v_previous = paths[path_idx - i].speedSlowDownPath;
                    if (path_idx - i >= 0 && (v_previous != -111.0f) &&
                        ((speed / paths[path_idx - i].speedSlowDownPath) > 1.1f ))     //from   large  to  small,  Reversely  is path is Travel Path
                    {
                        if ((path.config->type == PrintFeatureType::Infill || path.config->type == PrintFeatureType::InnerWall ||
                            path.config->type == PrintFeatureType::OuterWall) &&( paths[path_idx - i].config->type == PrintFeatureType::Infill ||
                            paths[path_idx - i].config->type == PrintFeatureType::InnerWall || paths[path_idx - i].config->type == PrintFeatureType::OuterWall)
                            && v_previous < 50.0 && this->layer_nr > 0)
                        {   //outwall   force  to  slowdown else  front line all no slow  ,   small  circle   also  faster
                            //if (path.config->type == PrintFeatureType::OuterWall && application->scene->settings.get<bool>("optimize_wall_printing_order"))
                            //    break;
                            //if (path.config->type == PrintFeatureType::InnerWall && !application->scene->settings.get<bool>("optimize_wall_printing_order"))
                            //    break;

                            float speed_slowdown_factor = application->scene->settings.get<Ratio>("speed_slowtofast_slowdown_revise_speed_factor");
                            float speed_span_quarter = (abs(speed  - v_previous )) * speed_slowdown_factor;

                            speed_quarter = v_previous + speed_span_quarter;
                            judgeVelocityDip = true;
                            break;
                            //if (this->layer_nr > 14 && path.config->type == PrintFeatureType::OuterWall)
                            //{
                            //    std::cout << "  this->layer_nr =  " << this->layer_nr;
                            //    std::cout << "  path_idx =  " << path_idx;
                            //    std::cout << "  i =  " << i;
                            //    std::cout << "  speed =  " << speed;
                            //    std::cout << "  path.config->type =  " << speed;
                            //    std::cout << "  v_previous =  " << v_previous;
                            //    std::cout << "   vv * paths[path_idx - i].speed_back_pressure_factor) =  " << v_previous;
                            //    std::cout << "   speed_span_quarter =  " << speed_span_quarter << std::endl;
                            //    std::cout << "   speed_quarter =  " << speed_quarter << std::endl;
                            //}
                              
                        }
                    }
                }
            }
           
           

            bool spiralize = path.spiralize;
            if (! spiralize) // normal (extrusion) move (with coasting)
            {
                // if path provides a valid (in range 0-100) fan speed, use it
                const double path_fan_speed = path.getFanSpeed();
                gcode.writeFanCommand(path_fan_speed != GCodePathConfig::FAN_SPEED_DEFAULT ? path_fan_speed : extruder_plan.getFanSpeed(), cds_fan_speed);

                bool coasting = extruder.settings.get<bool>("coasting_enable");
                if (coasting)
                {
                    coasting = writePathWithCoasting(gcode, extruder_plan_idx, path_idx, layer_thickness);
                }
                if (! coasting) // not same as 'else', cause we might have changed [coasting] in the line above...
                { // normal path to gcode algorithm
                    bool arc_configure_enable = application->scene->settings.get<bool>("arc_configure_enable");
                    bool slope_slice_enable = application->scene->settings.get<coord_t>("special_slope_slice_angle") != 0;
                    if (arc_configure_enable && !slope_slice_enable)
                    {
                        if (1)
                        {
                            switch (path.config->type)
                            {
                            case PrintFeatureType::OuterWall:
                            case PrintFeatureType::InnerWall:
                            case PrintFeatureType::Infill:
                            {
                                {
                                    //std::stringstream ss;
                                    //ss << "arc_fitting start layer_nr=" << layer_nr;
                                    //gcode.writeComment(ss.str());
                                }
                                double tolerance = application->scene->settings.get<double>("arc_tolerance");

                                //填充模式下加大偏差值
                                if (PrintFeatureType::Infill == path.config->type)
                                    tolerance *= 2.0;

                                //double tolerance = 100;// 200;
                                Slic3r::Points points;
                                std::vector<Slic3r::PathFittingData> fitting_result;

                                if(isAvoidPoint && !path.points.empty())//统计被过滤的点 用于G2G3的判断
                                {
                                    if (path.points[0] != gcode.getPositionXY())
                                    {
                                        points.emplace_back(Slic3r::Point((int64_t)gcode.getPositionXY().X, (int64_t)gcode.getPositionXY().Y));
                                    }
                                }
                                for (unsigned int point_idx = 0; point_idx < path.points.size(); point_idx++)
                                {
                                    points.emplace_back(Slic3r::Point((int64_t)path.points[point_idx].X, (int64_t)path.points[point_idx].Y));
                                    //points.emplace_back(Slic3r::Point(path.points[point_idx].X, path.points[point_idx].Y));
                                }

                                //Slic3r::ArcFitter::do_arc_fitting_and_simplify(points, fitting_result, tolerance);
                                /*bool arcFittingValiable = */Slic3r::ArcFitter::do_arc_fitting(points, fitting_result, tolerance);
                                bool Large_arc_exist = false;
                                for (size_t fitting_index = 0; fitting_index < fitting_result.size(); fitting_index++)
                                {
                                    if (fitting_result[fitting_index].path_type == Slic3r::EMovePathType::Arc_move_cw
                                        || fitting_result[fitting_index].path_type == Slic3r::EMovePathType::Arc_move_ccw)
                                    {
                                        const double arc_length = fitting_result[fitting_index].arc_data.length;
                                        if (arc_length > 5000)
                                        {
                                            Large_arc_exist = true;
                                            break;
                                        }
                                    }
                                }
                                if (Large_arc_exist)
                                {//大圆弧拟合需缩小公差
                                    fitting_result.clear();
                                    tolerance = 1;
                                    Slic3r::ArcFitter::do_arc_fitting(points, fitting_result, tolerance);
                                }

                                float dis = 0;
                                float dis_threshold = application->scene->settings.get<coord_t>("speed_slowtofast_slowdown_revise_distance"); //如果大于这一段距离，则是原来的速度，否则速度50
                                bool slow2fastSlowdown = application->scene->settings.get<bool>("speed_slowtofast_slowdown");
                                // BBS: start to generate gcode from arc fitting data which includes line and arc
                                for (size_t fitting_index = 0; fitting_index < fitting_result.size(); fitting_index++)
                                {

                                    switch (fitting_result[fitting_index].path_type)
                                    {
                                    case Slic3r::EMovePathType::Linear_move:
                                    {
                                        size_t start_index = fitting_result[fitting_index].start_point_index;
                                        size_t end_index = fitting_result[fitting_index].end_point_index;
                                        double extrude_speed;// = speed * path.speed_back_pressure_factor;                          
                                        for (size_t point_index = start_index; point_index < end_index + 1; point_index++)
                                        {
                                            Point gcodePt(points[point_index].x(), points[point_index].y());
                                            if (slow2fastSlowdown && !wall_slow_OK  && judgeVelocityDip && (path.config->type == PrintFeatureType::OuterWall || path.config->type == PrintFeatureType::InnerWall))
                                            {//经过测试大部分的圆弧都是经常这个case，Arc_move_ccw和Linear_move共同组成了慢速到快速的距离计算，如果大于这一段距离，则是原来的速度，否则速度50
                                                extrude_speed = speed_quarter;
                                                if (dis > dis_threshold)  wall_slow_OK = true;
                                            }
                                            else if (slow2fastSlowdown && !infill_slow_OK  && judgeVelocityDip && path.config->type == PrintFeatureType::Infill)
                                            {
                                                extrude_speed = speed_quarter;
                                                if (dis > dis_threshold)  infill_slow_OK = true;
                                            }
                                            else
                                                extrude_speed = speed * path.speed_back_pressure_factor;
                                            //if (this->layer_nr == 50 && path.config->type == PrintFeatureType::InnerWall)
                                            //    std::cout << "  Linear_move  extrude_speed =  " << extrude_speed  << std ::endl; 
                                            gcode.writeExtrusion(gcodePt, extrude_speed, path.getExtrusionMM3perMM(), path.config->type, update_extrusion_offset);
                                            if (point_index > 0)
                                            {
                                                dis += vSize(Point(points[point_index].x(), points[point_index].y()) - Point(points[point_index - 1].x(), points[point_index - 1].y()));
                                            }
                                        }
                                    }
                                    break;
                                    case  Slic3r::EMovePathType::Arc_move_cw:
                                    case  Slic3r::EMovePathType::Arc_move_ccw:
                                    {
                                        const  Slic3r::ArcSegment& arc = fitting_result[fitting_index].arc_data;
                                        const double arc_length = arc.length;
                                        Point start_point(arc.start_point.x(), arc.start_point.y());
                                        Point end_point(arc.end_point.x(), arc.end_point.y());
                                        Point center(arc.center.x(), arc.center.y());
                                        Point center_offset = gcode.getGcodePos(center.X, center.Y, gcode.getExtruderNr()) - gcode.getGcodePos(start_point.X, start_point.Y, gcode.getExtruderNr());
                                        double extrude_speed;// = speed * path.speed_back_pressure_factor;
                                        {
                                            //屏蔽说明信息
                                            //std::stringstream ss;
                                            //ss << "do_arc_fitting start pos=" << INT2MM(start_point.X) << " " << INT2MM(start_point.Y) << " " << INT2MM(arc_length);
                                            //gcode.writeComment(ss.str());
                                            //确保每次都在圆弧拟合的起点
                                            //gcode.writeArcSatrt(start_point);
                                        }
                                        extrude_speed = speed * path.speed_back_pressure_factor;
                                        //if (this->layer_nr == 50 && path.config->type == PrintFeatureType::InnerWall)
                                        //    std::cout << " Arc_move_ccw extrude_speed =  " << extrude_speed;
                                        dis += vSize(Point(arc.start_point.x(), arc.start_point.y()) - Point(arc.end_point.x(), arc.end_point.y()));

                                        if ( dis < dis_threshold && judgeVelocityDip)
                                        {
                                            extrude_speed = speed_quarter;
                                        }
                                        gcode.writeExtrusionG2G3(end_point, center_offset, arc_length, extrude_speed, path.getExtrusionMM3perMM(), path.config->type, update_extrusion_offset, arc.direction == Slic3r::ArcDirection::Arc_Dir_CCW);
                                    }
                                    break;
                                    default:
                                        //BBS: should never happen that a empty path_type has been stored
                                        assert(0);
                                        break;
                                    }
                                }
                                {
                                    //std::stringstream ss;
                                    //ss << "arc_fitting end layer_nr=" << layer_nr;
                                    //gcode.writeComment(ss.str());
                                }

                            }
                            break;
                            default:
                                for (unsigned int point_idx = 0; point_idx < path.points.size(); point_idx++)
                                {
                                    const double extrude_speed = speed * path.speed_back_pressure_factor;
                                    gcode.writeExtrusion(path.points[point_idx], extrude_speed, path.getExtrusionMM3perMM(), path.config->type, update_extrusion_offset);
                                }
                                break;
                            }

                        }
                        else
                        {
                            for (unsigned int point_idx = 0; point_idx < path.points.size(); point_idx++)
                            {
                                const double extrude_speed = speed * path.speed_back_pressure_factor;
                                gcode.writeExtrusion(path.points[point_idx], extrude_speed, path.getExtrusionMM3perMM(), path.config->type, update_extrusion_offset);
                            }

                        }
                    }
                    else
                    {
                        for(unsigned int point_idx = 0; point_idx < path.points.size(); point_idx++)
                        {
							auto start_speed_down = [&](const double length,const Velocity deceleration_speed)
							{
								const Point diff = path.points[point_idx] - gcode.getPositionXY();
								if (cura52::shorterThan(diff, 100))
								{
									return;
								}
								double diff_length = (sqrt(diff.X * diff.X + diff.Y * diff.Y));
								realLength += diff_length;
								if (realLength > length * 0.8 && realLength < length * 1.2)
								{
									gcode.writeExtrusion(path.points[point_idx], deceleration_speed, path.getExtrusionMM3perMM(), path.config->type, update_extrusion_offset);
									//is_deceleration_speed = false;
									realLength = 0.0;
									iflag += 1;
								}
								else if (realLength < length)
								{
									gcode.writeExtrusion(path.points[point_idx], deceleration_speed, path.getExtrusionMM3perMM(), path.config->type, update_extrusion_offset);
								}
								else
								{
									float needLength = diff_length - realLength + length;
									Point newPoint;
									newPoint.X = (needLength * diff.X) / diff_length + gcode.getPositionXY().X;
									newPoint.Y = (needLength * diff.Y) / diff_length + gcode.getPositionXY().Y;
									gcode.writeExtrusion(newPoint, deceleration_speed, path.getExtrusionMM3perMM(), path.config->type, update_extrusion_offset);
									//is_deceleration_speed = false;
									realLength = 0.0;
									iflag += 1;

									if (3 == iflag)
									{
										const double extrude_speed = speed * path.speed_back_pressure_factor;
										gcode.writeExtrusion(path.points[point_idx], extrude_speed, path.getExtrusionMM3perMM(), path.config->type, update_extrusion_offset);
									}
								}
							};


							if (is_deceleration_speed)//颗粒机 每层起始段减速
							{
								float length = application->scene->settings.get<coord_t>("layer_deceleration_length");
								float deceleration_speed = application->scene->settings.get<Velocity>("layer_deceleration_speed");
								if (iflag == 0)
								{
									start_speed_down(length * 0.6, deceleration_speed);
								}
								if (1 == iflag)
								{
									Velocity currentSpeed = deceleration_speed + (speed * path.speed_back_pressure_factor - deceleration_speed) * 0.25;
									start_speed_down(length * 0.2, currentSpeed);
								}
								if (2 == iflag)
								{
									Velocity currentSpeed = deceleration_speed + (speed * path.speed_back_pressure_factor - deceleration_speed) * 0.6;
									start_speed_down(length * 0.2, currentSpeed);
								}
								if (3 == iflag)
								{ 
									is_deceleration_speed = false; 
								}
							} 
							else
							{
								const double extrude_speed = speed * path.speed_back_pressure_factor;
								gcode.writeExtrusion(path.points[point_idx], extrude_speed, path.getExtrusionMM3perMM(), path.config->type, update_extrusion_offset);
							}
                        }
                    }
                }
            }
            else
            { // SPIRALIZE
                // If we need to spiralize then raise the head slowly by 1 layer as this path progresses.
                float totalLength = 0.0;
                Point p0 = gcode.getPositionXY();
                for (unsigned int _path_idx = path_idx; _path_idx < paths.size() && ! paths[_path_idx].isTravelPath(); _path_idx++)
                {
                    GCodePath& _path = paths[_path_idx];
                    for (unsigned int point_idx = 0; point_idx < _path.points.size(); point_idx++)
                    {
                        Point p1 = _path.points[point_idx];
                        totalLength += vSizeMM(p0 - p1);
                        p0 = p1;
                    }
                }

                float length = 0.0;
                p0 = gcode.getPositionXY();
                for (; path_idx < paths.size() && paths[path_idx].spiralize; path_idx++)
                { // handle all consecutive spiralized paths > CHANGES path_idx!
                    GCodePath& path = paths[path_idx];

                    for (unsigned int point_idx = 0; point_idx < path.points.size(); point_idx++)
                    {
                        const Point p1 = path.points[point_idx];
                        length += vSizeMM(p0 - p1);
                        p0 = p1;
                        if (application->scene->settings.get<coord_t>("special_slope_slice_angle") == 0)
                        {
                            gcode.setZ(std::round(z + layer_thickness * length / totalLength));
                        }
                        else
                        {
                            gcode.setZ(z);
                            gcode.setZOffset(std::round(layer_thickness * length / totalLength));
                        }

                        const double extrude_speed = speed * path.speed_back_pressure_factor;
                        gcode.writeExtrusion(path.points[point_idx], extrude_speed, path.getExtrusionMM3perMM(), path.config->type, update_extrusion_offset);
                    }
                    // for layer display only - the loop finished at the seam vertex but as we started from
                    // the location of the previous layer's seam vertex the loop may have a gap if this layer's
                    // seam vertex is "behind" the previous layer's seam vertex. So output another line segment
                    // that joins this layer's seam vertex to the following vertex. If the layers have been blended
                    // then this can cause a visible ridge (on the screen, not on the print) because the first vertex
                    // would have been shifted in x/y to make it nearer to the previous layer outline but the seam
                    // vertex would not be shifted (as it's the last vertex in the sequence). The smoother the model,
                    // the less the vertices are shifted and the less obvious is the ridge. If the layer display
                    // really displayed a spiral rather than slices of a spiral, this would not be required.
                }
                path_idx--; // the last path_idx didnt spiralize, so it's not part of the current spiralize path
                if(paths[path_idx].spiralize)
                   extruder_plan.handleInserts(path_idx, gcode);
            }
            infill_slow_OK = false;
            wall_slow_OK = false;
        } // paths for this extruder /\  .

        if (extruder.settings.get<bool>("cool_lift_head") && extruder_plan.extraTime > 0.0)
        {
            gcode.writeComment("Small layer, adding delay");
            const RetractionConfig& retraction_config = storage.retraction_config_per_extruder[gcode.getExtruderNr()];
            gcode.writeRetraction(retraction_config);
            if (extruder_plan_idx == extruder_plans.size() - 1 || ! extruder.settings.get<bool>("machine_extruder_end_pos_abs"))
            { // only move the head if it's the last extruder plan; otherwise it's already at the switching bay area
                // or do it anyway when we switch extruder in-place
                gcode.setZ(gcode.getPositionZ() + MM2INT(3.0));
                gcode.writeTravel(gcode.getPositionXY(), configs_storage.travel_config_per_extruder[extruder_nr].getSpeed());

                const Point current_pos = gcode.getPositionXY();
                const Point machine_middle = storage.machine_size.flatten().getMiddle();
                const Point toward_middle_of_bed = current_pos - normal(current_pos - machine_middle, MM2INT(20.0));
                gcode.writeTravel(toward_middle_of_bed, configs_storage.travel_config_per_extruder[extruder_nr].getSpeed());
            }
            gcode.writeDelay(extruder_plan.extraTime);
        }

        extruder_plan.handleAllRemainingInserts(gcode);
    } // extruder plans /\  .

    gcode.updateTotalPrintTime();
}

void LayerPlan::overrideFanSpeeds(double speed)
{
    for (ExtruderPlan& extruder_plan : extruder_plans)
    {
        extruder_plan.setFanSpeed(speed);
    }
}


bool LayerPlan::makeRetractSwitchRetract(unsigned int extruder_plan_idx, unsigned int path_idx)
{
    std::vector<GCodePath>& paths = extruder_plans[extruder_plan_idx].paths;
    for (unsigned int path_idx2 = path_idx + 1; path_idx2 < paths.size(); path_idx2++)
    {
        if (paths[path_idx2].getExtrusionMM3perMM() > 0)
        {
            return false;
        }
    }

    if (extruder_plans.size() <= extruder_plan_idx + 1)
    {
        return false; // TODO: check first extruder of the next layer! (generally only on the last layer of the second extruder)
    }

    if (extruder_plans[extruder_plan_idx + 1].extruder_nr != extruder_plans[extruder_plan_idx].extruder_nr)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool LayerPlan::writePathWithCoasting(GCodeExport& gcode, const size_t extruder_plan_idx, const size_t path_idx, const coord_t layer_thickness)
{
    ExtruderPlan& extruder_plan = extruder_plans[extruder_plan_idx];
    const ExtruderTrain& extruder = application->scene->extruders[extruder_plan.extruder_nr];
    const double coasting_volume = extruder.settings.get<double>("coasting_volume");
    if (coasting_volume <= 0)
    {
        return false;
    }
    const std::vector<GCodePath>& paths = extruder_plan.paths;
    const GCodePath& path = paths[path_idx];
    if (path_idx + 1 >= paths.size() || (path.isTravelPath() || ! paths[path_idx + 1].config->isTravelPath()) || path.points.size() < 2)
    {
        return false;
    }

    coord_t coasting_min_dist_considered = MM2INT(0.1); // hardcoded setting for when to not perform coasting

    double extrude_speed = path.config->getSpeed() * path.speed_factor * path.speed_back_pressure_factor;
    if (path.needSlowdown(extruder_plan.slowdown_level))
    {
        extrude_speed *= extruder_plan.getExtrudeSpeedFactor();
    }

    const coord_t coasting_dist = MM2INT(MM2_2INT(coasting_volume) / layer_thickness) / path.config->getLineWidth(); // closing brackets of MM2INT at weird places for precision issues
    const double coasting_min_volume = extruder.settings.get<double>("coasting_min_volume");
    const coord_t coasting_min_dist = MM2INT(MM2_2INT(coasting_min_volume + coasting_volume) / layer_thickness) / path.config->getLineWidth(); // closing brackets of MM2INT at weird places for precision issues
    //           /\ the minimal distance when coasting will coast the full coasting volume instead of linearly less with linearly smaller paths

    std::vector<coord_t> accumulated_dist_per_point; // the first accumulated dist is that of the last point! (that of the last point is always zero...)
    accumulated_dist_per_point.push_back(0);

    coord_t accumulated_dist = 0;

    bool length_is_less_than_min_dist = true;

    unsigned int acc_dist_idx_gt_coast_dist = NO_INDEX; // the index of the first point with accumulated_dist more than coasting_dist (= index into accumulated_dist_per_point)
                                                        // == the point printed BEFORE the start point for coasting

    const Point* last = &path.points[path.points.size() - 1];
    for (unsigned int backward_point_idx = 1; backward_point_idx < path.points.size(); backward_point_idx++)
    {
        const Point& point = path.points[path.points.size() - 1 - backward_point_idx];
        const coord_t distance = vSize(point - *last);
        accumulated_dist += distance;
        accumulated_dist_per_point.push_back(accumulated_dist);

        if (acc_dist_idx_gt_coast_dist == NO_INDEX && accumulated_dist >= coasting_dist)
        {
            acc_dist_idx_gt_coast_dist = backward_point_idx; // the newly added point
        }

        if (accumulated_dist >= coasting_min_dist)
        {
            length_is_less_than_min_dist = false;
            break;
        }

        last = &point;
    }

    if (accumulated_dist < coasting_min_dist_considered)
    {
        return false;
    }
    coord_t actual_coasting_dist = coasting_dist;
    if (length_is_less_than_min_dist)
    {
        // in this case accumulated_dist is the length of the whole path
        actual_coasting_dist = accumulated_dist * coasting_dist / coasting_min_dist;
        if (actual_coasting_dist == 0) // Downscaling due to Minimum Coasting Distance reduces coasting to less than 1 micron.
        {
            return false; // Skip coasting at all then.
        }
        for (acc_dist_idx_gt_coast_dist = 1; acc_dist_idx_gt_coast_dist < accumulated_dist_per_point.size(); acc_dist_idx_gt_coast_dist++)
        { // search for the correct coast_dist_idx
            if (accumulated_dist_per_point[acc_dist_idx_gt_coast_dist] >= actual_coasting_dist)
            {
                break;
            }
        }
    }

    assert(acc_dist_idx_gt_coast_dist < accumulated_dist_per_point.size()); // something has gone wrong; coasting_min_dist < coasting_dist ?

    const size_t point_idx_before_start = path.points.size() - 1 - acc_dist_idx_gt_coast_dist;

    Point start;
    { // computation of begin point of coasting
        const coord_t residual_dist = actual_coasting_dist - accumulated_dist_per_point[acc_dist_idx_gt_coast_dist - 1];
        const Point& a = path.points[point_idx_before_start];
        const Point& b = path.points[point_idx_before_start + 1];
        start = b + normal(a - b, residual_dist);
    }

    { // write normal extrude path:
        for (size_t point_idx = 0; point_idx <= point_idx_before_start; point_idx++)
        {
            gcode.writeExtrusion(path.points[point_idx], extrude_speed, path.getExtrusionMM3perMM(), path.config->type);
        }
        gcode.writeExtrusion(start, extrude_speed, path.getExtrusionMM3perMM(), path.config->type);
    }

    // write coasting path
    for (size_t point_idx = point_idx_before_start + 1; point_idx < path.points.size(); point_idx++)
    {
        const Ratio coasting_speed_modifier = extruder.settings.get<Ratio>("coasting_speed");
        const Velocity speed = Velocity(coasting_speed_modifier * path.config->getSpeed()* path.speed_factor * extruder_plan.getExtrudeSpeedFactor());
        gcode.writeTravel(path.points[point_idx], speed);
    }
    return true;
}

void LayerPlan::applyBackPressureCompensation()
{
    for (auto& extruder_plan : extruder_plans)
    {
        const Settings& extruder_settings = application->scene->extruders[extruder_plan.extruder_nr].settings;
        const Ratio back_pressure_compensation = extruder_settings.get<Ratio>("speed_equalize_flow_width_factor");
        const Velocity machine_max_feedrate = std::min(extruder_settings.get<Velocity>("machine_max_feedrate_x"), extruder_settings.get<Velocity>("machine_max_feedrate_y"));
        if (back_pressure_compensation != 0.0)
        {
            extruder_plan.applyBackPressureCompensation(back_pressure_compensation, machine_max_feedrate);
        }
    }
}

int LayerPlan::getLayerNr() const
{
    return layer_nr;
}

Point LayerPlan::getLastPlannedPositionOrStartingPosition() const
{
    return last_planned_position.value_or(layer_start_pos_per_extruder[getExtruder()]);
}

bool LayerPlan::getIsInsideMesh() const
{
    return was_inside;
}

bool LayerPlan::getSkirtBrimIsPlanned(unsigned int extruder_nr) const
{
    return skirt_brim_is_processed[extruder_nr];
}

void LayerPlan::setSkirtBrimIsPlanned(unsigned int extruder_nr)
{
    skirt_brim_is_processed[extruder_nr] = true;
}

size_t LayerPlan::getExtruder() const
{
    return extruder_plans.back().extruder_nr;
}

void LayerPlan::setBridgeWallMask(const Polygons& polys)
{
    bridge_wall_mask = polys;
}

void LayerPlan::setOverhangMask(const Polygons& polys, int idx)
{
    if (overhang_mask.empty())
        overhang_mask.resize(4);
    if (idx > 3 || idx < 0) return;
        overhang_mask[idx] = overhang_mask[idx].unionPolygons(polys);
}

void LayerPlan::setOverhangSpeedSections(const std::vector<std::pair<float, float>>& SpeedSections)
{
    overhang_speed_sections = SpeedSections;
}

std::vector<std::pair<float, float>> LayerPlan::getOverhangSpeedSections()
{
    return overhang_speed_sections;
}

void LayerPlan::setFillLineWidthDiff(coord_t diff)
{
    fill_lineWidth_diff = diff;
}

void LayerPlan::calculateSpeedSections(const float original_speed, const float line_width, const std::vector<Ratio>& speeds, const std::vector<double>& overlaps, std::vector<std::pair<float, float>>& speed_sections)
{
    //const std::vector<double> overlaps({ 75, 50, 25, 5 });
    //const std::vector<float> speeds({ 100,50,30,10 });

    size_t  speed_sections_count = std::min(overlaps.size(), speeds.size());

    for (size_t i = 0; i < speed_sections_count; i++) {
        float distance = line_width * (overlaps.at(i) / 100.0);
        float speed = speeds.at(i) >= 1 ? (float)(original_speed * speeds.at(i) / 100.0) : (float)speeds.at(i);
        speed_sections.push_back({ distance, speed });
    }
    std::sort(speed_sections.begin(), speed_sections.end(),
        [](const std::pair<float, float>& a, const std::pair<float, float>& b) {
            if (a.first == b.first) {
                return a.second > b.second;
            }
            return a.first < b.first; });

    std::pair<float, float> last_section{ INFINITY, 0 };
    for (auto& section : speed_sections) {
        if (section.first == last_section.first) {
            section.second = last_section.second;
        }
        else {
            last_section = section;
        }
    }
}
bool LayerPlan::needs_retraction(const GCodePath& travel, RetractionHopType& HopType, Polygons& bp)
{
    //LiftType& lift_type = auto;
    //if (travel.length() < scale_(EXTRUDER_CONFIG(retraction_minimum_travel))) {
    //    // skip retraction if the move is shorter than the configured threshold
    //    return false;
    //}

    //BBS: input travel polyline must be in current plate coordinate system
    //
    float max_z_hop = 400.f;
    //for (int i = 0; i < m_config.z_hop.size(); i++)
        //max_z_hop = std::max(max_z_hop, (float)m_config.z_hop.get_at(i));
    
    float travel_len_thresh = max_z_hop / tan(200);

    float accum_len = 0.f;
    


    //BBS: force to retract when leave from external perimeter for a long travel
    //Better way is judging whether the travel move direction is same with last extrusion move.


    if (travel.config->type == PrintFeatureType::InnerWall && travel.config->type != PrintFeatureType::MoveCombing
        && travel.config->type != PrintFeatureType::MoveRetraction) {
        
        //lift_type = is_through_overhang(clipped_travel) ? LiftType::SpiralLift : LiftType::LazyLift;
        Polygons path;
        if (travel.points.size() > 1)
        {
            for (int i = 0; i < travel.points.size()-1; i++)
            {
                path.addLine(travel.points.at(i), travel.points.at(i+1));
            }
        }


        if (!bp.intersection(path).empty())
        {
            HopType = cura52::RetractionHopType::TRAPEZOIDALLEFT;
        }
        else
        {
            HopType = cura52::RetractionHopType::SPIRALLIFT;
        }
        return true;
    }

    //if (travel.config->type == PrintFeatureType::Support) {
    //    const SupportLayer* support_layer = this-> dynamic_cast<const SupportLayer*>(m_layer);
    //    //FIXME support_layer->support_islands.contains should use some search structure!
    //    if (support_layer != NULL)
    //        // skip retraction if this is a travel move inside a support material island
    //        //FIXME not retracting over a long path may cause oozing, which in turn may result in missing material
    //        // at the end of the extrusion path!
    //        for (const ExPolygon& support_island : support_layer->support_islands)
    //            if (support_island.contains(travel))
    //                return false;
    //    //reduce the retractions in lightning infills for tree support
    //    if (support_layer != NULL && support_layer->support_type == stInnerTree)
    //        for (auto& area : support_layer->base_areas)
    //            if (area.contains(travel))
    //                return false;
    //}
    //BBS: need retract when long moving to print perimeter to avoid dropping of material

    //if (!is_perimeter(role) && m_config.reduce_infill_retraction && m_layer != nullptr &&
    //    m_config.sparse_infill_density.value > 0 && m_retract_when_crossing_perimeters.travel_inside_internal_regions(*m_layer, travel))
    //    // Skip retraction if travel is contained in an internal slice *and*
    //    // internal infill is enabled (so that stringing is entirely not visible).
    //    //FIXME any_internal_region_slice_contains() is potentionally very slow, it shall test for the bounding boxes first.
    //    return false;

    // retract if reduce_infill_retraction is disabled or doesn't apply when role is perimeter
    //if (ZHopType(EXTRUDER_CONFIG(z_hop_types)) == ZHopType::zhtAuto) {
    //    lift_type = is_through_overhang(clipped_travel) ? LiftType::SpiralLift : LiftType::LazyLift;
    //}
    //else {
    //    lift_type = to_lift_type(ZHopType(EXTRUDER_CONFIG(z_hop_types)));
    //}
   // return true;
     return false;
}
Ratio LayerPlan::getSpeedFactorP(const Point& p0, const float distance, const float original_speed, const std::vector<std::pair<float, float>>& speed_sections)
{
    float final_speed = original_speed;

    if (distance <= speed_sections.front().first) {
        final_speed = original_speed;
    }
    else if (distance >= speed_sections.back().first) {
        final_speed = speed_sections.back().second;
    }
    else {
        size_t section_idx = 0;
        while (distance > speed_sections[section_idx + 1].first) {
            section_idx++;
        }
        float t = (distance - speed_sections[section_idx].first) /
            (speed_sections[section_idx + 1].first - speed_sections[section_idx].first);
        t = std::clamp(t, 0.0f, 1.0f);
        final_speed = (1.0f - t) * speed_sections[section_idx].second + t * speed_sections[section_idx + 1].second;
    }
    return Ratio((final_speed * 1.0f) / original_speed);
}

} // namespace cura52
