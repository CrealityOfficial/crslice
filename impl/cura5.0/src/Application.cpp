// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include "Application.h"

#include <chrono>
#include <memory>
#include <string>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include "communication/Communication.h"

#include "ccglobal/log.h"

#include "Slice.h"
#include "FffProcessor.h"
#include "progress/Progress.h"
#include "utils/ThreadPool.h"
#include "utils/string.h" //For stringcasecompare.

namespace cura52
{
    Application::Application(ccglobal::Tracer* _tracer)
        : tracer(_tracer)
        , m_error(false)
    {
        assert(tracer);

        progressor.application = this;
        processor.gcode_writer.application = this;
        processor.polygon_generator.application = this;
        processor.gcode_writer.gcode.application = this;
        processor.gcode_writer.layer_plan_buffer.application = this;
        processor.gcode_writer.layer_plan_buffer.preheat_config.application = this;

        application = this;
    }

    Application::~Application()
    {
        if (thread_pool)
        {
            delete thread_pool;
            thread_pool = nullptr;
        }
    }

    void Application::sendProgress(float r)
    {
        if(tracer)
            tracer->progress(r);
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

    void Application::runCommulication(Communication* _communication)
    {
        if (!_communication)
            return;

        CALLTICK("start");
        progressor.init();
        CALLTICK("progress init");
        startThreadPool(); // Start the thread pool
        CALLTICK("thread pool");
        if (_communication->hasSlice())
        {
            CALLTICK("start create slice");
            std::shared_ptr<Slice> slice(_communication->createSlice());
            CALLTICK("end create slice");
            if (slice)
            {
                processor.time_keeper.restart();

                current_slice = slice.get();
                current_slice->scene.application = this;
                current_slice->application = this;

                processor.setTargetFile(current_slice->gcodeFile.c_str());

                CALLTICK("finalize 0");
                current_slice->finalize();
                CALLTICK("finalize 1");
                CALLTICK("compute 0");
                current_slice->compute();
                CALLTICK("compute 1");
                // Finalize the processor. This adds the end g-code and reports statistics.
                processor.finalize();

                CALLTICK("finalize");
            }
        }
        CALLTICK("slice over");
    }

    void Application::startThreadPool(int nworkers)
    {
        size_t nthreads;
        if (nworkers <= 0)
        {
            if (thread_pool)
            {
                return; // Keep the previous ThreadPool
            }
            nthreads = std::thread::hardware_concurrency() - 1;
        }
        else
        {
            nthreads = nworkers - 1; // Minus one for the main thread
        }
        if (thread_pool && thread_pool->thread_count() == nthreads)
        {
            return; // Keep the previous ThreadPool
        }
        delete thread_pool;
        thread_pool = new ThreadPool(nthreads);
    }
} // namespace cura52