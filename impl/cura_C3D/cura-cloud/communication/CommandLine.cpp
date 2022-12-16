//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#include <cstring> //For strtok and strcopy.
#include <fstream> //To check if files exist.
#include <errno.h> // error number when trying to read file
#include <numeric> //For std::accumulate.
#ifdef _OPENMP
    #include <omp.h> //To change the number of threads to slice with.
#endif //_OPENMP
#include <unordered_set>

#include "CommandLine.h"
#include "../Application.h" //To get the extruders for material estimates.
#include "cxutil/slicer/ExtruderTrain.h"
#include "../FffProcessor.h" //To start a slice and get time estimates.
#include "../Slice.h"
#include "cxutil/math/floatpoint.h"
#include "cxutil/util/loadsettings.h"
#include "cxutil/util/loadmesh.h"
#include "cxutil/settings/PrintFeature.h"
#include "cxutil/settings/Settings.h"
#include "cxutil/slicer/mesh.h"
#include "cxutil/util/print.h"
#include "cxutil/util/logoutput.h"


namespace cxutil
{

    CommandLine::CommandLine(const std::vector<std::string>& arguments)
        : arguments(arguments)
    {
    }

    //These are not applicable to command line slicing.
    void CommandLine::beginGCode() { }
    void CommandLine::flushGCode() { }
    void CommandLine::sendCurrentPosition(const Point&) { }
    void CommandLine::sendFinishedSlicing() const { }
    void CommandLine::sendLayerComplete(const LayerIndex&, const coord_t&, const coord_t&) { }
    void CommandLine::sendLineTo(const PrintFeatureType&, const Point&, const coord_t&, const coord_t&, const Velocity&) { }
    void CommandLine::sendOptimizedLayerData() { }
    void CommandLine::sendPolygon(const PrintFeatureType&, const ConstPolygonRef&, const coord_t&, const coord_t&, const Velocity&) { }
    void CommandLine::sendPolygons(const PrintFeatureType&, const Polygons&, const coord_t&, const coord_t&, const Velocity&) { }
    void CommandLine::setExtruderForSend(const ExtruderTrain&) { }
    void CommandLine::setLayerForSend(const LayerIndex&) { }

    bool CommandLine::hasSlice() const
    {
        return !arguments.empty();
    }

    bool CommandLine::isSequential() const
    {
        return true; //We have to receive the g-code in sequential order. Start g-code before the rest and so on.
    }

    void CommandLine::sendGCodePrefix(const std::string&) const
    {
        //TODO: Right now this is done directly in the g-code writer. For consistency it should be moved here?
    }

    void CommandLine::sendPrintTimeMaterialEstimates() const
    {
    }

    void CommandLine::sendProgress(const float& progress) const
    {
    }

    void CommandLine::sliceNext()
    {
        //Count the number of mesh groups to slice for.
        size_t num_mesh_groups = 1;
        for (size_t argument_index = 2; argument_index < arguments.size(); argument_index++)
        {
            if (arguments[argument_index].find("--next") == 0) //Starts with "--next".
            {
                num_mesh_groups++;
            }
        }
        Slice slice(num_mesh_groups);
        slice.scene.communication = this;
        slice.scene.progress.communication = this;
        slice.scene.processor.gcode_writer.scene = &slice.scene;
        slice.scene.processor.gcode_writer.gcode.scene = &slice.scene;
        slice.scene.processor.polygon_generator.scene = &slice.scene;
        slice.scene.processor.gcode_writer.layer_plan_buffer.scene = &slice.scene;
        slice.scene.processor.gcode_writer.layer_plan_buffer.preheat_config.scene = &slice.scene;
        slice.scene.settings.extruders = &slice.scene.extruders;
        for(size_t i = 0; i < num_mesh_groups; ++i)
            slice.scene.mesh_groups[i].settings->extruders = &slice.scene.extruders;

        size_t mesh_group_index = 0;
        Settings* last_settings = &slice.scene.settings;

        slice.scene.extruders.reserve(arguments.size() >> 1); //Allocate enough memory to prevent moves.
        slice.scene.extruders.emplace_back(0, &slice.scene.settings); //Always have one extruder.
        ExtruderTrain* last_extruder = &slice.scene.extruders[0];

        for (size_t argument_index = 2; argument_index < arguments.size(); argument_index++)
        {
            std::string argument = arguments[argument_index];
            if (argument[0] == '-') //Starts with "-".
            {
                if (argument[1] == '-') //Starts with "--".
                {
                    if (argument.find("--next") == 0) //Starts with "--next".
                    {
                        try
                        {
                            mesh_group_index++;
                            last_settings = slice.scene.mesh_groups[mesh_group_index].settings;
                        }
                        catch (...)
                        {
                            //Catch all exceptions.
                            //This prevents the "something went wrong" dialogue on Windows to pop up on a thrown exception.
                            //Only ClipperLib currently throws exceptions. And only in the case that it makes an internal error.
                            LOGE("Unknown exception!\n");
                        }
                    }
                    else
                    {
                        LOGE("Unknown option: %s\n", argument.c_str());
                    }
                }
                else //Starts with "-" but not with "--".
                {
                    argument = arguments[argument_index];
                    switch (argument[1])
                    {
                    case 'v':
                    {
                        break;
                    }
#ifdef _OPENMP
                    case 'm':
                    {
                        int threads = stoi(argument.substr(2));
                        threads = std::max(1, threads);
                        omp_set_num_threads(threads);
                        break;
                    }
#endif //_OPENMP
                    case 'p':
                    {
                        break;
                    }
                    case 'j':
                    {
                        argument_index++;
                        if (argument_index >= arguments.size())
                        {
                            LOGE("Missing JSON file with -j argument.");
                        }
                        argument = arguments[argument_index];
                        if (loadSettingsJSON(argument, last_settings, slice.scene.extruders, &slice.scene.settings))
                        {
                            LOGE("Failed to load JSON file: %s\n", argument.c_str());
                        }

                        //If this was the global stack, create extruders for the machine_extruder_count setting.
                        if (last_settings == &slice.scene.settings)
                        {
                            const size_t extruder_count = slice.scene.settings.get<size_t>("machine_extruder_count");
                            while (slice.scene.extruders.size() < extruder_count)
                            {
                                slice.scene.extruders.emplace_back(slice.scene.extruders.size(), &slice.scene.settings);
                            }

                            for (size_t i = 0; i < extruder_count; ++i)
                            {
                                slice.scene.extruders[i].settings->extruders = &slice.scene.extruders;
                            }
                        }
                        //If this was an extruder stack, make sure that the extruder_nr setting is correct.
                        if (last_settings == last_extruder->settings)
                        {
                            last_extruder->settings->add("extruder_nr", std::to_string(last_extruder->extruder_nr));
                        }
                        break;
                    }
                    case 'e':
                    {
                        size_t extruder_nr = stoul(argument.substr(2));
                        while (slice.scene.extruders.size() <= extruder_nr) //Make sure we have enough extruders up to the extruder_nr that the user wanted.
                        {
                            slice.scene.extruders.emplace_back(extruder_nr, &slice.scene.settings);
                        }
                        last_settings = slice.scene.extruders[extruder_nr].settings;
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
                        }
                        argument = arguments[argument_index];

                        const FMatrix3x3 transformation = last_settings->get<FMatrix3x3>("mesh_rotation_matrix"); //The transformation applied to the model when loaded.
                        FMatrix4x4 fmatrix;
                        if (!loadMeshIntoMeshGroup(&slice.scene.mesh_groups[mesh_group_index], argument.c_str(), fmatrix, last_extruder->settings))
                        {
                            LOGE("Failed to load model: %s. (error number %d)\n", argument.c_str(), errno);
                        }
                        else
                        {
                            last_settings = slice.scene.mesh_groups[mesh_group_index].meshes.back()->settings;
                            last_settings->extruders = &slice.scene.extruders;
                        }
                        break;
                    }
                    case 'o':
                    {
                        argument_index++;
                        if (argument_index >= arguments.size())
                        {
                            LOGE("Missing output file with -o argument.");
                        }
                        argument = arguments[argument_index];
                        if (!slice.scene.processor.setTargetFile(argument.c_str()))
                        {
                            LOGE("Failed to open %s for output.\n", argument.c_str());
                        }
                        break;
                    }
                    case 'g':
                    {
                        last_settings = slice.scene.mesh_groups[mesh_group_index].settings;
                        break;
                    }
                    /* ... falls through ... */
                    case 's':
                    {
                        //Parse the given setting and store it.
                        argument_index++;
                        if (argument_index >= arguments.size())
                        {
                            LOGE("Missing setting name and value with -s argument.");
                        }
                        argument = arguments[argument_index];
                        const size_t value_position = argument.find("=");
                        std::string key = argument.substr(0, value_position);
                        if (value_position == std::string::npos)
                        {
                            LOGE("Missing value in setting argument: -s %s", argument.c_str());
                        }
                        std::string value = argument.substr(value_position + 1);
                        last_settings->add(key, value);
                        break;
                    }
                    default:
                    {
                        LOGE("Unknown option: -%c\n", argument[1]);
                        printCall(arguments);
                        printHelp();
                        break;
                    }
                    }
                }
            }
            else
            {
                LOGE("Unknown option: %s\n", argument.c_str());
                printCall(arguments);
                printHelp();
            }
        }

        arguments.clear(); //We've processed all arguments now.

#ifndef DEBUG
        try
        {
#endif //DEBUG
            slice.scene.mesh_groups[mesh_group_index].finalize();

            //Start slicing.
            slice.compute();
#ifndef DEBUG
        }
        catch (...)
        {
            //Catch all exceptions.
            //This prevents the "something went wrong" dialogue on Windows to pop up on a thrown exception.
            //Only ClipperLib currently throws exceptions. And only in the case that it makes an internal error.
            LOGE("Unknown exception.\n");
            exit(1);
        }
#endif //DEBUG

		slice.reset();
    }
} //namespace cxutil