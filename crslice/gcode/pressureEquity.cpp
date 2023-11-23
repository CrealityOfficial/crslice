#include "pressureEquity.h"
#include "src/gcode/PressureEqualizer.h"



namespace crslice
{
	void pressureE(std::vector<std::string>& inputGcodes, std::vector<std::string>& outputGcodes, double filament_diameter, float extrusion_rate, float segment_length, bool use_relative_e_distances)
	{
		Slic3r::PressureEqualizer eq( filament_diameter, extrusion_rate, segment_length, use_relative_e_distances);
		Slic3r::LayerResult Lr;

		for (int i = 0; i < inputGcodes.size(); i++)
		{
			Lr.gcode = inputGcodes.at(i);
			Slic3r::LayerResult getLayerResult = eq.process_layer(Lr);
			outputGcodes.push_back(getLayerResult.gcode);
		}
	
	


	}
}
