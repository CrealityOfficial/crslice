#ifndef CX_GCODETEMPSTRUCT_1603347900896_H
#define CX_GCODETEMPSTRUCT_1603347900896_H
#include "cxutil/input/param.h"

#include "cxutil/settings/RetractionConfig.h"
#include "cxutil/settings/WipeScriptConfig.h"
#include <map>
#include <vector>
#include <list>

#include "cxutil/math/AABB3D.h"

namespace cxutil
{
	struct LPBufferParam
	{
		bool firstGroup;
		std::string machine_start_gcode;

		bool machine_heated_build_volume;
		Temperature build_volume_temperature;

		bool relative_extrusion;
		bool retraction_enable;
		bool machine_heated_bed;
		bool material_bed_temp_prepend;
		bool material_print_temp_prepend;
		Temperature material_bed_temperature_layer_0;
		Temperature material_bed_temperature;
		bool material_bed_temp_wait;

		Velocity speed_travel;
		Point start_pos;
		int max_object_height;
		RetractionConfig retraction;

		int num_extruders;

		std::map<size_t, Temperature> prependTemperatureCommands;
		std::map<size_t, Temperature> waitTemperatureCommands;

		double flow_rate_max_extrusion_offset;
		double flow_rate_extrusion_offset_factor;

		bool acceleration_enabled;
		bool jerk_enabled;
	};

	struct GroupGCodeParam
	{
		MeshGroupParam meshGroupParam;
		AABB3D machine_size;

		std::vector<RetractionConfig> retraction_config_per_extruder;
		std::vector<RetractionConfig> extruder_switch_retraction_config_per_extruder;
		std::vector<WipeScriptConfig> wipe_config_per_extruder;
		std::vector<int> max_print_height_per_extruder;
	};
}

#endif // CX_GCODETEMPSTRUCT_1603347900896_H