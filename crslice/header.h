#ifndef CRSLICE_HEADER_INTERFACE
#define CRSLICE_HEADER_INTERFACE
#include "crcommon/header.h"
#include <memory>

namespace crslice
{
	class FDMDebugger
	{
	public:
		virtual ~FDMDebugger() {}

		virtual void tick(const std::string& tag) = 0;
	};
}
#endif // CRSLICE_HEADER_INTERFACE