#ifndef CRCOMMON_PARAMETERMETA_1690769853657_H
#define CRCOMMON_PARAMETERMETA_1690769853657_H
#include "crslice/interface.h"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#define META_LABEL "label"
#define META_DESCRIPTION "description"
#define META_TYPE "type"
#define META_DEFAULT_VALUE "default_value"
#define META_VALUE "value"
#define META_ENABLED "enabled"

namespace crslice
{
	struct ParameterMeta
	{
		std::string name;
		std::string label;
		std::string description;
		std::string type;
		std::string default_value;
		std::string value;
		std::string enabled;
		
		std::string unit;
		std::string minimum_value;
		std::string maximum_value;
		std::string minimum_value_warning;
		std::string maximum_value_warning;

		std::unordered_map<std::string, std::string> options;

		ParameterMeta()
		{

		}

		ParameterMeta(const ParameterMeta& meta)
		{
			name = meta.name;
			label = meta.label;
			description = meta.description;
			type = meta.type;
			default_value = meta.default_value;
			value = meta.value;
			enabled = meta.enabled;

			unit = meta.unit;
			minimum_value = meta.minimum_value;
			maximum_value = meta.maximum_value;
			minimum_value_warning = meta.minimum_value_warning;
			maximum_value_warning = meta.maximum_value_warning;

			options = meta.options;
		}
	};

	typedef std::unordered_map<std::string, std::string>::value_type OptionValue;
	typedef std::unordered_map<std::string, ParameterMeta*> MetasMap;
	typedef MetasMap::iterator MetasMapIter;

	class CRSLICE_API ParameterMetas
	{
	public:
		ParameterMetas();
		~ParameterMetas();

		ParameterMeta* find(const std::string& key);

		// used for base
		void initializeBase(const std::string& path);
		ParameterMetas* createInherits(const std::string& fileName);
	protected:
		void clear();
		ParameterMetas* copy();
	public:
		MetasMap mm;
	};

	CRSLICE_API void saveKeysJson(const std::vector<std::string>& keys, const std::string& fileName);

	CRSLICE_API void parseMetasMap(MetasMap& datas);   //from binary

	enum class MetaGroup {
		emg_machine,
		emg_extruder,
		emg_material,
		emg_profile
	};

	CRSLICE_API void getMetaKeys(MetaGroup metaGroup, std::vector<std::string>& keys);
}

#endif // CRCOMMON_PARAMETERMETA_1690769853657_H