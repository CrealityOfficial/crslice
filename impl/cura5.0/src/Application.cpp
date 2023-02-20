// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include "Application.h"

#include <chrono>
#include <memory>
#include <string>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "ccglobal/log.h"

#include "FffProcessor.h"
#include "communication/ArcusCommunication.h" //To connect via Arcus to the front-end.
#include "communication/CommandLine.h" //To use the command line to slice stuff.
#include "progress/Progress.h"
#include "utils/ThreadPool.h"
#include "utils/string.h" //For stringcasecompare.

namespace cura52
{

Application::Application()
{
}

Application::~Application()
{
    releaseCommulication();
    delete thread_pool;
}

Application& Application::getInstance()
{
    static Application instance; // Constructs using the default constructor.
    return instance;
}

void Application::printCall(int argc, const char** argv) const
{
    LOGE("Command called: %s", *argv);
}

void Application::printHelp() const
{
#if 0
    fmt::print("\n");
    fmt::print("usage:\n");
    fmt::print("CuraEngine help\n");
    fmt::print("\tShow this help message\n");
    fmt::print("\n");
#ifdef ARCUS
    fmt::print("CuraEngine connect <host>[:<port>] [-j <settings.def.json>]\n");
    fmt::print("  --connect <host>[:<port>]\n\tConnect to <host> via a command socket, \n\tinstead of passing information via the command line\n");
    fmt::print("  -v\n\tIncrease the verbose level (show log messages).\n");
#ifdef _OPENMP
    fmt::print("  -m<thread_count>\n\tSet the desired number of threads. Supports only a single digit.\n");
#endif // _OPENMP
    fmt::print("\n");
#endif // ARCUS
    fmt::print("CuraEngine slice [-v] [-p] [-j <settings.json>] [-s <settingkey>=<value>] [-g] [-e<extruder_nr>] [-o <output.gcode>] [-l <model.stl>] [--next]\n");
    fmt::print("  -v\n\tIncrease the verbose level (show log messages).\n");
#ifdef _OPENMP
    fmt::print("  -m<thread_count>\n\tSet the desired number of threads.\n");
#endif // _OPENMP
    fmt::print("  -p\n\tLog progress information.\n");
    fmt::print("  -j\n\tLoad settings.def.json file to register all settings and their defaults.\n");
    fmt::print("  -s <setting>=<value>\n\tSet a setting to a value for the last supplied object, \n\textruder train, or general settings.\n");
    fmt::print("  -l <model_file>\n\tLoad an STL model. \n");
    fmt::print("  -g\n\tSwitch setting focus to the current mesh group only.\n\tUsed for one-at-a-time printing.\n");
    fmt::print("  -e<extruder_nr>\n\tSwitch setting focus to the extruder train with the given number.\n");
    fmt::print("  --next\n\tGenerate gcode for the previously supplied mesh group and append that to \n\tthe gcode of further models for one-at-a-time printing.\n");
    fmt::print("  -o <output_file>\n\tSpecify a file to which to write the generated gcode.\n");
    fmt::print("\n");
    fmt::print("The settings are appended to the last supplied object:\n");
    fmt::print("CuraEngine slice [general settings] \n\t-g [current group settings] \n\t-e0 [extruder train 0 settings] \n\t-l obj_inheriting_from_last_extruder_train.stl [object settings] \n\t--next [next group settings]\n\t... etc.\n");
    fmt::print("\n");
    fmt::print("In order to load machine definitions from custom locations, you need to create the environment variable CURA_ENGINE_SEARCH_PATH, which should contain all search paths delimited by a (semi-)colon.\n");
    fmt::print("\n");
#endif
}

void Application::printLicense() const
{
#if 0
    fmt::print("\n");
    fmt::print("Cura_SteamEngine version %s\n", CURA_ENGINE_VERSION);
    fmt::print("Copyright (C) 2022 Ultimaker\n");
    fmt::print("\n");
    fmt::print("This program is free software: you can redistribute it and/or modify\n");
    fmt::print("it under the terms of the GNU Affero General Public License as published by\n");
    fmt::print("the Free Software Foundation, either version 3 of the License, or\n");
    fmt::print("(at your option) any later version.\n");
    fmt::print("\n");
    fmt::print("This program is distributed in the hope that it will be useful,\n");
    fmt::print("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    fmt::print("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    fmt::print("GNU Affero General Public License for more details.\n");
    fmt::print("\n");
    fmt::print("You should have received a copy of the GNU Affero General Public License\n");
    fmt::print("along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");
#endif
}

void Application::slice(ccglobal::Tracer* tracer)
{
    communication = new CommandLine(m_args, tracer);
    communication->application = this;
}

void Application::setSliceCommunication(Communication *ptr)
{
    communication = ptr;
}

void Application::run(int argc, const char** argv, ccglobal::Tracer* tracer)
{
    for (size_t argument_index = 0; argument_index < argc; argument_index++)
    {
        m_args.emplace_back(argv[argument_index]);
    }


    printLicense();
    Progress::init();

    if (argc < 2)
    {
        printHelp();
        exit(1);
    }

    if (stringcasecompare(argv[1], "slice") == 0)
    {
        slice(tracer);
    }
    else if (stringcasecompare(argv[1], "help") == 0)
    {
        printHelp();
    }
    else
    {
        LOGE("Unknown command: {}", argv[1]);
        printCall(argc, argv);
        printHelp();
        exit(1);
    }

    if (! communication)
    {
        // No communication channel is open any more, so either:
        //- communication failed to connect, or
        //- the engine was called with an unknown command or a command that doesn't connect (like "help").
        // In either case, we don't want to slice.
        exit(0);
    }
    startThreadPool(); // Start the thread pool
    while (communication->hasSlice())
    {
        communication->sliceNext();
    }
}

void Application::runCommulication(Communication* _communication)
{
    if (!_communication)
        return;

    releaseCommulication();
    communication = _communication;
    communication->application = this;

    startThreadPool(); // Start the thread pool
    while (communication->hasSlice())
    {
        communication->sliceNext();
    }
}

void Application::releaseCommulication()
{
    if (communication)
    {
        delete communication;
        communication = nullptr;
    }
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