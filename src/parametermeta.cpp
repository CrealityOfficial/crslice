#include "crslice/base/parametermeta.h"
#include <boost/filesystem.hpp>

#include "jsonhelper.h"
#include "ccglobal/log.h"

namespace crslice
{
	ParameterMetas::ParameterMetas()
	{
	}

	ParameterMetas::~ParameterMetas()
	{
        clear();
	}

    ParameterMeta* ParameterMetas::find(const std::string& key)
    {
        MetasMapIter iter = mm.find(key);
        if (iter != mm.end())
            return iter->second;
        
        LOGE("ParameterMetas::find key error [%s].", key.c_str());
        return nullptr;
    }

	void ParameterMetas::initializeBase(const std::string& path)
	{
		std::string baseFile = path + "/base.json";

		rapidjson::Document baseDoc;
		if (!openJson(baseDoc, baseFile))
		{
			LOGE("ParameterMetas::initializeBase error. [%s] not contain base.json", path.c_str());
			return;
		}

        if (baseDoc.HasMember("subs") && baseDoc["subs"].IsArray())
        {
            const rapidjson::Value& value = baseDoc["subs"];
            for (rapidjson::Value::ConstValueIterator it = value.Begin(); it != value.End(); ++it)
            {
                std::string sub = it->GetString();
                std::string json = path + "/" + sub;

                rapidjson::Document subDoc;
                if (!openJson(subDoc, json))
                {
                    LOGE("ParameterMetas::initializeBase parse sub. [%s] error.", sub.c_str());
                    continue;
                }

                processSub(subDoc, mm);
            }
        }
        else {
            LOGE("ParameterMetas::initializeBase base.json no subs.");
        }
	}

	ParameterMetas* ParameterMetas::createInherits(const std::string& fileName)
	{
        std::string directory = boost::filesystem::path(fileName).parent_path().string();

        rapidjson::Document machineDoc;
        if (!openJson(machineDoc, fileName))
        {
            LOGE("ParameterMetas::createInherits error. [%s] not valid.", fileName.c_str());
            return nullptr;
        }

        ParameterMetas* newMetas = copy();
        if (machineDoc.HasMember("inherits") && machineDoc["inherits"].IsString())
        {
            std::string inherits = machineDoc["inherits"].GetString();
            processInherit(inherits, fileName, *newMetas);
        }
        return newMetas;
	}

    void ParameterMetas::clear()
    {
        for (MetasMapIter it = mm.begin();
            it != mm.end(); ++it)
            delete it->second;
        mm.clear();
    }

    ParameterMetas* ParameterMetas::copy()
    {
        ParameterMetas* nm = new ParameterMetas();
        for (MetasMapIter it = mm.begin();
            it != mm.end(); ++it)
            nm->mm.insert(MetasMap::value_type(it->first, new ParameterMeta(*it->second)));
        return nm;
    }

    void saveKeysJson(const std::vector<std::string>& keys, const std::string& fileName)
    {
        std::string content = createKeysContent(keys);

        saveJson(fileName, content);
    }

    void parseMetasMap(MetasMap& datas)
    {
#ifdef USE_BINARY_JSON
#include "base.json.h"

        rapidjson::Document baseDoc;
        baseDoc.Parse(base);
        if (baseDoc.HasParseError())
        {
            LOGE("ParameterMetas::initializeBase error. [%d] not contain base.json", (int)baseDoc.GetParseError());
            return;
        }

        if (baseDoc.HasMember("subs") && baseDoc["subs"].IsArray())
        {
            const rapidjson::Value& value = baseDoc["subs"];
            for (rapidjson::Value::ConstValueIterator it = value.Begin(); it != value.End(); ++it)
            {
                std::string sub = it->GetString();

                const char* subStr = nullptr;
                rapidjson::Document subDoc;
                subDoc.Parse(subStr);
                if (subDoc.HasParseError())
                {
                    LOGE("ParameterMetas::initializeBase parse sub. [%s] [%d] error.", sub.c_str(), (int)subDoc.GetParseError());
                    continue;
                }

                processSub(subDoc, datas);
            }
        }
        else {
            LOGE("ParameterMetas::initializeBase base.json no subs.");
        }
#else
        LOGE("ParameterMetas::Binary not support.");
#endif
    }
}