// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include "Application.h"

#include "magic/Weaver.h"
#include "magic/Wireframe2gcode.h"

#include "sliceDataStorage.h"
#include <boost/filesystem.hpp>

#include "communication/scenefactory.h"

namespace cura52
{
    Application::Application(ccglobal::Tracer* _tracer)
        : tracer(_tracer)
        , m_error(false)
    {
        assert(tracer);

        progressor.tracer = tracer;
        gcode_writer.application = this;
        polygon_generator.application = this;
        gcode_writer.gcode.application = this;
        gcode_writer.layer_plan_buffer.application = this;
        gcode_writer.layer_plan_buffer.preheat_config.application = this;
    }

    Application::~Application()
    {
    }

    ThreadPool* Application::pool()
    {
        return thread_pool.get();
    }

    bool Application::checkInterrupt(const std::string& message)
    {
        if (m_error)
            return true;

        if (!tracer)
            return false;

        bool rupt = tracer->interrupt();
        if (rupt)
        {
            m_error = true;
            std::string msg = "slice interrupt for -->" + message;
            tracer->failed(msg.c_str());
        }
        return rupt;
    }

    void Application::setFailed()
    {
        m_error = true;
    }

    void Application::tick(const std::string& tag)
    {
        if (scene && scene->fDebugger)
            scene->fDebugger->tick(tag);
    }

    void Application::message(const char* msg)
    {
        if (tracer)
            tracer->message(msg);
    }

    ccglobal::Tracer* Application::getTracer()
    {
        return tracer;
    }

    crslice::FDMDebugger* Application::debugger()
    {
        return scene->fDebugger;
    }

    Cache* Application::cache()
    {
        return m_cache.get();
    }

    SliceResult Application::runSceneFactory(SceneFactory* factory)
    {
        if (!factory)
            return m_result;

        progressor.init();
        startThreadPool(); // Start the thread pool
        if (factory->hasSlice())
        {
            scene.reset(factory->createSlice());
            if (scene)
            {
                tick("slice 0");
                initCache();

                gcode_writer.setTargetFile(scene->gcodeFile.c_str());

                compute();
                // Finalize the processor. This adds the end g-code and reports statistics.
                gcode_writer.finalize();
                gcode_writer.closeGcodeWriterFile();

                tick("slice 1");

                if (tracer)
                    tracer->variadicFormatMessage(10);
            }
        }

        return m_result;
    }

    int Application::extruderCount()
    {
        return (int)scene->extruders.size();
    }

    const std::vector<ExtruderTrain>& Application::extruders() const 
    {
        return scene->extruders;
    }

    std::vector<ExtruderTrain>& Application::extruders()
    {
        return scene->extruders;
    }

    const Settings& Application::sceneSettings()
    {
        return scene->settings;
    }

    bool Application::isCenterZero()
    {
        return scene->machine_center_is_zero;
    }

    std::string Application::polygonFile()
    {
        return scene->ploygonFile;
    }

    MeshGroup* Application::currentGroup()
    {
        return &*scene->current_mesh_group;
    }

    bool Application::isFirstGroup()
    {
        return scene->current_mesh_group == scene->mesh_groups.begin();
    }

    int Application::groupCount()
    {
        return (int)scene->mesh_groups.size();
    }

    FPoint3 Application::groupOffset()
    {
        return scene->mesh_groups.back().m_offset;
    }

    void Application::setResult(const SliceResult& result)
    {
        m_result = result;
    }

    void Application::messageProgress(Progress::Stage stage, int progress_in_stage, int progress_in_stage_max)
    {
        progressor.messageProgress(stage, progress_in_stage, progress_in_stage_max);
    }

    void Application::messageProgressStage(Progress::Stage stage)
    {
        progressor.messageProgressStage(stage);
    }

    void Application::startThreadPool(int nworkers)
    {
        int nthreads = 1;
        if (nworkers <= 0)
        {
            if (thread_pool)
            {
                return; // Keep the previous ThreadPool
            }
            nthreads = (int)std::thread::hardware_concurrency() - 1;
        }
        else
        {
            nthreads = nworkers - 1; // Minus one for the main thread
        }
        if (thread_pool && (int)thread_pool->thread_count() == nthreads)
        {
            return; // Keep the previous ThreadPool
        }
          thread_pool.reset(new ThreadPool((size_t)nthreads));
    }

    void Application::compute()
    {
        //build setting
        size_t numExtruder = scene->extruders.size();
        for (size_t i = 0; i < numExtruder; ++i)
        {
            ExtruderTrain& train = scene->extruders[i];
            train.extruder_nr = i;
            train.settings.application = this;
            train.settings.add("extruder_nr", std::to_string(i));
        }

        for (MeshGroup& meshGroup : scene->mesh_groups)
        {
            meshGroup.settings.application = this;
            for (Mesh& mesh : meshGroup.meshes)
            {
                mesh.settings.application = this;

                if (mesh.settings.has("extruder_nr"))
                {
                    size_t i = mesh.settings.get<size_t>("extruder_nr");
                    if (i <= numExtruder)
                    {
                        mesh.settings.setParent(&scene->extruders[i].settings);
                        continue;
                    }
                }

                mesh.settings.setParent(&scene->extruders[0].settings);
            }
        }

        for (MeshGroup& meshGroup : scene->mesh_groups)
            meshGroup.finalize();


        // slice
        int index = 0;
        for (std::vector<MeshGroup>::iterator mesh_group = scene->mesh_groups.begin(); mesh_group != scene->mesh_groups.end(); mesh_group++)
        {
            scene->current_mesh_group = mesh_group;
            for (ExtruderTrain& extruder : scene->extruders)
            {
                extruder.settings.setParent(&scene->current_mesh_group->settings);
            }

            {
                progressor.restartTime();
                if (tracer)
                    tracer->variadicFormatMessage(0);

                TimeKeeper time_keeper_total;

                bool empty = true;
                for (Mesh& mesh : mesh_group->meshes)
                {
                    if (!mesh.settings.get<bool>("infill_mesh") && !mesh.settings.get<bool>("anti_overhang_mesh"))
                    {
                        empty = false;
                        break;
                    }
                }
                if (empty)
                {
                    progressor.messageProgress(Progress::Stage::FINISH, 1, 1); // 100% on this meshgroup
                    LOGI("Total time elapsed { %f }s.", time_keeper_total.restart());
                    return;
                }

                if (mesh_group->settings.get<bool>("wireframe_enabled"))
                {
                    LOGI("Starting Neith Weaver...");

                    Weaver weaver;
                    weaver.application = this;

                    weaver.weave(&*mesh_group);

                    LOGI("Starting Neith Gcode generation...");
                    Wireframe2gcode gcoder(weaver, gcode_writer.gcode);
                    gcoder.writeGCode();
                    LOGI("Finished Neith Gcode generation...");
                }
                else // Normal operation (not wireframe).
                {
                    SliceDataStorage storage(this);
                    
                    if (!polygon_generator.generateAreas(storage, &*mesh_group))
                    {
                        return;
                    }
                    storage.m_Object_Exclude_FileName = scene.get()->m_Object_Exclude_FileName;
                    progressor.messageProgressStage(Progress::Stage::EXPORT);

                    tick("writeGCode 0");
                    gcode_writer.writeGCode(storage);
                    tick("writeGCode 1");
                }
				
                if (tracer)
                    tracer->variadicFormatMessage(9);

                progressor.messageProgress(Progress::Stage::FINISH, 1, 1); // 100% on this meshgroup
                LOGI("Total time elapsed { %f }s.\n", time_keeper_total.restart());
            }

            ++index;
        }
    }

    void Application::initCache()
    {
        initUseCache(false, "");
        if (scene.get())
        {
            const Settings& settings = scene->settings;
            if (settings.has("fdm_slice_debug") && settings.get<bool>("fdm_slice_debug"))
            {
                std::string path = settings.get<std::string>("fdm_slice_debug_path");
                if (boost::filesystem::exists(path))
                {
                    initUseCache(true, path);
                    m_cache.reset(new Cache(path, this));

                    startThreadPool(2);
                }
            }
        }
    }
} // namespace cura52