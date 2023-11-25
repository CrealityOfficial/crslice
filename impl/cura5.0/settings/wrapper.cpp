#include "wrapper.h"
#include "settings/Settings.h"

namespace cura52
{
	SceneParamWrapper::SceneParamWrapper()
	{

	}

	void SceneParamWrapper::initialize(Settings* settings)
	{
		if (!settings)
			return;

		machine_name = settings->get<std::string>("machine_name");
		relative_extrusion = settings->get<bool>("relative_extrusion");
		machine_use_extruder_offset_to_offset_coords = settings->get<bool>("machine_use_extruder_offset_to_offset_coords");
		always_write_active_tool = settings->get<bool>("machine_always_write_active_tool");
		machine_heated_build_volume = settings->get<bool>("machine_heated_build_volume");
		build_volume_temperature = settings->get<Temperature>("build_volume_temperature");

		special_slope_slice_angle = settings->get<double>("special_slope_slice_angle");
		special_slope_slice_axis = settings->get<std::string>("special_slope_slice_axis");
	}

	std::string SceneParamWrapper::get_machine_name() const
	{
		return machine_name;
	}

	bool SceneParamWrapper::get_relative_extrusion() const
	{
		return relative_extrusion;
	}

	bool SceneParamWrapper::get_machine_use_extruder_offset_to_offset_coords() const
	{
		return machine_use_extruder_offset_to_offset_coords;
	}

	bool SceneParamWrapper::get_always_write_active_tool() const
	{
		return always_write_active_tool;
	}
	
	bool SceneParamWrapper::get_machine_heated_build_volume() const
	{
		return machine_heated_build_volume;
	}

	Temperature SceneParamWrapper::get_build_volume_temperature() const
	{
		return build_volume_temperature;
	}

	double SceneParamWrapper::get_special_slope_slice_angle() const
	{
		return special_slope_slice_angle;
	}

	bool SceneParamWrapper::special_slope_slice_angle_enabled() const
	{
		return special_slope_slice_angle != 0.0;
	}

	std::string SceneParamWrapper::get_special_slope_slice_axis() const 
	{
		return special_slope_slice_axis;
	}
} //namespace cura52


