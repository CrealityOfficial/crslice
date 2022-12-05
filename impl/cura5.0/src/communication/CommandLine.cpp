// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include <cstring> //For strtok and strcopy.
#include <errno.h> // error number when trying to read file
#include <filesystem>
#include <fstream> //To check if files exist.
#include <numeric> //For std::accumulate.
#include <unordered_set>

#include "crcommon/jsonloader.h"

#include "ccglobal/log.h"

#include "Application.h" //To get the extruders for material estimates.
#include "ExtruderTrain.h"
#include "FffProcessor.h" //To start a slice and get time estimates.
#include "Slice.h"
#include "communication/CommandLine.h"
#include "utils/FMatrix4x3.h" //For the mesh_rotation_matrix setting.

namespace cura52
{
    bool _loadJson(const std::string& json_filename, cura52::Settings* settings)
    {
        std::vector<crcommon::KValues> extruderKVs;
        int r = crcommon::loadJSON(json_filename, settings->settings, extruderKVs);
        if (r == 0)
        {
            size_t extruder_nr = extruderKVs.size();
            Scene& scene = Application::getInstance().current_slice->scene;
            while (scene.extruders.size() <= static_cast<size_t>(extruder_nr))
            {
                scene.extruders.emplace_back(scene.extruders.size(), &scene.settings);
            }

            for (size_t i = 0; i < extruder_nr; ++i)
            {
                cura52::ExtruderTrain& extruder = scene.extruders.at(i);
                extruder.settings.settings.swap(extruderKVs.at(i));
            }
        }
        return r == 0;
    }

    CommandLine::CommandLine()
        : last_shown_progress(0)
    {
    }

    CommandLine::CommandLine(const std::vector<std::string>& arguments)
        : arguments(arguments)
        , last_shown_progress(0)
    {
    }

    // These are not applicable to command line slicing.
    void CommandLine::beginGCode()
    {
    }

    void CommandLine::flushGCode()
    {
    }

    void CommandLine::sendCurrentPosition(const Point&)
    {
    }

    void CommandLine::sendFinishedSlicing() const
    {
    }

    void CommandLine::sendLayerComplete(const LayerIndex&, const coord_t&, const coord_t&)
    {
    }

    void CommandLine::sendLineTo(const PrintFeatureType&, const Point&, const coord_t&, const coord_t&, const Velocity&)
    {
    }

    void CommandLine::sendOptimizedLayerData()
    {
    }

    void CommandLine::sendPolygon(const PrintFeatureType&, const ConstPolygonRef&, const coord_t&, const coord_t&, const Velocity&)
    {
    }

    void CommandLine::sendPolygons(const PrintFeatureType&, const Polygons&, const coord_t&, const coord_t&, const Velocity&)
    {
    }

    void CommandLine::setExtruderForSend(const ExtruderTrain&)
    {
    }

    void CommandLine::setLayerForSend(const LayerIndex&)
    {
    }

    bool CommandLine::hasSlice() const
    {
        return !arguments.empty();
    }

    bool CommandLine::isSequential() const
    {
        return true; // We have to receive the g-code in sequential order. Start g-code before the rest and so on.
    }

    void CommandLine::sendGCodePrefix(const std::string&) const
    {
        // TODO: Right now this is done directly in the g-code writer. For consistency it should be moved here?
    }

    void CommandLine::sendSliceUUID(const std::string& slice_uuid) const
    {
        // pass
    }

    void CommandLine::sendPrintTimeMaterialEstimates() const
    {
        std::vector<Duration> time_estimates = FffProcessor::getInstance()->getTotalPrintTimePerFeature();
        double sum = std::accumulate(time_estimates.begin(), time_estimates.end(), 0.0);
        LOGI("Total print time: {:3}", sum);

        sum = 0.0;
        for (size_t extruder_nr = 0; extruder_nr < Application::getInstance().current_slice->scene.extruders.size(); extruder_nr++)
        {
            sum += FffProcessor::getInstance()->getTotalFilamentUsed(extruder_nr);
        }
    }

    void CommandLine::sendProgress(const float& progress) const
    {
        const unsigned int rounded_amount = 100 * progress;
        if (last_shown_progress == rounded_amount) // No need to send another tiny update step.
        {
            return;
        }
        // TODO: Do we want to print a progress bar? We'd need a better solution to not have that progress bar be ruined by any logging.
    }

    void CommandLine::sliceNext()
    {
        FffProcessor::getInstance()->time_keeper.restart();

        // Count the number of mesh groups to slice for.
        size_t num_mesh_groups = 1;
        for (size_t argument_index = 2; argument_index < arguments.size(); argument_index++)
        {
            if (arguments[argument_index].find("--next") == 0) // Starts with "--next".
            {
                num_mesh_groups++;
            }
        }
        Slice slice(num_mesh_groups);

        Application::getInstance().current_slice = &slice;

        size_t mesh_group_index = 0;
        Settings* last_settings = &slice.scene.settings;

        slice.scene.extruders.reserve(arguments.size() >> 1); // Allocate enough memory to prevent moves.
        slice.scene.extruders.emplace_back(0, &slice.scene.settings); // Always have one extruder.
        ExtruderTrain* last_extruder = &slice.scene.extruders[0];

        for (size_t argument_index = 2; argument_index < arguments.size(); argument_index++)
        {
            std::string argument = arguments[argument_index];
            if (argument[0] == '-') // Starts with "-".
            {
                if (argument[1] == '-') // Starts with "--".
                {
                    if (argument.find("--next") == 0) // Starts with "--next".
                    {
                        try
                        {
                            LOGI("Loaded from disk in {}", FffProcessor::getInstance()->time_keeper.restart());

                            mesh_group_index++;
                            FffProcessor::getInstance()->time_keeper.restart();
                            last_settings = &slice.scene.mesh_groups[mesh_group_index].settings;
                        }
                        catch (...)
                        {
                            // Catch all exceptions.
                            // This prevents the "something went wrong" dialogue on Windows to pop up on a thrown exception.
                            // Only ClipperLib currently throws exceptions. And only in the case that it makes an internal error.
                            LOGE("Unknown exception!");
                            exit(1);
                        }
                    }
                    else
                    {
                        LOGE("Unknown option: {}", argument);
                    }
                }
                else // Starts with "-" but not with "--".
                {
                    argument = arguments[argument_index];
                    switch (argument[1])
                    {
                    case 'v':
                    {
                        LOGLEVEL(1);
                        break;
                    }
                    case 'm':
                    {
                        int threads = stoi(argument.substr(2));
                        Application::getInstance().startThreadPool(threads);
                        break;
                    }
                    case 'p':
                    {
                        // enableProgressLogging(); FIXME: how to handle progress logging? Is this still relevant?
                        break;
                    }
                    case 'j':
                    {
                        argument_index++;
                        if (argument_index >= arguments.size())
                        {
                            LOGE("Missing JSON file with -j argument.");
                            exit(1);
                        }
                        argument = arguments[argument_index];
                        if (_loadJson(argument, last_settings))
                        {
                            LOGE("Failed to load JSON file: %s", argument.c_str());
                            exit(1);
                        }

                        // If this was the global stack, create extruders for the machine_extruder_count setting.
                        if (last_settings == &slice.scene.settings)
                        {
                            const size_t extruder_count = slice.scene.settings.get<size_t>("machine_extruder_count");
                            while (slice.scene.extruders.size() < extruder_count)
                            {
                                slice.scene.extruders.emplace_back(slice.scene.extruders.size(), &slice.scene.settings);
                            }
                        }
                        // If this was an extruder stack, make sure that the extruder_nr setting is correct.
                        if (last_settings == &last_extruder->settings)
                        {
                            last_extruder->settings.add("extruder_nr", std::to_string(last_extruder->extruder_nr));
                        }
                        break;
                    }
                    case 'e':
                    {
                        size_t extruder_nr = stoul(argument.substr(2));
                        while (slice.scene.extruders.size() <= extruder_nr) // Make sure we have enough extruders up to the extruder_nr that the user wanted.
                        {
                            slice.scene.extruders.emplace_back(extruder_nr, &slice.scene.settings);
                        }
                        last_settings = &slice.scene.extruders[extruder_nr].settings;
                        last_settings->add("extruder_nr", argument.substr(2));
                        last_extruder = &slice.scene.extruders[extruder_nr];
                        break;
                    }
                    case 'l':
                    {
                        argument_index++;
                        if (argument_index >= arguments.size())
                        {
                            LOGE("Missing model file with -l argument.");
                            exit(1);
                        }
                        argument = arguments[argument_index];

                        const FMatrix4x3 transformation = last_settings->get<FMatrix4x3>("mesh_rotation_matrix"); // The transformation applied to the model when loaded.

                        if (!loadMeshIntoMeshGroup(&slice.scene.mesh_groups[mesh_group_index], argument.c_str(), transformation, last_extruder->settings))
                        {
                            LOGE("Failed to load model: {}. (error number {})", argument, errno);
                            exit(1);
                        }
                        else
                        {
                            last_settings = &slice.scene.mesh_groups[mesh_group_index].meshes.back().settings;
                        }
                        break;
                    }
                    case 'o':
                    {
                        argument_index++;
                        if (argument_index >= arguments.size())
                        {
                            LOGE("Missing output file with -o argument.");
                            exit(1);
                        }
                        argument = arguments[argument_index];
                        if (!FffProcessor::getInstance()->setTargetFile(argument.c_str()))
                        {
                            LOGE("Failed to open {} for output.", argument.c_str());
                            exit(1);
                        }
                        break;
                    }
                    case 'g':
                    {
                        last_settings = &slice.scene.mesh_groups[mesh_group_index].settings;
                        break;
                    }
                    /* ... falls through ... */
                    case 's':
                    {
                        // Parse the given setting and store it.
                        argument_index++;
                        if (argument_index >= arguments.size())
                        {
                            LOGE("Missing setting name and value with -s argument.");
                            exit(1);
                        }
                        argument = arguments[argument_index];
                        const size_t value_position = argument.find("=");
                        std::string key = argument.substr(0, value_position);
                        if (value_position == std::string::npos)
                        {
                            LOGE("Missing value in setting argument: -s {}", argument);
                            exit(1);
                        }
                        std::string value = argument.substr(value_position + 1);
                        last_settings->add(key, value);
                        break;
                    }
                    default:
                    {
                        LOGE("Unknown option: -{}", argument[1]);
                        Application::getInstance().printHelp();
                        exit(1);
                        break;
                    }
                    }
                }
            }
            else
            {
                LOGE("Unknown option: {}", argument);
                Application::getInstance().printHelp();
                exit(1);
            }
        }

        arguments.clear(); // We've processed all arguments now.

#ifndef DEBUG
        try
        {
#endif // DEBUG
            slice.scene.mesh_groups[mesh_group_index].finalize();
            LOGI("Loaded from disk in {:3}s\n", FffProcessor::getInstance()->time_keeper.restart());

            // Start slicing.
            slice.compute();
#ifndef DEBUG
        }
        catch (...)
        {
            // Catch all exceptions.
            // This prevents the "something went wrong" dialogue on Windows to pop up on a thrown exception.
            // Only ClipperLib currently throws exceptions. And only in the case that it makes an internal error.
            LOGE("Unknown exception.");
            exit(1);
        }
#endif // DEBUG

        // Finalize the processor. This adds the end g-code and reports statistics.
        FffProcessor::getInstance()->finalize();
    }

} // namespace cura52