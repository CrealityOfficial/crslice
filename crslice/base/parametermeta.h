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
		}
	};

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
}

#endif // CRCOMMON_PARAMETERMETA_1690769853657_H