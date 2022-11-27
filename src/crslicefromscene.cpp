#include "crslicefromscene.h"
#include "ccglobal/log.h"

#include "Application.h"
#include "ExtruderTrain.h"
#include "FffProcessor.h"
#include "Slice.h"

namespace crslice
{
    CRSliceFromScene::CRSliceFromScene(CrScenePtr scene)
        : m_sliced(false)
        , m_scene(scene)
    {
    }

    CRSliceFromScene::~CRSliceFromScene()
    {
    }

    void CRSliceFromScene::beginGCode()
    {
    }

    void CRSliceFromScene::flushGCode()
    {
    }

    void CRSliceFromScene::sendCurrentPosition(const cura::Point&)
    {
    }

    void CRSliceFromScene::sendFinishedSlicing() const
    {
    }

    void CRSliceFromScene::sendLayerComplete(const cura::LayerIndex&, const cura::coord_t&, const cura::coord_t&)
    {
    }

    void CRSliceFromScene::sendLineTo(const cura::PrintFeatureType&, const cura::Point&, const cura::coord_t&, const cura::coord_t&, const cura::Velocity&)
    {
    }

    void CRSliceFromScene::sendOptimizedLayerData()
    {
    }

    void CRSliceFromScene::sendPolygon(const cura::PrintFeatureType&, const cura::ConstPolygonRef&, const cura::coord_t&, const cura::coord_t&, const cura::Velocity&)
    {
    }

    void CRSliceFromScene::sendPolygons(const cura::PrintFeatureType&, const cura::Polygons&, const cura::coord_t&, const cura::coord_t&, const cura::Velocity&)
    {
    }

    void CRSliceFromScene::setExtruderForSend(const cura::ExtruderTrain&)
    {
    }

    void CRSliceFromScene::setLayerForSend(const cura::LayerIndex&)
    {
    }

    bool CRSliceFromScene::hasSlice() const
    {
        return !m_sliced;
    }

    bool CRSliceFromScene::isSequential() const
    {
        return true;
    }

    void CRSliceFromScene::sendGCodePrefix(const std::string&) const
    {
    }

    void CRSliceFromScene::sendSliceUUID(const std::string& slice_uuid) const
    {
    }

    void CRSliceFromScene::sendPrintTimeMaterialEstimates() const
    {
    }

    void CRSliceFromScene::sendProgress(const float& progress) const
    {
    }

    void CRSliceFromScene::sliceNext()
    {
        //cura::FffProcessor::getInstance()->time_keeper.restart();
        //
        //// Count the number of mesh groups to slice for.
        //size_t num_mesh_groups = 1;
        //cura::Slice slice(num_mesh_groups);
        //
        //cura::Application::getInstance().current_slice = &slice;
        //
        //size_t mesh_group_index = 0;
        //Settings* last_settings = &slice.scene.settings;
        //
        //slice.scene.extruders.reserve(arguments.size() >> 1); // Allocate enough memory to prevent moves.
        //slice.scene.extruders.emplace_back(0, &slice.scene.settings); // Always have one extruder.
        //ExtruderTrain* last_extruder = &slice.scene.extruders[0];
        //
        //for (size_t argument_index = 2; argument_index < arguments.size(); argument_index++)
        //{
        //    std::string argument = arguments[argument_index];
        //    if (argument[0] == '-') // Starts with "-".
        //    {
        //        if (argument[1] == '-') // Starts with "--".
        //        {
        //            if (argument.find("--next") == 0) // Starts with "--next".
        //            {
        //                try
        //                {
        //                    LOGI("Loaded from disk in {}", FffProcessor::getInstance()->time_keeper.restart());
        //
        //                    mesh_group_index++;
        //                    FffProcessor::getInstance()->time_keeper.restart();
        //                    last_settings = &slice.scene.mesh_groups[mesh_group_index].settings;
        //                }
        //                catch (...)
        //                {
        //                    // Catch all exceptions.
        //                    // This prevents the "something went wrong" dialogue on Windows to pop up on a thrown exception.
        //                    // Only ClipperLib currently throws exceptions. And only in the case that it makes an internal error.
        //                    LOGE("Unknown exception!");
        //                    exit(1);
        //                }
        //            }
        //            else
        //            {
        //                LOGE("Unknown option: {}", argument);
        //            }
        //        }
        //        else // Starts with "-" but not with "--".
        //        {
        //            argument = arguments[argument_index];
        //            switch (argument[1])
        //            {
        //            case 'v':
        //            {
        //                LOGLEVEL(1);
        //                break;
        //            }
        //            case 'm':
        //            {
        //                int threads = stoi(argument.substr(2));
        //                Application::getInstance().startThreadPool(threads);
        //                break;
        //            }
        //            case 'p':
        //            {
        //                // enableProgressLogging(); FIXME: how to handle progress logging? Is this still relevant?
        //                break;
        //            }
        //            case 'j':
        //            {
        //                argument_index++;
        //                if (argument_index >= arguments.size())
        //                {
        //                    LOGE("Missing JSON file with -j argument.");
        //                    exit(1);
        //                }
        //                argument = arguments[argument_index];
        //                if (loadJSON(argument, *last_settings))
        //                {
        //                    LOGE("Failed to load JSON file: %s", argument.c_str());
        //                    exit(1);
        //                }
        //
        //                // If this was the global stack, create extruders for the machine_extruder_count setting.
        //                if (last_settings == &slice.scene.settings)
        //                {
        //                    const size_t extruder_count = slice.scene.settings.get<size_t>("machine_extruder_count");
        //                    while (slice.scene.extruders.size() < extruder_count)
        //                    {
        //                        slice.scene.extruders.emplace_back(slice.scene.extruders.size(), &slice.scene.settings);
        //                    }
        //                }
        //                // If this was an extruder stack, make sure that the extruder_nr setting is correct.
        //                if (last_settings == &last_extruder->settings)
        //                {
        //                    last_extruder->settings.add("extruder_nr", std::to_string(last_extruder->extruder_nr));
        //                }
        //                break;
        //            }
        //            case 'e':
        //            {
        //                size_t extruder_nr = stoul(argument.substr(2));
        //                while (slice.scene.extruders.size() <= extruder_nr) // Make sure we have enough extruders up to the extruder_nr that the user wanted.
        //                {
        //                    slice.scene.extruders.emplace_back(extruder_nr, &slice.scene.settings);
        //                }
        //                last_settings = &slice.scene.extruders[extruder_nr].settings;
        //                last_settings->add("extruder_nr", argument.substr(2));
        //                last_extruder = &slice.scene.extruders[extruder_nr];
        //                break;
        //            }
        //            case 'l':
        //            {
        //                argument_index++;
        //                if (argument_index >= arguments.size())
        //                {
        //                    LOGE("Missing model file with -l argument.");
        //                    exit(1);
        //                }
        //                argument = arguments[argument_index];
        //
        //                const FMatrix4x3 transformation = last_settings->get<FMatrix4x3>("mesh_rotation_matrix"); // The transformation applied to the model when loaded.
        //
        //                if (!loadMeshIntoMeshGroup(&slice.scene.mesh_groups[mesh_group_index], argument.c_str(), transformation, last_extruder->settings))
        //                {
        //                    LOGE("Failed to load model: {}. (error number {})", argument, errno);
        //                    exit(1);
        //                }
        //                else
        //                {
        //                    last_settings = &slice.scene.mesh_groups[mesh_group_index].meshes.back().settings;
        //                }
        //                break;
        //            }
        //            case 'o':
        //            {
        //                argument_index++;
        //                if (argument_index >= arguments.size())
        //                {
        //                    LOGE("Missing output file with -o argument.");
        //                    exit(1);
        //                }
        //                argument = arguments[argument_index];
        //                if (!FffProcessor::getInstance()->setTargetFile(argument.c_str()))
        //                {
        //                    LOGE("Failed to open {} for output.", argument.c_str());
        //                    exit(1);
        //                }
        //                break;
        //            }
        //            case 'g':
        //            {
        //                last_settings = &slice.scene.mesh_groups[mesh_group_index].settings;
        //                break;
        //            }
        //            /* ... falls through ... */
        //            case 's':
        //            {
        //                // Parse the given setting and store it.
        //                argument_index++;
        //                if (argument_index >= arguments.size())
        //                {
        //                    LOGE("Missing setting name and value with -s argument.");
        //                    exit(1);
        //                }
        //                argument = arguments[argument_index];
        //                const size_t value_position = argument.find("=");
        //                std::string key = argument.substr(0, value_position);
        //                if (value_position == std::string::npos)
        //                {
        //                    LOGE("Missing value in setting argument: -s {}", argument);
        //                    exit(1);
        //                }
        //                std::string value = argument.substr(value_position + 1);
        //                last_settings->add(key, value);
        //                break;
        //            }
        //            default:
        //            {
        //                LOGE("Unknown option: -{}", argument[1]);
        //                Application::getInstance().printHelp();
        //                exit(1);
        //                break;
        //            }
        //            }
        //        }
        //    }
        //}
        //
        //arguments.clear(); // We've processed all arguments now.
        //
        //
        //slice.scene.mesh_groups[mesh_group_index].finalize();
        //// Start slicing.
        //slice.compute();
        //// Finalize the processor. This adds the end g-code and reports statistics.
        //cura::FffProcessor::getInstance()->finalize();

        m_sliced = true;
    }

} // namespace crslice