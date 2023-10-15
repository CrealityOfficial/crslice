//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef APPLICATION_H
#define APPLICATION_H

#include "utils/NoCopy.h"
#include <cstddef> //For size_t.
#include <cassert>
#include <string>
#include <vector>

#include "FffGcodeWriter.h"
#include "FffPolygonGenerator.h"
#include "Scene.h"
#include "progress/Progress.h"
#include "crslice/header.h"

#include "ccglobal/tracer.h"

namespace cura52
{
    class Communication;
    class ThreadPool;

    struct SliceResult
    {
        unsigned long int print_time; // Ô¤¹À´òÓ¡ºÄÊ±£¬µ¥Î»£ºÃë
        double filament_len; // Ô¤¹À²ÄÁÏÏûºÄ£¬µ¥Î»£ºÃ×
        double filament_volume; // Ô¤¹À²ÄÁÏÖØÁ¿£¬µ¥Î»£ºg
        unsigned long int layer_count;  // ÇÐÆ¬²ãÊý
        double x;   // ÇÐÆ¬x³ß´ç
        double y;   // ÇÐÆ¬y³ß´ç
        double z;   // ÇÐÆ¬z³ß´ç

        SliceResult()
        {
            print_time = 0;
            filament_len = 0.0f;
            filament_volume = 0.0f;
            layer_count = 0;
            x = 0.0f;
            y = 0.0f;
            z = 0.0f;
        }
    };

    /*!
     * A singleton class that serves as the starting point for all slicing.
     *
     * The application provides a starting point for the slicing engine. It
     * maintains communication with other applications and uses that to schedule
     * slices.
     */
    class Application
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
        ~Application();

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
        /*!
         * \brief ThreadPool with lifetime tied to Application
         */
        ThreadPool* thread_pool = nullptr;

        void runCommulication(Communication* communication);
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
        bool checkInterrupt(const std::string& message = "");
        void tick(const std::string& tag);

        void compute();

        SliceResult sliceResult;
    private:
        bool m_error;
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