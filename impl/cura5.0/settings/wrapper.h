// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef WRAPPER_SETTINGS_SETTINGS_H
#define WRAPPER_SETTINGS_SETTINGS_H
#include <string>

namespace cura52
{
	class Settings;
	class SceneParamWrapper
	{
	public:
		SceneParamWrapper() {}
		~SceneParamWrapper() = default;

		void initialize(Settings* settings);

		double get_special_slope_slice_angle() const;
		bool special_slope_slice_angle_enabled() const;

		std::string get_special_slope_slice_axis() const;
	protected:
		double special_slope_slice_angle;
		std::string special_slope_slice_axis;
	};
} //namespace cura52

#endif //SETTINGS_SETTINGS_H

