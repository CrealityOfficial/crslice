#include "getpath.h"
#include <iostream>
#include"ccglobal/log.h"
//#include "spdlog/cxlog_macro.h"

#ifdef _WIN32
#include <windows.h> // GetFullPathNameA
#else
#include <libgen.h> // dirname
#include <cstring> // std::strcpy
#endif

namespace cxutil
{
    std::string getPathName(const std::string& filePath)
    {
#ifdef _WIN32
        char buffer[MAX_PATH];  // This buffer will hold the complete path.
        char* file_name_start;  // This is the pointer to the start of the file name.
        DWORD path_size = GetFullPathNameA(filePath.c_str(), static_cast<DWORD>(MAX_PATH), buffer, &file_name_start);
        if (path_size == 0)
        {
            LOGE("Failed to get full path for [%s]" ,  filePath.c_str());
            return std::string("");
           // exit(1);
        }
        // Only take the directory part of
        DWORD dir_path_size = path_size - (path_size - (file_name_start - buffer));
        std::string folder_name{ buffer, dir_path_size };
#else
        char buffer[filePath.size()];
        std::strcpy(buffer, filePath.c_str()); // copy the string because dirname(.) changes the input string!!!
        std::string folder_name{ dirname(buffer) };
#endif
        return folder_name;
    }

    std::unordered_set<std::string> defaultSearchDirectories()
    {
        std::unordered_set<std::string> result;

        char* search_path_env = getenv("CURA_ENGINE_SEARCH_PATH");
        if (search_path_env)
        {
#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
            char delims[] = ":"; //Colon for Unix.
#else
            char delims[] = ";"; //Semicolon for Windows.
#endif
            char paths[128 * 1024]; //Maximum length of environment variable.
            strcpy(paths, search_path_env); //Necessary because strtok actually modifies the original string, and we don't want to modify the environment variable itself.
            char* path = strtok(paths, delims);
            while (path != nullptr)
            {
                result.emplace(path);
                path = strtok(nullptr, ";:,"); //Continue searching in last call to strtok.
            }
        }

        return result;
    }
}