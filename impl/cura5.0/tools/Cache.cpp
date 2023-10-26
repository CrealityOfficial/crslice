// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#include <sstream>
#include "Cache.h"

namespace cura52
{
	Cache::Cache(const std::string& fileName)
		:m_root(fileName)
	{

	}

	Cache::~Cache()
	{

	}
} // namespace cura52
