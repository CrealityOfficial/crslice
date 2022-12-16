#ifndef CURA_LOADSETTINGS_1605061398960_H
#define CURA_LOADSETTINGS_1605061398960_H
#include <string>
#include "cxutil/slicer/ExtruderTrain.h"
#include "cxutil/settings/Settings.h"
#include <vector>
#include<unordered_map>

namespace cxutil
{
#define MAX_JSON_CONTENT_SIZE 4096
       /*
    * \brief Load a JSON file and store the settings inside it.
    * \param json_filename The location of the JSON file to load settings from.
    * \param settings The settings storage to store the settings in.
    * \return Error code. If it's 0, the file was successfully loaded. If it's
    * 1, the file could not be opened. If it's 2, there was a syntax error in
    * the file.
    */
    int loadSettingsJSON(const std::string& json_filename, Settings* settings,
        std::vector<ExtruderTrain>& extruders, Settings* extruderParent);

	int loadSettingsFromBuffer(char* buffer, Settings* settings,
		std::vector<ExtruderTrain>& extruders, Settings* extruderParent);

    void modifySettings(Settings* settings, std::vector<std::pair<std::string, std::string>>& kvs);


    bool loadProfileJSON(const std::string& json_filename, const std::unordered_map<std::string,std::string> defaultProfileCategoryMap,std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& proflieKVS);
}

#endif // CURA_LOADSETTINGS_1605061398960_H