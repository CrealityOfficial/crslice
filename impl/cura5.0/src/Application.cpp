// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include "Application.h"

#include <chrono>
#include <memory>
#include <string>

#include "ccglobal/log.h"

#include "Weaver.h"
#include "Wireframe2gcode.h"
#include "sliceDataStorage.h"
#include "utils/ThreadPool.h"
#include "utils/string.h" //For stringcasecompare.

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

        application = this;
    }

    Application::~Application()
    {
    }

    void Application::sendProgress(float r)
    {
        if(tracer)
            tracer->progress(r);
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

    void Application::tick(const std::string& tag)
    {
        if (fDebugger)
            fDebugger->tick(tag);
    }

    void Application::message(const char* msg)
    {
        if (tracer)
            tracer->message(msg);
    }

    SliceResult Application::runSceneFactory(SceneFactory* factory)
    {
        if (!factory)
            return m_result;

        CALLTICK("slice 0");
        progressor.init();
        startThreadPool(); // Start the thread pool
        if (factory->hasSlice())
        {
            std::shared_ptr<Scene> slice(factory->createSlice());
            if (slice)
            {
                this->scene = slice.get();
                this->scene->application = this;

                gcode_writer.setTargetFile(scene->gcodeFile.c_str());

                scene->finalize();
                compute();
                // Finalize the processor. This adds the end g-code and reports statistics.
                gcode_writer.finalize();
                gcode_writer.closeGcodeWriterFile();
            }
        }
		CALLTICK("slice 1");

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
                SAFE_MESSAGE("{0}");

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
                    SliceDataStorage storage(application);
                    if (!polygon_generator.generateAreas(storage, &*mesh_group))
                    {
                        return;
                    }

                    progressor.messageProgressStage(Progress::Stage::EXPORT);

                    CALLTICK("writeGCode 0");
                    gcode_writer.writeGCode(storage);
                    CALLTICK("writeGCode 1");
                }

                progressor.messageProgress(Progress::Stage::FINISH, 1, 1); // 100% on this meshgroup
                LOGI("Total time elapsed { %f }s.\n", time_keeper_total.restart());
            }

            ++index;
        }
    }
} // namespace cura52