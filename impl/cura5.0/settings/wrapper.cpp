#include "wrapper.h"
#include "settings/Settings.h"

namespace cura52
{
	void SceneParamWrapper::initialize(Settings* settings)
	{
		if (!settings)
			return;

		special_slope_slice_angle = settings->get<double>("special_slope_slice_angle");
		special_slope_slice_axis = settings->get<std::string>("special_slope_slice_axis");
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


