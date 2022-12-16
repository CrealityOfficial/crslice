#include "cxutil/input/param.h"
#include "cxutil/input/sceneinput.h"

namespace cxutil
{
	SceneParam::SceneParam()
	{

	}

	SceneParam::~SceneParam()
	{

	}

	void SceneParam::initialize(SceneInput* scene)
	{
		Settings* settings = scene->settings();
//zeng
		machine_gcode_flavor = settings->get<EGCodeFlavor>("machine_gcode_flavor");
//yi

//liu

//yao
	}

	MeshGroupParam::MeshGroupParam()
		:wireframe_enabled(false)
		, carve_multiple_volumes(false)
		, layer_height(1)
		, layer_height_0(0)
		, adaptive_layer_height_enabled(false)
		, adaptive_layer_height_variation(1)
		, adaptive_layer_height_variation_step(1)
		, adaptive_layer_height_threshold(1)
		, magic_spiralize(false)
		, ooze_shield_enabled(false)
		, ooze_shield_angle(60.0)
		, draft_shield_enabled(false)
		, draft_shield_height_limitation(DraftShieldHeightLimitation::FULL)
		, draft_shield_height(10)
		, draft_shield_dist(10)
		, support_enable(false)
		, support_tree_branch_diameter(2)
		, support_tree_branch_diameter_angle(5)
		, support_tree_collision_resolution(0.85)
		, support_wall_count(1)
		, support_bottom_distance(0)
	{

	}

	MeshGroupParam::~MeshGroupParam()
	{

	}

	void MeshGroupParam::initialize(GroupInput* meshGroup)
	{
		Settings* settings = meshGroup->settings();
		layer_height_0 = settings->get<coord_t>("layer_height_0");
		layer_height = settings->get<coord_t>("layer_height");
		adaptive_layer_height_enabled = settings->get<bool>("adaptive_layer_height_enabled");
		if (adaptive_layer_height_enabled)
		{
			adaptive_layer_height_variation = settings->get<coord_t>("adaptive_layer_height_variation");
			adaptive_layer_height_variation_step = settings->get<coord_t>("adaptive_layer_height_variation_step");
			adaptive_layer_height_threshold = settings->get<coord_t>("adaptive_layer_height_threshold");
		}

		carve_multiple_volumes = settings->get<bool>("carve_multiple_volumes");
		alternate_carve_order = settings->get<bool>("alternate_carve_order");
		support_brim_enable = settings->get<bool>("support_brim_enable");
		//zeng
		remove_empty_first_layers = settings->get<bool>("remove_empty_first_layers");
		support_type = settings->get<ESupportType>("support_type");
		support_tree_enable = settings->get<bool>("support_tree_enable");
		smooth_spiralized_contours = settings->get<bool>("smooth_spiralized_contours");
		support_xy_distance = settings->get<coord_t>("support_xy_distance");
		support_tree_collision_resolution = settings->get<coord_t>("support_tree_collision_resolution");
		support_tree_angle = settings->get<AngleRadians>("support_tree_angle");
		support_line_width = settings->get<coord_t>("support_line_width");
		layer_0_z_overlap = settings->get<coord_t>("layer_0_z_overlap");
		//yi
		machine_width = settings->get<coord_t>("machine_width");
		machine_height = settings->get<coord_t>("machine_height");
		machine_depth = settings->get<coord_t>("machine_depth");

		brim_gap = settings->get<coord_t>("brim_gap");


		support_roof_enable = settings->get<bool>("support_roof_enable");
		support_bottom_enable = settings->get<bool>("support_bottom_enable");
		support_enable = settings->get<bool>("support_enable");

		prime_blob_enable = settings->get<bool>("prime_blob_enable");
		machine_shape = settings->get < BuildPlateShape>("machine_shape") ;
		support_extruder_nr_layer_0 = settings->get<size_t>("support_extruder_nr_layer_0");
		support_infill_extruder_nr = settings->get<size_t>("support_infill_extruder_nr");
		support_roof_extruder_nr = settings->get<size_t>("support_roof_extruder_nr");
		support_bottom_extruder_nr = settings->get<size_t>("support_bottom_extruder_nr");

		
		brim_outside_only = settings->get<bool>("brim_outside_only");
		brim_replaces_support = settings->get<bool>("brim_replaces_support");
		support_brim_line_count = settings->get<coord_t>("support_brim_line_count");
		//liu
		magic_spiralize = settings->get<bool>("magic_spiralize");
		ooze_shield_enabled = settings->get<bool>("ooze_shield_enabled");
		ooze_shield_angle = settings->get<AngleDegrees>("ooze_shield_angle");
		draft_shield_enabled = settings->get<bool>("draft_shield_enabled");
		draft_shield_height_limitation = settings->get<DraftShieldHeightLimitation>("draft_shield_height_limitation");
		draft_shield_height = settings->get< coord_t>("draft_shield_height");
		draft_shield_dist = settings->get<coord_t>("draft_shield_dist");
		machine_gcode_flavor = settings->get<EGCodeFlavor>("machine_gcode_flavor");
		machine_use_extruder_offset_to_offset_coords = settings->get<bool>("machine_use_extruder_offset_to_offset_coords");

		machine_name = settings->get<std::string>("machine_name");
		machine_buildplate_type = settings->get<std::string>("machine_buildplate_type");
		relative_extrusion = settings->get<bool>("relative_extrusion");
		wall_line_width_0 = settings->get<coord_t>("wall_line_width_0");

		wall_line_width_x = settings->get<coord_t>("wall_line_width_x");

		 machine_max_feedrate_x = settings->get<Velocity>("machine_max_feedrate_x");
		 machine_max_feedrate_y = settings->get<Velocity>("machine_max_feedrate_y");
		 machine_max_feedrate_z = settings->get<Velocity>("machine_max_feedrate_z");
		 machine_max_feedrate_e = settings->get<Velocity>("machine_max_feedrate_e");
		 machine_max_acceleration_x = settings->get<Acceleration>("machine_max_acceleration_x");

		 machine_max_acceleration_y = settings->get<Acceleration>("machine_max_acceleration_y");
		 machine_max_acceleration_z = settings->get<Acceleration>("machine_max_acceleration_z");
		 machine_max_acceleration_e = settings->get<Acceleration>("machine_max_acceleration_e");

		 machine_max_jerk_xy = settings->get<Velocity>("machine_max_jerk_xy");

		 machine_max_jerk_z = settings->get<Velocity>("machine_max_jerk_z");
		 machine_max_jerk_e = settings->get<Velocity>("machine_max_jerk_e");
		 machine_minimum_feedrate = settings->get<Velocity>("machine_minimum_feedrate");
		 machine_acceleration = settings->get<Acceleration>("machine_acceleration");
		 support_tree_branch_diameter = settings->get<coord_t>("support_tree_branch_diameter");
		 support_wall_count = settings->get<size_t>("support_wall_count");
		 support_bottom_distance = settings->get<coord_t>("support_bottom_distance");
		 support_tree_branch_diameter_angle = settings->get<double>("support_tree_branch_diameter_angle");
		 support_interface_skip_height = settings->get<coord_t>("support_interface_skip_height");
		 support_bottom_height = settings->get<coord_t>("support_bottom_height");
		 initial_layer_line_width_factor = settings->get<Ratio>("initial_layer_line_width_factor");
		 infill_line_distance = settings->get<coord_t>("infill_line_distance");
		 material_print_temperature = settings->get<Temperature>("material_print_temperature");
		 support_conical_angle = settings->get<AngleRadians>("support_conical_angle");
		 support_pattern = settings->get<EFillMethod>("support_pattern");
		 support_infill_sparse_thickness = settings->get<coord_t>("support_infill_sparse_thickness");
		 minimum_support_area = settings->get<double>("minimum_support_area");
		 support_roof_height = settings->get<coord_t>("support_roof_height");
		//yao

		//wang
		adhesion_type = settings->get<EPlatformAdhesion>("adhesion_type");
		prime_tower_enable = settings->get<bool>("prime_tower_enable");
		prime_tower_min_volume = settings->get<coord_t>("prime_tower_min_volume");
		prime_tower_size = settings->get<coord_t>("prime_tower_size");
		adhesion_extruder_nr = settings->get<size_t>("adhesion_extruder_nr");
		prime_tower_brim_enable = settings->get<bool>("prime_tower_brim_enable");
		prime_tower_position_x = settings->get<coord_t>("prime_tower_position_x");
		prime_tower_position_y = settings->get<coord_t>("prime_tower_position_y");
		material_bed_temp_prepend = settings->get<bool>("material_bed_temp_prepend");
		machine_heated_bed = settings->get<bool>("machine_heated_bed");
		material_bed_temperature_layer_0 = settings->get<Temperature>("material_bed_temperature_layer_0");
		material_bed_temp_wait = settings->get<bool>("material_bed_temp_wait");
		material_print_temp_prepend = settings->get<bool>("material_print_temp_prepend");
		material_print_temp_wait = settings->get<bool>("material_print_temp_wait");
		machine_start_gcode = settings->get<std::string>("machine_start_gcode");
		machine_heated_build_volume = settings->get<bool>("machine_heated_build_volume");
		build_volume_temperature = settings->get<Temperature>("build_volume_temperature");
		retraction_amount = settings->get<coord_t>("retraction_amount");
		relative_extrusion = settings->get<bool>("relative_extrusion");
		retraction_enable = settings->get<bool>("retraction_enable");
		acceleration_enabled = settings->get<bool>("acceleration_enabled");
		machine_acceleration = settings->get<Acceleration>("machine_acceleration");
		jerk_enabled = settings->get<bool>("jerk_enabled");
		machine_max_jerk_xy = settings->get<Velocity>("machine_max_jerk_xy");
		machine_end_gcode = settings->get<std::string>("machine_end_gcode");

		machine_name = settings->get<std::string>("machine_name");
		machine_buildplate_type = settings->get<std::string>("machine_buildplate_type");
		speed_slowdown_layers = settings->get<size_t>("speed_slowdown_layers");

		retraction_combing = settings->get<CombingMode>("retraction_combing");

		wall_line_count = settings->get<size_t>("wall_line_count");
		travel_retract_before_outer_wall = settings->get<bool>("travel_retract_before_outer_wall");
		outer_inset_first = settings->get<bool>("outer_inset_first");

		flow_rate_max_extrusion_offset = settings->get<double>("flow_rate_max_extrusion_offset");
		flow_rate_extrusion_offset_factor = settings->get<Ratio>("flow_rate_extrusion_offset_factor");
		material_bed_temperature = settings->get<Temperature>("material_bed_temperature");
	}

	MeshParam::MeshParam()
		:m_meshType(0)
		, minimum_polygon_circumference(0)
		, surfaceMode(ESurfaceMode::NORMAL)
		, meshfix_extensive_stitching(false)
		, meshfix_keep_open_polygons(false)
		, mold_enabled(false)
		, mold_width(1)
		, wall_line_width_0(1)
		, mold_roof_height(1)
		, conical_overhang_enabled(false)
		, layer_height(1)
		, wall_line_count(1)
		, magic_spiralize(false)
		, alternate_extra_perimeter(false)
		, wall_0_inset(0)
		, wall_line_width_x(1.7)
		, wall_0_extruder_nr(0)
		, wall_x_extruder_nr(1)
		, support_enable(false)
		, fill_outline_gaps(false)
		, infill_line_distance(0)
		, ironing_enabled(false)
		, infill_support_enabled(false)
		, magic_fuzzy_skin_thickness(0.3)
		, adhesion_type(EPlatformAdhesion::SKIRT)
		, magic_fuzzy_skin_outside_only(false)
		, skin_overlap_mm(0.17)
		, support_tree_branch_distance(1)
		, support_interface_skip_height(0.2)
		, support_bottom_height(2.0)
	{

	}

	MeshParam::~MeshParam()
	{

	}

	void MeshParam::initialize(MeshInput* mesh)
	{
		Settings* settings = mesh->settings();

		bool infill = settings->get<bool>("infill_mesh");
		bool overhang = settings->get<bool>("anti_overhang_mesh");
		bool cutting = settings->get<bool>("cutting_mesh");
		bool support = settings->get<bool>("support_mesh");
		
		if (infill) m_meshType |= (int)MeshType::infill_mesh;
		if (overhang) m_meshType |= (int)MeshType::anti_overhang_mesh;
		if (cutting) m_meshType |= (int)MeshType::cutting_mesh;
		if (support) m_meshType |= (int)MeshType::support_mesh;

		minimum_polygon_circumference = settings->get<coord_t>("minimum_polygon_circumference");


		surfaceMode = settings->get<ESurfaceMode>("magic_mesh_surface_mode");
		meshfix_extensive_stitching = settings->get<bool>("meshfix_extensive_stitching");
		meshfix_keep_open_polygons = settings->get<bool>("meshfix_keep_open_polygons");
		filter_out_tiny_gaps = settings->get<bool>("filter_out_tiny_gaps");
		optimize_wall_printing_order = settings->get<bool>("optimize_wall_printing_order");
		top_bottom_pattern_0 = settings->get<EFillMethod>("top_bottom_pattern_0");
		top_bottom_pattern = settings->get<EFillMethod>("top_bottom_pattern");
		ironing_pattern = settings->get<EFillMethod>("ironing_pattern");

		ironing_line_spacing = settings->get<coord_t>("ironing_line_spacing");
		ironing_inset = settings->get<coord_t>("ironing_inset");

		infill_overlap_mm = settings->get<coord_t>("infill_overlap_mm");

		mold_enabled = settings->get<bool>("mold_enabled");
		mold_width = settings->get<coord_t>("mold_width");
		wall_line_width_0 = settings->get<coord_t>("wall_line_width_0");
		//const AngleDegrees angle = mesh.settings->get<AngleDegrees>("mold_angle");
		mold_roof_height = settings->get<coord_t>("mold_roof_height");

		conical_overhang_enabled = settings->get<bool>("conical_overhang_enabled");
		conical_overhang_angle = settings->get<AngleRadians>("conical_overhang_angle");
		multiple_mesh_overlap = settings->get<coord_t>("multiple_mesh_overlap");
		infill_mesh_order = settings->get<int>("infill_mesh_order");

		magic_mesh_surface_mode = settings->get<ESurfaceMode>("magic_mesh_surface_mode");

		layer_height = settings->get<coord_t>("layer_height");

		meshfix_union_all_remove_holes = settings->get<bool>("meshfix_union_all_remove_holes");
		meshfix_union_all = settings->get<bool>("meshfix_union_all");
		hole_xy_offset = settings->get<coord_t>("hole_xy_offset");
		xy_offset = settings->get<coord_t>("xy_offset");


		//zeng
		initial_bottom_layers = settings->get<size_t>("initial_bottom_layers");
		support_mesh_drop_down = settings->get<bool>("support_mesh_drop_down");
		top_skin_preshrink = settings->get<coord_t>("top_skin_preshrink");
		bottom_skin_preshrink = settings->get<coord_t>("bottom_skin_preshrink");
		top_skin_expand_distance = settings->get<coord_t>("top_skin_expand_distance");
		bottom_skin_expand_distance = settings->get<coord_t>("bottom_skin_expand_distance");
		skin_no_small_gaps_heuristic = settings->get<bool>("skin_no_small_gaps_heuristic");
		skin_line_width = settings->get<coord_t>("skin_line_width");
		infill_pattern = settings->get<EFillMethod>("infill_pattern");
		min_infill_area = settings->get<double>("min_infill_area");

		min_skin_width_for_expansion = settings->get<coord_t>("min_skin_width_for_expansion");
		infill_line_width = settings->get<coord_t>("infill_line_width");
		initial_layer_line_width_factor = settings->get<Ratio>("initial_layer_line_width_factor");
		infill_support_angle = settings->get<AngleRadians>("infill_support_angle");
		gradual_infill_steps = settings->get<size_t>("gradual_infill_steps");
		gradual_infill_step_height = settings->get<size_t>("gradual_infill_step_height");
		infill_sparse_thickness = settings->get<coord_t>("infill_sparse_thickness");
		support_top_distance = settings->get<coord_t>("support_top_distance");
		support_tower_maximum_supported_diameter = settings->get<coord_t>("support_tower_maximum_supported_diameter");
		support_use_towers = settings->get<bool>("support_use_towers");
		support_angle = settings->get<AngleRadians>("support_angle");
		support_infill_extruder_nr = extruderIndex("support_infill_extruder_nr", settings);
		support_roof_enable = settings->get<bool>("support_roof_enable");
		support_bottom_enable = settings->get<bool>("support_bottom_enable");
		minimum_support_area = settings->get<double>("minimum_support_area");
		support_roof_height = settings->get<coord_t>("support_roof_height");
		support_interface_skip_height = settings->get<coord_t>("support_interface_skip_height");
		minimum_roof_area = settings->get<double>("minimum_roof_area");
		support_bottom_height = settings->get<coord_t>("support_bottom_height");
		support_bottom_distance = settings->get<coord_t>("support_bottom_distance");
		minimum_bottom_area = settings->get<coord_t>("minimum_bottom_area");
		support_offset = settings->get<coord_t>("support_offset");
		support_xy_distance = settings->get<coord_t>("support_xy_distance");
		support_xy_distance_overhang = settings->get<coord_t>("support_xy_distance_overhang");
		support_xy_overrides_z = settings->get<SupportDistPriority>("support_xy_overrides_z");
		support_bottom_stair_step_width = settings->get<coord_t>("support_bottom_stair_step_width");
		support_bottom_stair_step_height = settings->get<coord_t>("support_bottom_stair_step_height");
		support_conical_angle = settings->get<AngleRadians>("support_conical_angle");
		support_conical_enabled = settings->get<bool>("support_conical_enabled");
		support_tower_diameter = settings->get<coord_t>("support_tower_diameter");
		support_tower_roof_angle = settings->get<AngleRadians>("support_tower_roof_angle");
		support_line_width = settings->get<coord_t>("support_line_width");
		//yi
		wall_line_width_0 = settings->get<coord_t>("wall_line_width_0");

		wall_line_width_x = settings->get<coord_t>("wall_line_width_x");
		magic_spiralize = settings->get<bool>("magic_spiralize");
		wall_line_count = settings->get<size_t>("wall_line_count");
		alternate_extra_perimeter = settings->get<bool>("alternate_extra_perimeter");

		infill_line_distance = settings->get<coord_t>("infill_line_distance");
		top_layers = settings->get<size_t>("top_layers");
		bottom_layers = settings->get<size_t>("bottom_layers");
		roofing_layer_count = settings->get<size_t>("roofing_layer_count");
		skin_outline_count = settings->get<size_t>("skin_outline_count");

		fill_outline_gaps = settings->get<bool>("fill_outline_gaps");

		meshfix_maximum_resolution = settings->get<coord_t>("meshfix_maximum_resolution");
		meshfix_maximum_deviation = settings->get<coord_t>("meshfix_maximum_deviation");

		z_seam_x = settings->get<coord_t>("z_seam_x");
		z_seam_y = settings->get<coord_t>("z_seam_y");
		z_seam_relative = settings->get<bool>("z_seam_relative");

		support_enable = settings->get<bool>("support_enable");
		support_tree_enable = settings->get<bool>("support_tree_enable");

		wall_0_extruder_nr = extruderIndex("wall_0_extruder_nr", settings);
		wall_x_extruder_nr = extruderIndex("wall_x_extruder_nr", settings);
		infill_extruder_nr = extruderIndex("infill_extruder_nr", settings);
		top_bottom_extruder_nr = extruderIndex("top_bottom_extruder_nr", settings);
		roofing_extruder_nr = extruderIndex("roofing_extruder_nr", settings);

		raft_margin = settings->get<coord_t>("raft_margin");
		skirt_gap = settings->get<coord_t> ("skirt_gap");
		skirt_line_count = settings->get<size_t>("skirt_line_count");

		support_brim_enable = settings->get<bool>("support_brim_enable");
		//liu
		magic_spiralize = settings->get<bool>("magic_spiralize");
		alternate_extra_perimeter = settings->get<bool>("alternate_extra_perimeter");
		wall_0_inset = settings->get<coord_t>("wall_0_inset");
		support_enable = settings->get<bool>("support_enable");
		fill_outline_gaps = settings->get<bool>("fill_outline_gaps");
		ironing_enabled = settings->get<bool>("ironing_enabled");
		infill_support_enabled = settings->get<bool>("infill_support_enabled");
		infill_offset_x = settings->get<coord_t>("infill_offset_x");
		infill_offset_y = settings->get<coord_t>("infill_offset_y");
		cross_infill_density_image = settings->get<std::string>("cross_infill_density_image");
		magic_fuzzy_skin_enabled = settings->get<bool>("magic_fuzzy_skin_enabled");
		magic_fuzzy_skin_thickness = settings->get<coord_t>("magic_fuzzy_skin_thickness");
		adhesion_type = settings->get< EPlatformAdhesion>("adhesion_type");
		magic_fuzzy_skin_outside_only = settings->get<bool>("magic_fuzzy_skin_outside_only");
		z_seam_type = settings->get<EZSeamType>("z_seam_type");
		support_tree_branch_distance = settings->get<coord_t>("support_tree_branch_distance");
		support_interface_skip_height = settings->get<coord_t>("support_interface_skip_height");
		support_bottom_height = settings->get<coord_t>("support_bottom_height");
		//yao

		//wang
		//z_seam_type = settings->get<EZSeamType>("z_seam_type");
		infill_angles = settings->get<std::vector<AngleDegrees>>("infill_angles");
		roofing_angles = settings->get<std::vector<AngleDegrees>>("roofing_angles");
		skin_angles = settings->get<std::vector<AngleDegrees>>("skin_angles");
		support_mesh = settings->get<bool>("support_mesh");
		anti_overhang_mesh = settings->get<bool>("anti_overhang_mesh");
		cutting_mesh = settings->get<bool>("cutting_mesh");
		infill_mesh = settings->get<bool>("infill_mesh");
		z_seam_corner = settings->get<EZSeamCornerPrefType>("z_seam_corner");
		wall_0_wipe_dist = settings->get<coord_t>("wall_0_wipe_dist");
		infill_before_walls = settings->get<bool>("infill_before_walls");
		spaghetti_infill_enabled = settings->get<bool>("spaghetti_infill_enabled");
		zig_zaggify_infill = settings->get<bool>("zig_zaggify_infill");
		connect_infill_polygons = settings->get<bool>("connect_infill_polygons");
		infill_multiplier = settings->get<size_t>("infill_multiplier");
		cross_infill_pocket_size = settings->get <coord_t>("cross_infill_pocket_size");
		infill_randomize_start_location = settings->get <bool>("infill_randomize_start_location");
		infill_enable_travel_optimization = settings->get <bool>("infill_enable_travel_optimization");
		infill_wall_line_count = settings->get <size_t>("infill_wall_line_count");
		skin_edge_support_layers = settings->get <size_t>("skin_edge_support_layers");
		infill_wipe_dist = settings->get <coord_t>("infill_wipe_dist");
		travel_compensate_overlapping_walls_0_enabled = settings->get<bool>("travel_compensate_overlapping_walls_0_enabled");
		travel_compensate_overlapping_walls_x_enabled = settings->get<bool>("travel_compensate_overlapping_walls_x_enabled");
		travel_retract_before_outer_wall = settings->get<bool>("travel_retract_before_outer_wall");
		bridge_settings_enabled = settings->get<bool>("bridge_settings_enabled");
		wall_overhang_angle = settings->get<AngleDegrees>("wall_overhang_angle");
		outer_inset_first = settings->get<bool>("outer_inset_first");
		roofing_pattern = settings->get<EFillMethod>("roofing_pattern");
		skin_overlap_mm = settings->get<coord_t>("skin_overlap_mm");
		bridge_enable_more_layers = settings->get<bool>("bridge_enable_more_layers");
		bridge_skin_support_threshold = settings->get<Ratio>("bridge_skin_support_threshold");
		bridge_skin_density = settings->get<Ratio>("bridge_skin_density");
		bridge_skin_density_2 = settings->get<Ratio>("bridge_skin_density_2");
		bridge_skin_density_3 = settings->get<Ratio>("bridge_skin_density_3");
		support_fan_enable = settings->get<bool>("support_fan_enable");
		support_supported_skin_fan_speed = settings->get<Ratio>("support_supported_skin_fan_speed");
		connect_skin_polygons = settings->get<bool>("connect_skin_polygons");

		speed_topbottom = settings->get<Velocity>("speed_topbottom");
		speed_equalize_flow_enabled = settings->get<bool>("speed_equalize_flow_enabled");
		wall_x_material_flow = settings->get<Ratio>("wall_x_material_flow");
		material_flow_layer_0 = settings->get<Ratio>("material_flow_layer_0");
		acceleration_topbottom = settings->get<Acceleration>("acceleration_topbottom");
		jerk_topbottom = settings->get<Velocity>("jerk_topbottom");

		wall_0_material_flow = settings->get<Ratio>("wall_0_material_flow");
		bridge_wall_material_flow = settings->get<Ratio>("bridge_wall_material_flow");
		bridge_fan_speed = settings->get<Ratio>("bridge_fan_speed");
		skin_material_flow = settings->get<Ratio>("skin_material_flow");
		bridge_skin_material_flow = settings->get<Ratio>("bridge_skin_material_flow");
		roofing_material_flow = settings->get<Ratio>("roofing_material_flow");
		bridge_skin_material_flow_2 = settings->get<Ratio>("bridge_skin_material_flow_2");
		bridge_skin_material_flow_3 = settings->get<Ratio>("bridge_skin_material_flow_3");

		acceleration_wall_0 = settings->get<Acceleration>("acceleration_wall_0");
		acceleration_wall_x = settings->get<Acceleration>("acceleration_wall_x");
		acceleration_roofing = settings->get<Acceleration>("acceleration_roofing");

		roofing_line_width = settings->get<coord_t>("roofing_line_width");

		speed_wall_0 = settings->get<Velocity>("speed_wall_0");
		jerk_wall_0 = settings->get<Velocity>("jerk_wall_0");
		speed_wall_x = settings->get<Velocity>("speed_wall_x");
		jerk_wall_x = settings->get<Velocity>("jerk_wall_x");
		bridge_wall_speed = settings->get<Velocity>("bridge_wall_speed");
		bridge_skin_speed = settings->get<Velocity>("bridge_skin_speed");
		speed_roofing = settings->get<Velocity>("speed_roofing");
		jerk_roofing = settings->get<Velocity>("jerk_roofing");
		bridge_skin_speed_2 = settings->get<Velocity>("bridge_skin_speed_2");
		bridge_fan_speed_2 = settings->get<Velocity>("bridge_fan_speed_2");
		bridge_skin_speed_3 = settings->get<Velocity>("bridge_skin_speed_3");
		bridge_fan_speed_3 = settings->get<Velocity>("bridge_fan_speed_3");

		ironing_flow = settings->get<Ratio>("ironing_flow");
		infill_material_flow = settings->get<Ratio>("infill_material_flow");

		speed_ironing = settings->get<Velocity>("speed_ironing");
		jerk_ironing = settings->get<Velocity>("jerk_ironing");
		speed_infill = settings->get<Velocity>("speed_infill");
		jerk_infill = settings->get<Velocity>("jerk_infill");

		acceleration_ironing = settings->get<Acceleration>("acceleration_ironing");
		acceleration_infill = settings->get<Acceleration>("acceleration_infill");

		acceleration_print_layer_0 = settings->get<Acceleration>("acceleration_print_layer_0");
		speed_print_layer_0 = settings->get<Velocity>("speed_print_layer_0");
		jerk_print_layer_0 = settings->get<Velocity>("jerk_print_layer_0");

		retraction_combing = settings->get<CombingMode>("retraction_combing");

		bridge_wall_min_length = settings->get<coord_t>("bridge_wall_min_length");
		small_feature_max_length = settings->get<coord_t>("small_feature_max_length");
		wall_min_flow = settings->get<Ratio>("wall_min_flow");
		small_feature_speed_factor_0 = settings->get<Ratio>("small_feature_speed_factor_0");
		small_feature_speed_factor = settings->get<Ratio>("small_feature_speed_factor");
		small_feature_speed_factor = settings->get<Ratio>("small_feature_speed_factor");
		bridge_wall_coast = settings->get<Ratio>("bridge_wall_coast");
		wall_overhang_speed_factor = settings->get<bool>("wall_overhang_speed_factor");
		bridge_sparse_infill_max_density = settings->get<Ratio>("bridge_sparse_infill_max_density");
		//wang
		extruder_nr = settings->get<size_t>("extruder_nr");
	}

	size_t MeshParam::extruderIndex(const char* name, Settings* settings)
	{
		int nr = settings->get<int>(name);
		if (nr < 0)
		{
			nr = settings->get<size_t>("extruder_nr");
		}
		return nr;
	}

	ExtruderParam::ExtruderParam()
	{

	}

	ExtruderParam::~ExtruderParam()
	{

	}

	void ExtruderParam::initialize(ExtruderInput* extruder)
	{
		Settings* settings = extruder->settings();
		//zeng
		support_line_width = settings->get<coord_t>("support_line_width");
		support_roof_line_width = settings->get<coord_t>("support_roof_line_width");
		support_roof_offset = settings->get<coord_t>("support_roof_offset");
		support_bottom_line_width = settings->get<coord_t>("support_bottom_line_width");
		support_bottom_offset = settings->get<coord_t>("support_bottom_offset");
		support_pattern = settings->get<EFillMethod>("support_pattern");
		cross_support_density_image = settings->get<std::string>("cross_support_density_image");
		support_line_distance = settings->get<coord_t>("support_line_distance");
		support_offset = settings->get<coord_t>("support_offset");
		support_wall_count = settings->get<size_t>("support_wall_count");
		support_top_distance = settings->get<coord_t>("support_top_distance");
		support_xy_distance = settings->get<coord_t>("support_xy_distance");
		support_xy_distance_overhang = settings->get<coord_t>("support_xy_distance_overhang");
		support_xy_overrides_z = settings->get<SupportDistPriority>("support_xy_overrides_z");
		support_angle = settings->get<AngleRadians>("support_angle");
		support_tower_maximum_supported_diameter = settings->get<coord_t>("support_tower_maximum_supported_diameter");
		support_use_towers = settings->get<bool>("support_use_towers");
		support_bottom_distance = settings->get<coord_t>("support_bottom_distance");
		support_conical_angle = settings->get<AngleRadians>("support_conical_angle");
		support_conical_enabled = settings->get<bool>("support_conical_enabled");
		support_tower_diameter = settings->get<coord_t>("support_tower_diameter");
		support_tower_roof_angle = settings->get<AngleRadians>("support_tower_roof_angle");
		raft_margin = settings->get<coord_t>("raft_margin");
		skirt_gap = settings->get<coord_t>("skirt_gap");
		skirt_line_count = settings->get<size_t>("skirt_line_count");
		support_conical_min_width = settings->get<coord_t>("support_conical_min_width");
		support_join_distance = settings->get<coord_t>("support_join_distance");
		support_infill_sparse_thickness = settings->get<coord_t>("support_infill_sparse_thickness");
		gradual_support_infill_step_height = settings->get<coord_t>("gradual_support_infill_step_height");
		gradual_support_infill_steps = settings->get<size_t>("gradual_support_infill_steps");
		adhesion_type = settings->get<EPlatformAdhesion>("adhesion_type");
		raft_surface_layers = settings->get<size_t>("raft_surface_layers");
		raft_airgap = settings->get<coord_t>("raft_airgap");
		machine_min_cool_heat_time_window = settings->get<Duration>("machine_min_cool_heat_time_window");
		material_final_print_temperature = settings->get<Temperature>("material_final_print_temperature");
		machine_extruder_end_code = settings->get<std::string>("machine_extruder_end_code");

		//yi
		skirt_brim_minimal_length = settings->get<coord_t>("skirt_brim_minimal_length");
		meshfix_maximum_resolution = settings->get<coord_t>("meshfix_maximum_resolution");
		meshfix_maximum_deviation = settings->get<coord_t>("meshfix_maximum_deviation");
		brim_outside_only = settings->get<bool>("brim_outside_only");
		prime_tower_brim_enable = settings->get<bool>("prime_tower_brim_enable");
		brim_replaces_support = settings->get<bool>("brim_replaces_support");
		support_brim_enable = settings->get<bool>("support_brim_enable");
		support_brim_line_count = settings->get<size_t>("support_brim_line_count");
		//liu
		raft_base_line_width = settings->get<coord_t>("raft_base_line_width");
		raft_base_fan_speed = settings->get<Ratio>("raft_base_fan_speed");
		raft_base_line_spacing = settings->get<coord_t>("raft_base_line_spacing");
		raft_base_thickness = settings->get<coord_t>("raft_base_thickness");
		travel_avoid_distance	= settings->get<coord_t>("travel_avoid_distance");
		raft_interface_thickness = settings->get<coord_t>("raft_interface_thickness");
		raft_interface_line_spacing = settings->get<coord_t>("raft_interface_line_spacing");
		raft_interface_fan_speed = settings->get<Ratio>("raft_interface_fan_speed");
		raft_interface_line_width = settings->get<coord_t>("raft_interface_line_width");
		raft_surface_thickness = settings->get<coord_t>("raft_surface_thickness");
		raft_surface_line_spacing = settings->get<coord_t>("raft_surface_line_spacing");
		raft_surface_fan_speed = settings->get<coord_t>("raft_surface_fan_speed");
		raft_surface_line_width = settings->get<coord_t>("raft_surface_line_width");
		raft_smoothing = settings->get<coord_t>("raft_smoothing");
		layer_0_z_overlap = settings->get<coord_t>("layer_0_z_overlap");

		material_diameter = settings->get<coord_t>("material_diameter");
		//retraction_prime_speed = settings->get<Velocity>("material_diameter");
		machine_extruder_cooling_fan_number = settings->get<size_t>("machine_extruder_cooling_fan_number");


		//yao
		machine_nozzle_id = settings->get<std::string>("machine_nozzle_id");
		//wang 
		prime_tower_line_width = settings->get<coord_t>("prime_tower_line_width");
		prime_tower_min_volume = settings->get<double>("prime_tower_min_volume");
		prime_tower_flow = settings->get<Ratio>("prime_tower_flow");
		initial_layer_line_width_factor = settings->get<Ratio>("initial_layer_line_width_factor");
		material_adhesion_tendency = settings->get<Ratio>("material_adhesion_tendency");
		prime_tower_wipe_enabled = settings->get<bool>("prime_tower_wipe_enabled");
		machine_nozzle_size = settings->get<coord_t>("machine_nozzle_size");
		brim_line_count = settings->get<size_t>("brim_line_count");
		skirt_brim_line_width = settings->get<coord_t>("skirt_brim_line_width");
		retraction_enable = settings->get<bool>("retraction_enable");
		retraction_amount = settings->get<double>("retraction_amount");
		retraction_extra_prime_amount = settings->get<double>("retraction_extra_prime_amount");
		retraction_retract_speed = settings->get<Velocity>("retraction_retract_speed");
		retraction_prime_speed = settings->get<Velocity>("retraction_prime_speed");
		retraction_hop = settings->get<coord_t>("retraction_hop");
		retraction_min_travel = settings->get<coord_t>("retraction_min_travel");
		retraction_extrusion_window = settings->get<double>("retraction_extrusion_window");
		retraction_count_max = settings->get<size_t>("retraction_count_max");
		switch_extruder_retraction_amount = settings->get<double>("switch_extruder_retraction_amount");
		switch_extruder_retraction_speed = settings->get<Velocity>("switch_extruder_retraction_speed");
		switch_extruder_prime_speed = settings->get<Velocity>("switch_extruder_prime_speed");
		retraction_hop_after_extruder_switch_height = settings->get<coord_t>("retraction_hop_after_extruder_switch_height");
		wipe_retraction_enable = settings->get<bool>("wipe_retraction_enable");
		wipe_retraction_amount = settings->get<double>("wipe_retraction_amount");
		wipe_retraction_retract_speed = settings->get<Velocity>("wipe_retraction_retract_speed");
		wipe_retraction_prime_speed = settings->get<Velocity>("wipe_retraction_prime_speed");
		wipe_retraction_extra_prime_amount = settings->get<double>("wipe_retraction_extra_prime_amount");
		wipe_pause = settings->get<Duration>("wipe_pause");
		wipe_hop_enable = settings->get<bool>("wipe_hop_enable");
		wipe_hop_amount = settings->get<coord_t>("wipe_hop_amount");
		wipe_hop_speed = settings->get<Velocity>("wipe_hop_speed");
		wipe_brush_pos_x = settings->get<coord_t>("wipe_brush_pos_x");
		wipe_repeat_count = settings->get<size_t>("wipe_repeat_count");
		wipe_move_distance = settings->get<coord_t>("wipe_move_distance");
		speed_travel = settings->get<Velocity>("speed_travel");
		max_extrusion_before_wipe = settings->get<double>("max_extrusion_before_wipe");
		clean_between_layers = settings->get<bool>("clean_between_layers");
		support_infill_angles = settings->get<std::vector<AngleDegrees>>("support_infill_angles");
		support_roof_pattern = settings->get<EFillMethod>("support_roof_pattern");
		support_bottom_pattern = settings->get<EFillMethod>("support_bottom_pattern");
		support_roof_angles = settings->get<std::vector<AngleDegrees>>("support_roof_angles");
		support_bottom_angles = settings->get<std::vector<AngleDegrees>>("support_bottom_angles");
		material_print_temperature_layer_0 = settings->get<Temperature>("material_print_temperature_layer_0");
		material_print_temperature = settings->get<Temperature>("material_print_temperature");
		material_standby_temperature = settings->get<Temperature>("material_standby_temperature");
		travel_avoid_other_parts = settings->get<bool>("travel_avoid_other_parts");
		wall_line_width_0 = settings->get<coord_t>("wall_line_width_0");
		prime_blob_enable = settings->get<bool>("prime_blob_enable");
		extruder_prime_pos_abs = settings->get<bool>("extruder_prime_pos_abs");
		extruder_prime_pos_x = settings->get<coord_t>("extruder_prime_pos_x");
		extruder_prime_pos_y = settings->get<coord_t>("extruder_prime_pos_y");
		extruder_prime_pos_z = settings->get<coord_t>("extruder_prime_pos_z");
		support_initial_layer_line_distance = settings->get<coord_t>("support_initial_layer_line_distance");
		infill_overlap_mm = settings->get<coord_t>("infill_overlap_mm");
		zig_zaggify_support = settings->get<bool>("zig_zaggify_support");
		support_skip_some_zags = settings->get<bool>("support_skip_some_zags");
		support_zag_skip_count = settings->get<size_t>("support_zag_skip_count");
		support_connect_zigzags = settings->get<bool>("support_connect_zigzags");
		support_roof_line_distance = settings->get<coord_t>("support_roof_line_distance");
		support_bottom_line_distance = settings->get<coord_t>("support_bottom_line_distance");
		cool_min_layer_time = settings->get<Duration>("cool_min_layer_time");
		cool_min_layer_time_fan_speed_max = settings->get<Duration>("cool_min_layer_time_fan_speed_max");
		cool_fan_speed_0 = settings->get<Ratio>("cool_fan_speed_0");
		cool_fan_speed_min = settings->get<Ratio>("cool_fan_speed_min");
		cool_fan_speed_max = settings->get<Ratio>("cool_fan_speed_max");
		cool_min_speed = settings->get<Velocity>("cool_min_speed");
		cool_fan_full_layer = settings->get<LayerIndex>("cool_fan_full_layer");
		cool_fan_enabled = settings->get<bool>("cool_fan_enabled");

		material_guid = settings->get<std::string>("material_guid");
		retraction_enable = settings->get<bool>("retraction_enable");
		material_diameter = settings->get<coord_t>("material_diameter");
		machine_extruder_cooling_fan_number = settings->get<size_t>("machine_extruder_cooling_fan_number");

		machine_extruders_share_heater = settings->get<bool>("machine_extruders_share_heater");
		machine_nozzle_temp_enabled = settings->get<bool>("machine_nozzle_temp_enabled");
		machine_firmware_retract = settings->get<bool>("machine_firmware_retract");

		machine_extruder_start_code = settings->get<std::string>("machine_extruder_start_code");
		material_flow_layer_0 = settings->get<Ratio>("material_flow_layer_0");

		raft_base_speed = settings->get<Velocity>("raft_base_speed");
		raft_base_acceleration = settings->get<Acceleration>("raft_base_acceleration");
		raft_base_jerk = settings->get<Velocity>("raft_base_jerk");

		raft_interface_speed = settings->get<Velocity>("raft_interface_speed");
		raft_interface_acceleration = settings->get<Acceleration>("raft_interface_acceleration");
		raft_interface_jerk = settings->get<Velocity>("raft_interface_jerk");

		raft_surface_speed = settings->get<Velocity>("raft_surface_speed");
		raft_surface_acceleration = settings->get<Acceleration>("raft_surface_acceleration");
		raft_surface_jerk = settings->get<Velocity>("raft_surface_jerk");

		speed_support_roof = settings->get<Velocity>("speed_support_roof");
		acceleration_support_roof = settings->get<Acceleration>("acceleration_support_roof");
		jerk_support_roof = settings->get<Velocity>("jerk_support_roof");

		support_roof_material_flow = settings->get<Ratio>("support_roof_material_flow");

		speed_support_bottom = settings->get<Velocity>("speed_support_bottom");
		acceleration_support_bottom = settings->get<Acceleration>("acceleration_support_bottom");
		jerk_support_bottom = settings->get<Velocity>("jerk_support_bottom");

		support_bottom_material_flow = settings->get<Ratio>("support_bottom_material_flow");

		speed_travel = settings->get<Velocity>("speed_travel");
		acceleration_travel = settings->get<Acceleration>("acceleration_travel");
		jerk_travel = settings->get<Velocity>("jerk_travel");

		skirt_brim_speed = settings->get<Velocity>("skirt_brim_speed");
		acceleration_skirt_brim = settings->get<Acceleration>("acceleration_skirt_brim");
		jerk_skirt_brim = settings->get<Velocity>("jerk_skirt_brim");

		skirt_brim_material_flow = settings->get<Ratio>("skirt_brim_material_flow");

		speed_prime_tower = settings->get<Velocity>("speed_prime_tower");
		acceleration_prime_tower = settings->get<Acceleration>("acceleration_prime_tower");
		jerk_prime_tower = settings->get<Velocity>("jerk_prime_tower");

		prime_tower_flow = settings->get<Ratio>("prime_tower_flow");

		speed_support_infill = settings->get<Velocity>("speed_support_infill");
		acceleration_support_infill = settings->get<Acceleration>("acceleration_support_infill");
		jerk_support_infill = settings->get<Velocity>("jerk_support_infill");

		support_material_flow = settings->get<Ratio>("support_material_flow");

		speed_print_layer_0 = settings->get<Velocity>("speed_print_layer_0");
		acceleration_print_layer_0 = settings->get<Acceleration>("acceleration_print_layer_0");
		jerk_print_layer_0 = settings->get<Velocity>("jerk_support_infill");

		speed_travel_layer_0 = settings->get<Velocity>("speed_travel_layer_0");
		acceleration_travel_layer_0 = settings->get<Acceleration>("acceleration_travel_layer_0");
		jerk_travel_layer_0 = settings->get<Velocity>("jerk_travel_layer_0");

		machine_extruder_end_pos_abs = settings->get<coord_t>("machine_extruder_end_pos_abs");
		machine_extruder_end_pos_x = settings->get<coord_t>("machine_extruder_end_pos_x");
		machine_extruder_end_pos_y = settings->get<coord_t>("machine_extruder_end_pos_y");
		machine_nozzle_offset_x = settings->get<coord_t>("machine_nozzle_offset_x");
		machine_nozzle_offset_y = settings->get<coord_t>("machine_nozzle_offset_y");
		machine_extruder_start_pos_x = settings->get<coord_t>("machine_extruder_start_pos_x");
		machine_extruder_start_pos_y = settings->get<coord_t>("machine_extruder_start_pos_y");
		machine_nozzle_tip_outer_diameter = settings->get<coord_t>("machine_nozzle_tip_outer_diameter");
		meshfix_maximum_travel_resolution = settings->get<coord_t>("meshfix_maximum_travel_resolution");
		retraction_combing_max_distance = settings->get<coord_t>("retraction_combing_max_distance");
		wall_line_width_x = settings->get<coord_t>("wall_line_width_x");

		machine_extruder_start_pos_abs = settings->get<bool>("machine_extruder_start_pos_abs");
		retraction_hop_after_extruder_switch = settings->get<bool>("retraction_hop_after_extruder_switch");
		retraction_hop_enabled = settings->get<bool>("retraction_hop_enabled");
		limit_support_retractions = settings->get<bool>("limit_support_retractions");
		retraction_hop_only_when_collides = settings->get<bool>("retraction_hop_only_when_collides");
		travel_avoid_supports = settings->get<bool>("travel_avoid_supports");
		wall_line_count = settings->get<size_t>("wall_line_count");

		material_flow_dependent_temperature = settings->get<bool>("material_flow_dependent_temperature");
		material_initial_print_temperature = settings->get<Temperature>("material_initial_print_temperature");

		material_flow_temp_graph = settings->get<FlowTempGraph>("material_flow_temp_graph");
		retract_at_layer_change = settings->get<bool>("retract_at_layer_change");

		machine_nozzle_heat_up_speed = settings->get<Temperature>("machine_nozzle_heat_up_speed");
		material_extrusion_cool_down_speed = settings->get<Temperature>("material_extrusion_cool_down_speed");
		machine_nozzle_cool_down_speed = settings->get<Temperature>("machine_nozzle_cool_down_speed");

		switch_extruder_extra_prime_amount = settings->get<double>("switch_extruder_extra_prime_amount");

		coasting_enable = settings->get<bool>("coasting_enable");
		cool_lift_head = settings->get<bool>("cool_lift_head");
		speed_z_hop = settings->get<Velocity>("speed_z_hop");
		layer_start_y = settings->get<coord_t>("layer_start_y");
		layer_start_x = settings->get<coord_t>("layer_start_x");

		coasting_volume = settings->get<double>("coasting_volume");
		coasting_min_volume = settings->get<double>("coasting_min_volume");
		coasting_speed = settings->get<Ratio>("coasting_speed");
	}
}