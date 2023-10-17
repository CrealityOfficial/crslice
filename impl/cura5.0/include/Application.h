//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef APPLICATION_H
#define APPLICATION_H

#include <cstddef> //For size_t.
#include <cassert>
#include <string>
#include <vector>
#include <memory>

#include "communication/scenefactory.h"
#include "communication/slicecontext.h"

#include "FffGcodeWriter.h"
#include "FffPolygonGenerator.h"
#include "Scene.h"
#include "utils/ThreadPool.h"
#include "progress/Progress.h"
#include "crslice/header.h"

#include "ccglobal/tracer.h"

namespace cura52
{
    /*!
     * A singleton class that serves as the starting point for all slicing.
     *
     * The application provides a starting point for the slicing engine. It
     * maintains communication with other applications and uses that to schedule
     * slices.
     */
    class Application : public SliceContext
    {
    public:
        /*!
         * \brief Constructs a new Application instance.
         *
         * You cannot call this because this goes via the getInstance() function.
         */
        Application(ccglobal::Tracer* tracer = nullptr);

        /*!
         * \brief Destroys the Application instance.
         *
         * This destroys the Communication instance along with it.
         */
        virtual ~Application();

        Application* application = nullptr;

        /*!
         * The gcode writer, which generates paths in layer plans in a buffer, which converts these paths into gcode commands.
         */
        FffGcodeWriter gcode_writer;

        /*!
         * The polygon generator, which slices the models and generates all polygons to be printed and areas to be filled.
         */
        FffPolygonGenerator polygon_generator;

        Progress progressor;

        std::string tempDirectory;
        ccglobal::Tracer* tracer = nullptr;
        crslice::FDMDebugger* fDebugger = nullptr;
        /*
         * \brief The slice that is currently ongoing.
         *
         * If no slice has started yet, this will be a nullptr.
         */
        Scene* scene = nullptr;

        SliceResult runSceneFactory(SceneFactory* factory);
        /*!
         * \brief Start the global thread pool.
         *
         * If `nworkers` <= 0 and there is no pre-existing thread pool, a thread
         * pool with hardware_concurrency() workers is initialized.
         * The thread pool is restarted when the number of thread differs from
         * previous invocations.
         *
         * \param nworkers The number of workers (including the main thread) that are ran.
         */
        void startThreadPool(int nworkers = 0);

        void sendProgress(float r);

        void compute();
    protected:
        int extruderCount() override;
        const std::vector<ExtruderTrain>& extruders() const override;
        std::vector<ExtruderTrain>& extruders() override;
        const Settings& sceneSettings() override;
        bool isCenterZero() override;
        std::string polygonFile() override;

        MeshGroup* currentGroup() override;
        bool isFirstGroup() override;

        ThreadPool* pool() override;
        bool checkInterrupt(const std::string& message = "") override;
        void tick(const std::string& tag) override;
        void message(const char* msg) override;

        void messageProgress(Progress::Stage stage, int progress_in_stage, int progress_in_stage_max) override;
        void messageProgressStage(Progress::Stage stage) override;

        void setResult(const SliceResult& result) override;
    private:
        bool m_error;

        /*!
         * \brief ThreadPool with lifetime tied to Application
         */
        std::unique_ptr<ThreadPool> thread_pool;

        SliceResult m_result;
    };

} //Cura namespace.

#if 1   // USE_INTERRUPT
#define INTERRUPT_RETURN(x) 	if (application->checkInterrupt(x)) return
#define INTERRUPT_RETURN_FALSE(x) 	if (application->checkInterrupt(x)) return false
#define INTERRUPT_BREAK(x) 	if (application->checkInterrupt(x)) break
#else
#define INTERRUPT_RETURN(x) 	(void)0
#define INTERRUPT_RETURN_FALSE(x)  (void)0
#define INTERRUPT_BREAK(x) (void)0
#endif

#if 1
#define CALLTICK(x) application->tick(x) 
#else
#define CALLTICK(x) (void)0
#endif

#define SAFE_MESSAGE(...) \
	if(application->tracer) application->tracer->formatMessage(__VA_ARGS__)

#endif //APPLICATION_H