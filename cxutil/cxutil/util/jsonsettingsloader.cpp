#include "cxutil/util/jsonsettingsloader.h"
#include <sstream>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h> //Loading JSON documents to get settings from them.
#include <rapidjson/filereadstream.h>

#include "cxutil/settings/Settings.h"
namespace cxutil
{
    void loadJSONValue(const rapidjson::Value& element, Settings& settings)
    {
        for (rapidjson::Value::ConstMemberIterator setting = element.MemberBegin(); setting != element.MemberEnd(); setting++)
        {
            const std::string name = setting->name.GetString();

            const rapidjson::Value& setting_object = setting->value;
            if (!setting_object.IsObject())
            {
                continue;
            }

            if (setting_object.HasMember("children"))
            {
               
                loadJSONValue(setting_object["children"], settings);
            }
            else //Only process leaf settings. We don't process categories or settings that have sub-settings.
            {
                if (!setting_object.HasMember("default_value"))
                {
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
                    continue;
                }
                settings.add(name, value_string);
            }
        }
    }

    void loadJSON(const rapidjson::Document& document, Settings& settings)
    {
        if (document.HasMember("inherits") && document["inherits"].IsString())
        {
            return;
        }

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

    }

	void loadJsonSetting(const char* fileName, Settings* settings)
	{
        FILE* file = fopen(fileName, "rb");
        if (!file || !settings)
        {
            return;
        }

        rapidjson::Document json_document;
        char read_buffer[4096];
        rapidjson::FileReadStream reader_stream(file, read_buffer, sizeof(read_buffer));
        json_document.ParseStream(reader_stream);
        fclose(file);
        if (json_document.HasParseError())
        {
            return;
        }

        return loadJSON(json_document, *settings);
	}
}