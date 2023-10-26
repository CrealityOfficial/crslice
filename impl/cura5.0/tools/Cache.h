//Copyright (c) 2022 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef CACHE_H
#define CACHE_H
#include <string>

namespace cura52 
{
	class Cache 
	{
	public:
		Cache(const std::string& fileName);
		virtual ~Cache();

	protected:
		std::string m_root;
	};
} // namespace cura52
#endif // SVG_H
