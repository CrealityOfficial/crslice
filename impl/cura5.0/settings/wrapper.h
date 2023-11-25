// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef WRAPPER_SETTINGS_SETTINGS_H
#define WRAPPER_SETTINGS_SETTINGS_H
#include <string>
#include "types/Temperature.h"

namespace cura52
{
	class Settings;
	class SceneParamWrapper
	{
	public:
		SceneParamWrapper();
		~SceneParamWrapper() = default;

		void initialize(Settings* settings);

		std::string get_machine_name() const;
		bool get_relative_extrusion() const;
		bool get_machine_use_extruder_offset_to_offset_coords() const;
		bool get_always_write_active_tool() const;
		bool get_machine_heated_build_volume() const;
		Temperature get_build_volume_temperature() const;

		double get_special_slope_slice_angle() const;
		bool special_slope_slice_angle_enabled() const;

		std::string get_special_slope_slice_axis() const;
	protected:
		std::string machine_name;
		bool relative_extrusion = false; //!< whether to use relative extrusion distances rather than absolute
		bool machine_use_extruder_offset_to_offset_coords = false;
		bool always_write_active_tool = false; //!< whether to write the active tool after sending commands to inactive tool
		bool machine_heated_build_volume = false;  //!< does the machine have the ability to control/stabilize build-volume-temperature
		Temperature build_volume_temperature;  //!< build volume temperature

		double special_slope_slice_angle = 0.0;
		std::string special_slope_slice_axis;
	};
} //namespace cura52

#endif //SETTINGS_SETTINGS_H

