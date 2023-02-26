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
		m_settings->load(in);
		int have = templateLoad<int>(in);

		if (have)
		{
			m_mesh.reset(new trimesh::TriMesh());
			templateLoad(m_mesh->faces, in);
			templateLoad(m_mesh->vertices, in);
		}
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