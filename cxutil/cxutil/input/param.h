#ifndef CX_PARAM_1599711174722_H
#define CX_PARAM_1599711174722_H
#include "cxutil/slicer/ExtruderTrain.h"

#include "cxutil/settings/EnumSettings.h"
#include "cxutil/settings/Duration.h"
#include "cxutil/settings/Velocity.h"
#include "cxutil/settings/LayerIndex.h"
#include "cxutil/settings/Temperature.h"
#include "cxutil/settings/PrintFeature.h"
#include "cxutil/settings/FlowTempGraph.h"
#include "cxutil/settings/Ratio.h"
#include "cxutil/settings/AngleDegrees.h"
#include "cxutil/math/Coord_t.h"

namespace cxutil
{
	class SceneInput;
	class SceneParam
	{
	public:
		SceneParam();
		~SceneParam();

		void initialize(SceneInput* scene);
	public:
		//zeng
		EGCodeFlavor  machine_gcode_flavor;
		//yi

		//liu

		//yao
	};

	class GroupInput;
	class MeshGroupParam
	{
	public:
		MeshGroupParam();
		~MeshGroupParam();

		void initialize(GroupInput* meshGroup);
	public:
		bool wireframe_enabled;
		bool carve_multiple_volumes;

		coord_t layer_height_0;
		coord_t layer_height;

		bool adaptive_layer_height_enabled;
		coord_t adaptive_layer_height_variation;
		coord_t adaptive_layer_height_variation_step;
		coord_t adaptive_layer_height_threshold;

		bool alternate_carve_order;

		bool support_brim_enable;
		bool smooth_spiralized_contours;
		//zeng
		bool remove_empty_first_layers;
		ESupportType support_type;
		bool support_tree_enable;
		coord_t support_xy_distance;
		AngleRadians support_tree_angle;
		coord_t support_line_width;
		coord_t layer_0_z_overlap;
		//yi
		coord_t machine_width;
		coord_t machine_height;
		coord_t machine_depth;

		coord_t brim_gap;


		bool  support_roof_enable;
		bool support_bottom_enable; 

		bool prime_blob_enable;
		BuildPlateShape machine_shape;
		size_t support_extruder_nr_layer_0;
		size_t support_infill_extruder_nr;   //aware
		size_t support_roof_extruder_nr;
		size_t support_bottom_extruder_nr;

		bool brim_outside_only;
		bool brim_replaces_support;

		bool support_enable;
		std::vector< AngleDegrees> support_infill_angles;
		std::vector< AngleDegrees> interface_angles_setting;
		EFillMethod support_roof_pattern;
		EFillMethod support_bottom_pattern;
		
		std::string machine_start_gcode;
		Temperature build_volume_temperature;
		coord_t retraction_amount;
		bool retraction_enable;
		Acceleration machine_acceleration;
		bool machine_use_extruder_offset_to_offset_coords;
		//liu
		bool magic_spiralize;
		//Enable exterior ooze shield. This will create a shell around the model which is likely to wipe a second nozzle if it's at the same height as the first nozzle
		bool ooze_shield_enabled;
		//Distance of the ooze shield from the print, in the X/Y directions.
		coord_t ooze_shield_dist;//
		//The maximum angle a part in the ooze shield will have. With 0 degrees being vertical, and 90 degrees being horizontal. A smaller angle leads to less failed ooze shields, but more material.
		//渗出罩角度
		AngleDegrees ooze_shield_angle;
		//启用防风罩
		//This will create a wall around the model, which traps (hot) air and shields against exterior airflow. Especially useful for materials which warp easily
		bool draft_shield_enabled;
		//draft_shield_height_limitation	防风罩限制
		//Set the height of the draft shield. Choose to print the draft shield at the full height of the model or at a limited height
		DraftShieldHeightLimitation draft_shield_height_limitation;
		//draft_shield_height	防风罩高度
		coord_t draft_shield_height;
		//draft_shield_dist	防风罩 X/Y 距离	
		coord_t draft_shield_dist;
		EGCodeFlavor  machine_gcode_flavor;//
		//bool machine_use_extruder_offset_to_offset_coords;
		std::string machine_name;
		std::string machine_buildplate_type;
		bool relative_extrusion;

		Velocity machine_max_feedrate_x;
		Velocity machine_max_feedrate_y;
		Velocity machine_max_feedrate_z;
		Velocity machine_max_feedrate_e;
		Acceleration machine_max_acceleration_x;
		Acceleration machine_max_acceleration_y;
		Acceleration machine_max_acceleration_z;
		Acceleration machine_max_acceleration_e;
		Velocity machine_max_jerk_xy;
		Velocity machine_max_jerk_z;
		Velocity machine_max_jerk_e;
		Velocity machine_minimum_feedrate;
		//Acceleration machine_acceleration;
		coord_t support_tree_branch_diameter;
		AngleRadians support_tree_branch_diameter_angle;
		coord_t support_tree_collision_resolution;

		size_t support_wall_count;//	支撑墙行数 1
		coord_t support_bottom_distance;//	//支撑底部距离 0
		coord_t support_interface_skip_height;//	支撑接触面分辨率	0.2
		coord_t support_bottom_height;// 支撑底板厚度		2.0
		Ratio	initial_layer_line_width_factor;
		coord_t infill_line_distance;
		Temperature material_print_temperature;
		AngleRadians support_conical_angle;
		EFillMethod support_pattern;
		coord_t support_infill_sparse_thickness;
		double minimum_support_area;
		coord_t support_roof_height;
		//wang
		EPlatformAdhesion adhesion_type;
		bool	prime_tower_enable;
		coord_t prime_tower_min_volume;
		coord_t prime_tower_size;
		size_t adhesion_extruder_nr;
		bool prime_tower_brim_enable;
		coord_t prime_tower_position_x;
		coord_t prime_tower_position_y;
		bool material_bed_temp_prepend;
		bool machine_heated_bed;
		Temperature material_bed_temperature_layer_0;
		bool material_bed_temp_wait;
		bool material_print_temp_prepend;
		bool material_print_temp_wait;
		bool machine_heated_build_volume;
		bool acceleration_enabled;
		bool jerk_enabled;
		std::string machine_end_gcode;
		coord_t support_brim_line_count;
		size_t speed_slowdown_layers;

		CombingMode retraction_combing;

		bool travel_retract_before_outer_wall;
		bool outer_inset_first;
		size_t wall_line_count;
		coord_t wall_line_width_0;

		coord_t wall_line_width_x;

		double flow_rate_max_extrusion_offset;
		Ratio flow_rate_extrusion_offset_factor;
		Temperature material_bed_temperature;
	};

	enum class MeshType
	{
		normal_mesh = 0,
		infill_mesh = 1 << 0,
		anti_overhang_mesh = 1 << 1,
		cutting_mesh = 1 << 2,
		support_mesh = 1 << 3
	};

	class MeshInput;
	class MeshParam
	{
	public:
		MeshParam();
		~MeshParam();

		inline bool isInfill()
		{
			return m_meshType & (int)MeshType::infill_mesh;
		}

		inline bool isAntiOverhang()
		{
			return m_meshType & (int)MeshType::anti_overhang_mesh;
		}

		inline bool isCuttingMesh()
		{
			return m_meshType & (int)MeshType::cutting_mesh;
		}

		inline bool isSupportMesh()
		{
			return m_meshType & (int)MeshType::support_mesh;
		}

		void initialize(MeshInput* mesh);
	protected:
		size_t extruderIndex(const char* name, Settings* settings);
	public:
		int m_meshType;
		coord_t minimum_polygon_circumference;


		coord_t layer_height;
		ESurfaceMode surfaceMode;

		bool meshfix_extensive_stitching;
		bool meshfix_keep_open_polygons;
		bool filter_out_tiny_gaps;

		bool mold_enabled;
		coord_t mold_width;
		coord_t mold_roof_height;

		bool conical_overhang_enabled;
		AngleRadians conical_overhang_angle;
		coord_t multiple_mesh_overlap;
		int infill_mesh_order;

		ESurfaceMode magic_mesh_surface_mode;

		bool meshfix_union_all_remove_holes;
		bool meshfix_union_all;
		coord_t hole_xy_offset;
		coord_t  xy_offset;
		EFillMethod top_bottom_pattern_0;
		EFillMethod top_bottom_pattern;

		EFillMethod ironing_pattern;
		
		coord_t ironing_line_spacing;
		coord_t ironing_inset;

		coord_t infill_overlap_mm;
		//zeng
		size_t initial_bottom_layers;
		bool support_mesh_drop_down;

		coord_t top_skin_preshrink;
		coord_t bottom_skin_preshrink;
		coord_t top_skin_expand_distance;
		coord_t bottom_skin_expand_distance;
		bool skin_no_small_gaps_heuristic;
		coord_t skin_line_width;
		EFillMethod infill_pattern;

		double min_infill_area;
		coord_t min_skin_width_for_expansion;
		coord_t infill_line_width;
		Ratio initial_layer_line_width_factor;
		AngleDegrees infill_support_angle;
		size_t gradual_infill_steps;
		coord_t gradual_infill_step_height;
		coord_t infill_sparse_thickness;
		coord_t support_top_distance;
		coord_t support_tower_maximum_supported_diameter;
		bool support_use_towers;
		AngleRadians support_angle;

		size_t support_infill_extruder_nr;
		bool support_roof_enable;
		bool support_bottom_enable;
		double minimum_support_area;

		coord_t support_roof_height;
		coord_t support_interface_skip_height;
		double minimum_roof_area;

		coord_t support_bottom_height;
		coord_t support_bottom_distance;
		double minimum_bottom_area;
		coord_t support_offset;
		coord_t support_xy_distance;
		coord_t support_xy_distance_overhang;
		SupportDistPriority support_xy_overrides_z;
		coord_t support_bottom_stair_step_width;
		coord_t support_bottom_stair_step_height;
		AngleRadians support_conical_angle;
		bool support_conical_enabled;
		coord_t support_tower_diameter;
		AngleRadians support_tower_roof_angle;
		//yi
		coord_t wall_0_inset;
		coord_t wall_line_width_0;

		coord_t wall_line_width_x;

		bool alternate_extra_perimeter;

		bool alternate_extra_perimeter_bool;

		coord_t infill_line_distance;
		size_t top_layers;
		size_t bottom_layers;
		size_t roofing_layer_count;
		size_t skin_outline_count;

		bool fill_outline_gaps;

		coord_t  meshfix_maximum_resolution;
		coord_t meshfix_maximum_deviation;

		coord_t z_seam_x;
		coord_t z_seam_y;
		bool z_seam_relative;

		bool support_enable;
		bool support_tree_enable;

		size_t wall_0_extruder_nr;
		size_t wall_x_extruder_nr;
		size_t infill_extruder_nr;
		size_t top_bottom_extruder_nr;
		size_t roofing_extruder_nr;

		//wang
		size_t extruder_nr;

		coord_t raft_margin;
		coord_t skirt_gap;
		size_t	skirt_line_count;

		bool support_brim_enable;

		FillPerimeterGapMode  fill_perimeter_gaps;
		coord_t support_line_width;
		//liu
		//Different options that help to improve both priming your extrusionand adhesion to the build plate.Brim adds a single layer flat area 
		//around the base of your model to prevent warping.Raft adds a thick grid with a roof below the model.Skirt is a line printed around the model, but not connected to the model.
		EPlatformAdhesion adhesion_type;//
		size_t wall_line_count; //	The number of walls. When calculated by the wall thickness, this value is rounded to a whole number.
		bool magic_spiralize;  //Spiralize smooths out the Z move of the outer edge. This will create a steady Z increase over the whole print. This feature turns a solid model into a single walled print with a solid bottom. This feature should only be enabled when each layer only contains a single part.
		//Inset applied to the path of the outer wall.If the outer wall is smaller than the nozzle, and printed after the inner walls, use this offset to get the hole in the nozzle to overlap with the inner walls instead of the outside of the model

		bool ironing_enabled; //熨平模式
		bool ironing_only_highest_layer;//	仅熨平最高层
		//Print infill structures only where tops of the model should be supported. Enabling this reduces print time and material usage, but leads to ununiform object strength
		bool infill_support_enabled; // 填充支撑
		coord_t infill_offset_x;
		coord_t infill_offset_y;
		//The file location of an image of which the brightness values determine the minimal density at the corresponding location in the infill of the print.
		std::string cross_infill_density_image;
		bool magic_fuzzy_skin_enabled;//	模糊皮肤
		coord_t magic_fuzzy_skin_thickness;//	模糊皮肤厚度
		coord_t magic_fuzzy_skin_point_dist;// 模糊皮肤点距离

		bool magic_fuzzy_skin_outside_only;// Jitter only the parts' outlines and not the parts' holes
		EZSeamType z_seam_type;	//Z 缝对齐
		coord_t skin_overlap_mm;// 皮肤重叠	0.17
		coord_t support_tree_branch_distance; //support_tree_branch_distance	树形支撑分支间距 1mm

		Velocity speed_topbottom;
		bool speed_equalize_flow_enabled;
		Ratio wall_x_material_flow;
		Ratio material_flow_layer_0;
		Acceleration acceleration_topbottom;
		Velocity jerk_topbottom;
		//coord_t support_tree_branch_distance;
		//yao
		//wang
		//EZSeamType z_seam_type;
		std::vector<AngleDegrees> infill_angles;
		std::vector<AngleDegrees> roofing_angles;
		std::vector<AngleDegrees> skin_angles;
		bool support_mesh;
		bool anti_overhang_mesh;
		bool cutting_mesh;
		bool infill_mesh;
		EZSeamCornerPrefType z_seam_corner;
		coord_t wall_0_wipe_dist;
		bool infill_before_walls;
		bool spaghetti_infill_enabled;
		bool zig_zaggify_infill;
		bool connect_infill_polygons;
		size_t infill_multiplier;
		coord_t cross_infill_pocket_size;
		bool infill_randomize_start_location;
		bool infill_enable_travel_optimization;
		size_t infill_wall_line_count;
		size_t skin_edge_support_layers;
		coord_t infill_wipe_dist;
		bool travel_compensate_overlapping_walls_0_enabled;
		bool travel_compensate_overlapping_walls_x_enabled;
		bool travel_retract_before_outer_wall;
		bool bridge_settings_enabled;
		AngleDegrees wall_overhang_angle;
		bool outer_inset_first;
		EFillMethod roofing_pattern;
		bool bridge_enable_more_layers;
		Ratio bridge_skin_support_threshold;
		Ratio bridge_skin_density;
		Ratio bridge_skin_density_2;
		Ratio bridge_skin_density_3;
		bool support_fan_enable;
		Ratio support_supported_skin_fan_speed;
		bool connect_skin_polygons;

		Ratio wall_0_material_flow;
		Velocity speed_wall_0;
		Acceleration acceleration_wall_0;
		Velocity jerk_wall_0;

		Velocity speed_wall_x;
		Acceleration acceleration_wall_x;
		Velocity jerk_wall_x;

		Ratio bridge_wall_material_flow;
		Velocity bridge_wall_speed;
		Ratio bridge_fan_speed;
		Ratio skin_material_flow;
		Ratio bridge_skin_material_flow;
		Velocity bridge_skin_speed;

		coord_t roofing_line_width;
		Ratio roofing_material_flow;
		Velocity speed_roofing;
		Acceleration acceleration_roofing;
		Velocity jerk_roofing;

		Ratio bridge_skin_material_flow_2;
		Velocity bridge_skin_speed_2;
		Velocity bridge_fan_speed_2;

		Ratio bridge_skin_material_flow_3;
		Velocity bridge_skin_speed_3;
		Velocity bridge_fan_speed_3;


		Ratio ironing_flow;
		Velocity speed_ironing;
		Acceleration acceleration_ironing;
		Velocity jerk_ironing;

		Ratio infill_material_flow;
		Velocity speed_infill;
		Acceleration acceleration_infill;
		Velocity jerk_infill;

		Velocity speed_print_layer_0;
		Acceleration acceleration_print_layer_0;
		Velocity jerk_print_layer_0;

		CombingMode retraction_combing;
		bool optimize_wall_printing_order;

		coord_t bridge_wall_min_length;
		Ratio wall_min_flow;
		bool wall_min_flow_retract;
		coord_t small_feature_max_length;
		Ratio small_feature_speed_factor_0;
		Ratio small_feature_speed_factor;
		Ratio bridge_wall_coast;
		Ratio wall_overhang_speed_factor;
		Ratio bridge_sparse_infill_max_density;
	};

	class ExtruderInput;
	class ExtruderParam
	{
	public:
		ExtruderParam();
		~ExtruderParam();

		void initialize(ExtruderInput* extruder);
	public:

		//zeng
		coord_t support_line_width;
		coord_t support_roof_line_width;
		coord_t support_roof_offset;
		Duration machine_min_cool_heat_time_window;
		Temperature material_final_print_temperature;
		std::string machine_extruder_end_code;

		coord_t support_bottom_line_width;
		coord_t support_bottom_offset;
		EFillMethod support_pattern;
		std::string cross_support_density_image;
		coord_t support_line_distance;
		coord_t support_offset;
		size_t support_wall_count;
		coord_t support_top_distance;
		coord_t support_xy_distance;
		coord_t support_xy_distance_overhang;
		SupportDistPriority support_xy_overrides_z;
		AngleRadians support_angle;
		coord_t support_tower_maximum_supported_diameter;
		bool support_use_towers;
		coord_t support_bottom_distance;
		AngleRadians support_conical_angle;
		bool support_conical_enabled;
		coord_t support_tower_diameter;
		AngleRadians support_tower_roof_angle;
		coord_t raft_margin;
		coord_t skirt_gap;
		size_t skirt_line_count;
		coord_t support_conical_min_width;
		coord_t support_join_distance;
		coord_t support_infill_sparse_thickness;
		coord_t gradual_support_infill_step_height;
		size_t gradual_support_infill_steps;
		EPlatformAdhesion adhesion_type;
		size_t raft_surface_layers;
		coord_t raft_airgap;
		size_t support_brim_line_count;
		//yi
		coord_t skirt_brim_minimal_length;
		coord_t meshfix_maximum_resolution;
		coord_t meshfix_maximum_deviation;
		bool brim_outside_only;
		bool prime_tower_brim_enable;
		bool brim_replaces_support;
		bool support_brim_enable;
		Duration cool_min_layer_time;
		Duration cool_min_layer_time_fan_speed_max;
		double cool_fan_speed_0;
		double cool_fan_speed_min;
		double cool_fan_speed_max;
		Velocity cool_min_speed;
		LayerIndex cool_fan_full_layer;
		bool cool_fan_enabled;
		
		coord_t material_diameter;
		Velocity retraction_prime_speed;
		size_t machine_extruder_cooling_fan_number;

		double retraction_amount;
		double retraction_extra_prime_amount;
		Velocity retraction_retract_speed;
		coord_t retraction_hop;
		coord_t retraction_min_travel;
		double retraction_extrusion_window;
		size_t retraction_count_max;
		double switch_extruder_retraction_amount;
		Velocity switch_extruder_retraction_speed;
		Velocity switch_extruder_prime_speed;
		coord_t retraction_hop_after_extruder_switch_height;


		bool wipe_retraction_enable;
		double wipe_retraction_amount;
		Velocity wipe_retraction_retract_speed;
		Velocity wipe_retraction_prime_speed;
		double wipe_retraction_extra_prime_amount;
		Duration wipe_pause;
		bool wipe_hop_enable;
		coord_t wipe_hop_amount;
		Velocity wipe_hop_speed;
		coord_t wipe_brush_pos_x;
		size_t wipe_repeat_count;
		coord_t wipe_move_distance;
		Velocity speed_travel;
		Acceleration acceleration_travel;
		Velocity jerk_travel;
		double max_extrusion_before_wipe;
		bool clean_between_layers;

		bool travel_avoid_other_parts;
		coord_t wall_line_width_0;
		bool prime_blob_enable;
		bool extruder_prime_pos_abs;
		coord_t extruder_prime_pos_x;
		coord_t extruder_prime_pos_y;
		coord_t extruder_prime_pos_z;
		coord_t layer_start_y;
		coord_t layer_start_x;
		coord_t support_initial_layer_line_distance;
		coord_t infill_overlap_mm;
		bool zig_zaggify_support;
		bool support_skip_some_zags;
		size_t support_zag_skip_count;
		bool support_connect_zigzags;
		EFillMethod support_roof_pattern;
		coord_t support_roof_line_distance;
		EFillMethod support_bottom_pattern;
		coord_t support_bottom_line_distance;
		//liu
				///about raft 
		coord_t raft_base_thickness;//	底座 基础厚度
		coord_t raft_base_line_spacing;//	Raft 基础走线间距
		Ratio raft_base_fan_speed;//	Raft 基础走线间距
		coord_t raft_base_line_width;	//Raft 基础走线宽度
		coord_t travel_avoid_distance;// 空驶避让距离
		coord_t raft_interface_thickness;// 底座 中间厚度
		coord_t raft_interface_line_spacing;	//底座 中间间距
		Ratio raft_interface_fan_speed;//Raft 中间风扇速度
		coord_t raft_interface_line_width;//  底座 中间线宽度
		coord_t raft_surface_thickness;//	底座 顶层厚度
		coord_t raft_surface_line_spacing;//	底座 顶部间距
		Ratio raft_surface_fan_speed;// Raft 顶部风扇速度
		coord_t raft_surface_line_width;//	底座 顶线宽度
		coord_t raft_smoothing;	//底座 平滑度

		coord_t layer_0_z_overlap;

		//yao
		std::string material_guid;
		std::string machine_nozzle_id;
		//wang
		coord_t prime_tower_line_width;
		double	prime_tower_min_volume;

		Ratio	prime_tower_flow;
		Velocity speed_prime_tower;
		Acceleration acceleration_prime_tower;
		Velocity jerk_prime_tower;

		Ratio	initial_layer_line_width_factor;
		Ratio	material_adhesion_tendency;
		bool	prime_tower_wipe_enabled;
		coord_t machine_nozzle_size;
		size_t	brim_line_count;
		coord_t	skirt_brim_line_width;
		std::vector<AngleDegrees> support_infill_angles;
		std::vector<AngleDegrees> support_roof_angles;
		std::vector<AngleDegrees> support_bottom_angles;
		Temperature material_print_temperature_layer_0;
		Temperature material_print_temperature;
		Temperature material_standby_temperature;
		bool retraction_enable;

		bool machine_extruders_share_heater;
		bool machine_nozzle_temp_enabled;
		bool machine_firmware_retract;

		std::string machine_extruder_start_code;
		Ratio material_flow_layer_0;

		Velocity raft_base_speed;
		Acceleration raft_base_acceleration;
		Velocity raft_base_jerk;

		Velocity raft_interface_speed;
		Acceleration raft_interface_acceleration;
		Velocity raft_interface_jerk;

		Velocity raft_surface_speed;
		Acceleration raft_surface_acceleration;
		Velocity raft_surface_jerk;

		Ratio support_roof_material_flow;
		Velocity speed_support_roof;
		Acceleration acceleration_support_roof;
		Velocity jerk_support_roof;

		Ratio support_bottom_material_flow;
		Velocity speed_support_bottom;
		Acceleration acceleration_support_bottom;
		Velocity jerk_support_bottom;

		Ratio skirt_brim_material_flow;
		Velocity skirt_brim_speed;
		Acceleration acceleration_skirt_brim;
		Velocity jerk_skirt_brim;

		Ratio support_material_flow;
		Velocity speed_support_infill;
		Acceleration acceleration_support_infill;
		Velocity jerk_support_infill;

		Velocity speed_print_layer_0;
		Acceleration acceleration_print_layer_0;
		Velocity jerk_print_layer_0;

		Velocity speed_travel_layer_0;
		Acceleration acceleration_travel_layer_0;
		Velocity jerk_travel_layer_0;

		coord_t machine_extruder_end_pos_abs;
		coord_t machine_extruder_end_pos_x;
		coord_t machine_extruder_end_pos_y;
		coord_t machine_nozzle_offset_x;
		coord_t machine_nozzle_offset_y;

		bool machine_extruder_start_pos_abs;
		coord_t machine_extruder_start_pos_x;
		coord_t machine_extruder_start_pos_y;

		bool retraction_hop_after_extruder_switch;
		bool retraction_hop_enabled;
		coord_t machine_nozzle_tip_outer_diameter;
		bool limit_support_retractions;
		coord_t meshfix_maximum_travel_resolution;
		coord_t retraction_combing_max_distance;

		size_t wall_line_count;
		coord_t wall_line_width_x;

		bool retraction_hop_only_when_collides;
		bool travel_avoid_supports;
		bool material_flow_dependent_temperature;
		Temperature material_initial_print_temperature;
		FlowTempGraph material_flow_temp_graph;
		bool retract_at_layer_change;

		Temperature machine_nozzle_heat_up_speed;
		Temperature material_extrusion_cool_down_speed;
		Temperature machine_nozzle_cool_down_speed;

		double switch_extruder_extra_prime_amount;
		Velocity speed_z_hop;
		bool coasting_enable;
		bool cool_lift_head;
		
		double coasting_volume;
		double coasting_min_volume;
		Ratio coasting_speed;
	};
}

#endif // CX_PARAM_1599711174722_H