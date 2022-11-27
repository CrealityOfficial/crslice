#include "crobject.h"

namespace crslice
{
	CrObject::CrObject()
	{
		m_settings.reset(new crcommon::Settings());
	}

	CrObject::~CrObject()
	{

	}
}