#include "print.h"
#include "logoutput.h"

namespace cxutil
{
    void printCall(int argc, char* argv[])
    {
        LOGE("Command called:\n");
        for (size_t argument_index = 0; argument_index < argc; argument_index++)
        {
            LOGE("%s ", argv[argument_index]);
        }
        LOGE("\n");
    }

    void printCall(const std::vector<std::string>& args)
    {
        LOGE("Command called:\n");
        for (size_t argument_index = 0; argument_index < args.size(); argument_index++)
        {
            LOGE("%s ", args[argument_index].c_str());
        }
        LOGE("\n");
    }

    void printHelp()
    {
        LOGV("\n");
        LOGV("usage:\n");
        LOGV("CuraEngine help\n");
        LOGV("\tShow this help message\n");
        LOGV("\n");
        LOGV("CuraEngine slice [-v] [-p] [-j <settings.json>] [-s <settingkey>=<value>] [-g] [-e<extruder_nr>] [-o <output.gcode>] [-l <model.stl>] [--next]\n");
        LOGV("  -v\n\tIncrease the verbose level (show log messages).\n");
#ifdef _OPENMP
        LOGV("  -m<thread_count>\n\tSet the desired number of threads.\n");
#endif // _OPENMP
        LOGV("  -p\n\tLog progress information.\n");
        LOGV("  -j\n\tLoad settings.def.json file to register all settings and their defaults.\n");
        LOGV("  -s <setting>=<value>\n\tSet a setting to a value for the last supplied object, \n\textruder train, or general settings.\n");
        LOGV("  -l <model_file>\n\tLoad an STL model. \n");
        LOGV("  -g\n\tSwitch setting focus to the current mesh group only.\n\tUsed for one-at-a-time printing.\n");
        LOGV("  -e<extruder_nr>\n\tSwitch setting focus to the extruder train with the given number.\n");
        LOGV("  --next\n\tGenerate gcode for the previously supplied mesh group and append that to \n\tthe gcode of further models for one-at-a-time printing.\n");
        LOGV("  -o <output_file>\n\tSpecify a file to which to write the generated gcode.\n");
        LOGV("\n");
        LOGV("The settings are appended to the last supplied object:\n");
        LOGV("CuraEngine slice [general settings] \n\t-g [current group settings] \n\t-e0 [extruder train 0 settings] \n\t-l obj_inheriting_from_last_extruder_train.stl [object settings] \n\t--next [next group settings]\n\t... etc.\n");
        LOGV("\n");
        LOGV("In order to load machine definitions from custom locations, you need to create the environment variable CURA_ENGINE_SEARCH_PATH, which should contain all search paths delimited by a (semi-)colon.\n");
        LOGV("\n");
    }

}
