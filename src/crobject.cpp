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

	void CrObject::load(std::ifstream& in)
	{

	}

	void CrObject::save(std::ofstream& out)
	{
		m_settings->save(out);
		int have = m_mesh ? 1 : 0;
		templateSave(have, out);

		if (m_mesh)
		{
			templateSave(m_mesh->faces, out);
			templateSave(m_mesh->vertices, out);
		}
	}
}