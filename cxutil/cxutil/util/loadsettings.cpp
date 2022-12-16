#include "loadsettings.h"

#include "cxutil/settings/Settings.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/document.h> 

#include "cxutil/util/getpath.h"
#include "cxutil/util/logoutput.h"

#include <fstream>
#include <sstream>

namespace cxutil
{
    /*
    * \brief Find a definition file in the search directories.
    * \param definition_id The ID of the definition to look for.
    * \param search_directories The directories to search in.
    * \return The first definition file that matches the definition ID.
    */
    const std::string findDefinitionFile(const std::string& definition_id, const std::unordered_set<std::string>& search_directories)
    {
        for (const std::string& search_directory : search_directories)
        {
            const std::string candidate = search_directory + std::string("/") + definition_id + std::string(".def.json");
            const std::ifstream ifile(candidate.c_str()); //Check whether the file exists and is readable by opening it.
            if (ifile)
            {
                return candidate;
            }
        }
        LOGE("Couldn't find definition file with ID: %s\n", definition_id.c_str());
        return std::string("");
    }

    /*
     * \brief Load an element containing a list of settings.
     * \param element The JSON element "settings" or "overrides" that contains
     * settings.
     * \param settings The settings storage to store the new settings in.
     */
    void loadJSONValue(const rapidjson::Value& element, Settings* settings)
    {
        for (rapidjson::Value::ConstMemberIterator setting = element.MemberBegin(); setting != element.MemberEnd(); setting++)
        {
            const std::string name = setting->name.GetString();

            const rapidjson::Value& setting_object = setting->value;
            if (!setting_object.IsObject())
            {
                LOGE("JSON setting %s is not an object!\n", name.c_str());
                continue;
            }
#if 1
            if (setting_object.HasMember("default_value"))
            {
                const rapidjson::Value& default_value = setting_object["default_value"];
                std::string value_string;
                if (default_value.IsString())
                {
                    value_string = default_value.GetString();
                }
                else if (default_value.IsTrue())
                {
                    value_string = "true";
                }
                else if (default_value.IsFalse())
                {
                    value_string = "false";
                }
                else if (default_value.IsNumber())
                {
                    std::ostringstream ss;
                    ss << default_value.GetDouble();
                    value_string = ss.str();
                }
                else
                {
                    LOGW("Unrecognized data type in JSON setting %s\n", name.c_str());
                    continue;
                }
               settings->add(name, value_string);
            }
            if (setting_object.HasMember("children"))
            {
                loadJSONValue(setting_object["children"], settings);
            }
#elif 
            if (setting_object.HasMember("children"))
            {
                loadJSONValue(setting_object["children"], settings);
            }
            else //Only process leaf settings. We don't process categories or settings that have sub-settings.
            {
                if (!setting_object.HasMember("default_value"))
                {
                    LOGW("JSON setting %s has no default_value!\n", name.c_str());
                    continue;
                }
                const rapidjson::Value& default_value = setting_object["default_value"];
                std::string value_string;
                if (default_value.IsString())
                {
                    value_string = default_value.GetString();
                }
                else if (default_value.IsTrue())
                {
                    value_string = "true";
                }
                else if (default_value.IsFalse())
                {
                    value_string = "false";
                }
                else if (default_value.IsNumber())
                {
                    std::ostringstream ss;
                    ss << default_value.GetDouble();
                    value_string = ss.str();
                }
                else
                {
                    LOGW("Unrecognized data type in JSON setting %s\n", name.c_str());
                    continue;
                }
                settings->add(name, value_string);
            }
#endif
        }

    }
    void loadCategoryValue(const rapidjson::Value& element, std::unordered_map<std::string, std::string>& kvs)
    {
        for (rapidjson::Value::ConstMemberIterator setting = element.MemberBegin(); setting != element.MemberEnd(); setting++)
        {
            const std::string name = setting->name.GetString();

            const rapidjson::Value& setting_object = setting->value;
            if (!setting_object.IsObject())
            {
                LOGE("JSON setting %s is not an object!\n", name.c_str());
                continue;
            }
            if (setting_object.HasMember("default_value"))
            {
                const rapidjson::Value& default_value = setting_object["default_value"];
                std::string value_string;
                if (default_value.IsString())
                {
                    value_string = default_value.GetString();
                }
                else if (default_value.IsTrue())
                {
                    value_string = "true";
                }
                else if (default_value.IsFalse())
                {
                    value_string = "false";
                }
                else if (default_value.IsNumber())
                {
                    std::ostringstream ss;
                    ss << default_value.GetDouble();
                    value_string = ss.str();
                }
                else
                {
                    LOGW("Unrecognized data type in JSON setting %s\n", name.c_str());
                    continue;
                }
                kvs.insert(std::make_pair(name, value_string));
                continue;
            }
            if (name.find("children") != std::string::npos)
            {
                loadCategoryValue(setting_object, kvs);
            }
        }

    }

    bool loadJSONValue(const rapidjson::Value& element, const std::unordered_map<std::string, std::string> defaultProfileCategoryMap,std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& proflieKVS)
    {
        for (rapidjson::Value::ConstMemberIterator setting = element.MemberBegin(); setting != element.MemberEnd(); setting++)
        {
            const std::string name = setting->name.GetString();
            const rapidjson::Value& setting_object = setting->value;
            if (!setting_object.IsObject())
            {
                LOGE("JSON setting %s is not an object!\n", name.c_str());
                continue;
            }
            if (defaultProfileCategoryMap.find(name) != defaultProfileCategoryMap.end())
            {
                std::unordered_map<std::string, std::string> kvs;
                loadCategoryValue(setting_object,kvs);
               
                proflieKVS.insert(std::make_pair(name, kvs));
            }
        }
        return true;
    }

    /*
    * \brief Load a JSON document and store the settings inside it.
    * \param document The JSON document to load the settings from.
    * \param settings The settings storage to store the settings in.
    * \return Error code. If it's 0, the document was successfully loaded. If
    * it's 1, some inheriting file could not be opened.
    */
    int loadJSON(const rapidjson::Document& document, const std::unordered_set<std::string>& search_directories, Settings* settings,
        std::vector<ExtruderTrain>& extruders, Settings* extruderParent)
    {
        //Inheritance from other JSON documents.
        if (document.HasMember("inherits") && document["inherits"].IsString())
        {
            std::string parent_file = findDefinitionFile(document["inherits"].GetString(), search_directories);
            if (parent_file == "")
            {
                LOGE("Inherited JSON file \"%s\" not found.\n", document["inherits"].GetString());
                return 1;
            }
            int error_code = loadSettingsJSON(parent_file, settings, extruders, extruderParent); //Head-recursively load the settings file that we inherit from.
            if (error_code)
            {
                return error_code;
            }
        }

        //Extruders defined from here, if any.
        //Note that this always puts the extruder settings in the slice of the current extruder. It doesn't keep the nested structure of the JSON files, if extruders would have their own sub-extruders.
        if (document.HasMember("metadata") && document["metadata"].IsObject())
        {
            const rapidjson::Value& metadata = document["metadata"];
            if (metadata.HasMember("machine_extruder_trains") && metadata["machine_extruder_trains"].IsObject())
            {
                const rapidjson::Value& extruder_trains = metadata["machine_extruder_trains"];
                for (rapidjson::Value::ConstMemberIterator extruder_train = extruder_trains.MemberBegin(); extruder_train != extruder_trains.MemberEnd(); extruder_train++)
                {
                    const int extruder_nr = atoi(extruder_train->name.GetString());
                    if (extruder_nr < 0)
                    {
                        continue;
                    }
                    while (extruders.size() <= static_cast<size_t>(extruder_nr))
                    {
                        extruders.emplace_back(extruders.size(), extruderParent);
                    }
                    const rapidjson::Value& extruder_id = extruder_train->value;
                    if (!extruder_id.IsString())
                    {
                        continue;
                    }
                    const std::string extruder_definition_id(extruder_id.GetString());
                    const std::string extruder_file = findDefinitionFile(extruder_definition_id, search_directories);
                    loadSettingsJSON(extruder_file, extruders[extruder_nr].settings, extruders, extruderParent);
                }
            }
        }

        if (document.HasMember("settings") && document["settings"].IsObject())
        {
            loadJSONValue(document["settings"], settings);
        }
        if (document.HasMember("overrides") && document["overrides"].IsObject())
        {
            loadJSONValue(document["overrides"], settings);
        }
        return 0;
    }

    bool loadJSON(const rapidjson::Document& document, const std::unordered_map<std::string, std::string> defaultProfileCategoryMap, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& proflieKVS)
    {
        if (document.HasMember("settings") && document["settings"].IsObject())
        {
            loadJSONValue(document["settings"], defaultProfileCategoryMap, proflieKVS);
        }
        return true;
    }

    int loadSettingsJSON(const std::string& json_filename, Settings* settings,
        std::vector<ExtruderTrain>& extruders, Settings* extruderParent)
    {
        if (!settings)
            return 1;

        FILE* file = fopen(json_filename.c_str(), "rb");
        if (!file)
        {
            LOGE("Couldn't open JSON file: %s\n", json_filename.c_str());
            return 1;
        }

        rapidjson::Document json_document;
        char read_buffer[4096];
        rapidjson::FileReadStream reader_stream(file, read_buffer, sizeof(read_buffer));
        json_document.ParseStream(reader_stream);
        fclose(file);
        if (json_document.HasParseError())
        {
            LOGE("Error parsing JSON (offset %u): %s\n", static_cast<unsigned int>(json_document.GetErrorOffset()), GetParseError_En(json_document.GetParseError()));
            return 2;
        }

        std::unordered_set<std::string> search_directories = cxutil::defaultSearchDirectories(); //For finding the inheriting JSON files.
        std::string directory = cxutil::getPathName(json_filename);
        search_directories.emplace(directory);

        return loadJSON(json_document, search_directories, settings, extruders, extruderParent);
    }

	int loadSettingsFromBuffer(char* buffer, Settings* settings,
		std::vector<ExtruderTrain>& extruders, Settings* extruderParent)
	{
		if (!settings || !buffer)
			return 1;

		rapidjson::Document json_document;
		json_document.Parse((char*)buffer);
		if (json_document.HasParseError())
		{
			LOGE("Error parsing JSON (offset %u): %s\n", static_cast<unsigned int>(json_document.GetErrorOffset()), GetParseError_En(json_document.GetParseError()));
			return 2;
		}

		std::unordered_set<std::string> search_directories = cxutil::defaultSearchDirectories(); //For finding the inheriting JSON files.

		return loadJSON(json_document, search_directories, settings, extruders, extruderParent);
	}

    void modifySettings(Settings* settings, std::vector<KeyValue>& kvs)
    {
        if (!settings)
            return;

        for (KeyValue& kv : kvs)
            settings->add(kv.first, kv.second);
    }

    bool loadProfileJSON(const std::string& json_filename, const std::unordered_map<std::string, std::string> defaultProfileCategoryMap, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& proflieKVS)
    {
        FILE* file = fopen(json_filename.c_str(), "rb");
        if (!file)
        {
            LOGE("Couldn't open JSON file: %s\n", json_filename.c_str());
            return false;
        }

        rapidjson::Document json_document;
        char read_buffer[MAX_JSON_CONTENT_SIZE];
        rapidjson::FileReadStream reader_stream(file, read_buffer, sizeof(read_buffer));
        json_document.ParseStream(reader_stream);
        fclose(file);
        if (json_document.HasParseError())
        {
            LOGE("Error parsing JSON (offset %u): %s\n", static_cast<unsigned int>(json_document.GetErrorOffset()), GetParseError_En(json_document.GetParseError()));
            return false;
        }
        return loadJSON(json_document, defaultProfileCategoryMap,proflieKVS);
    }
}