#include "crslice2/gcodeprocessor.h"
#include "libslic3r/GCode/GCodeProcessor.hpp"

namespace crslice2
{
    void deal_roles_times(const std::vector<std::pair<Slic3r::ExtrusionRole, float>>& moves_times, std::vector<std::vector<std::pair<int, float>>>& times)
    {
        for (auto& custom_gcode_time : moves_times)
        {
            std::pair<int, float> _pair;
            _pair.first = (int)custom_gcode_time.first;

            if(_pair.first == 2)
                _pair.first = 1;
            else if (_pair.first >0 )
                _pair.first += 19;

            _pair.second = custom_gcode_time.second;
            times.back().push_back(_pair);
        }
    }

    void deal_moves_times(const std::vector<std::pair<Slic3r::EMoveType, float>>& roles_times, std::vector<std::vector<std::pair<int, float>>>& times)
    {
        for (auto& custom_gcode_time : roles_times)
        {
            std::pair<int, float> _pair;
            _pair.first = (int)custom_gcode_time.first;

            if (_pair.first == 1)
            {
                _pair.first =14;
            }
            else if (_pair.first == 8)
            {
                _pair.first = 13;
            }
            else
            {
                _pair.first += 40;
            }
            _pair.second = custom_gcode_time.second;
            times.back().push_back(_pair);
        }
    }

    void process_file(const std::string& file, std::vector<std::vector<std::pair<int, float>>>& times)
    {
        // process gcode
        Slic3r::GCodeProcessor processor;
        try
        {
            processor.process_file(file);

            Slic3r::PrintEstimatedStatistics::ETimeMode mode = Slic3r::PrintEstimatedStatistics::ETimeMode::Normal;
            float time = processor.get_time(mode);
            float prepare_time = processor.get_prepare_time(mode);
            const std::vector<std::pair<Slic3r::CustomGCode::Type, std::pair<float, float>>>& custom_gcode_times = processor.get_custom_gcode_times(mode, true);
            const std::vector<std::pair<Slic3r::EMoveType, float>>& moves_times = processor.get_moves_time(mode);
            const std::vector<std::pair<Slic3r::ExtrusionRole, float>>& roles_times = processor.get_roles_time(mode);
            const std::vector<float>& layers_times = processor.get_layers_time(mode);

            times.push_back(std::vector<std::pair<int, float>>());
            deal_moves_times(moves_times, times);
            deal_roles_times(roles_times, times);

            times.push_back(std::vector<std::pair<int, float>>());
            for (int i = 0; i < layers_times.size();i++)
            {
                std::pair<int, float> _pair;
                _pair.first = i-1;
                _pair.second = layers_times[i];
                times.back().push_back(_pair);
            }
        }
        catch (const std::exception& ex)
        {
            //show_error(this, ex.what());
            return;
        }
    }


}
