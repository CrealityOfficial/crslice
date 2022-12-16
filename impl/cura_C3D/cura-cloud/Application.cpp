//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifdef _OPENMP
    #include <omp.h> // omp_get_num_threads
#endif // _OPENMP
#include <string>
#include "Application.h"
#include "FffProcessor.h"
#include "communication/CommandLine.h" //To use the command line to slice stuff.
#include "progress/Progress.h"

#include "cxutil/util/print.h"
#include "cxutil/util/string.h" //For stringcasecompare.
#include "cxutil/util/logoutput.h"


namespace cxutil
{

    Application::Application()
        : communication(nullptr)
        , current_slice(0)
        , argc(0)
        , argv(nullptr)
    {
    }

    Application::~Application()
    {
        delete communication;
    }

    void Application::slice()
    {
        std::vector<std::string> arguments;
        for (size_t argument_index = 0; argument_index < argc; argument_index++)
        {
            arguments.emplace_back(argv[argument_index]);
        }

        communication = new CommandLine(arguments);
    }

    void Application::run(const size_t argc, char** argv)
    {
        this->argc = argc;
        this->argv = argv;

        if (argc < 2)
        {
            printHelp();
            exit(1);
        }

#pragma omp parallel
        {
#pragma omp master
            {
#ifdef _OPENMP
                LOGI("OpenMP multithreading enabled, likely number of threads to be used: %u\n", omp_get_num_threads());
#else
                LOGI("OpenMP multithreading disabled\n");
#endif
            }
        }

        if (cxutil::stringcasecompare(argv[1], "slice") == 0)
        {
            slice();
        }
        else if (cxutil::stringcasecompare(argv[1], "help") == 0)
        {
            printHelp();
        }
        else
        {
            //LOGE("Unknown command: %s\n", argv[1]);
            printCall(argc, argv);
            printHelp();
            exit(1);
        }

        if (!communication)
        {
            exit(0);
        }

        while (communication->hasSlice())
        {
            communication->sliceNext();
        }
    }

} //Cura namespace.