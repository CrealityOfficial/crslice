#include "crslice/crcacheslice.h"
#include "Application.h"

#include "conv.h"

#include "ccglobal/log.h"
#include "cachedebugger.h"

namespace crslice
{
	CrSCacheSlice::CrSCacheSlice()
	{

	}

	CrSCacheSlice::~CrSCacheSlice()
	{

	}

	void CrSCacheSlice::sliceFromScene(CrScenePtr scene, ccglobal::Tracer* tracer)
	{
		if (!scene)
		{
			LOGM("CrSlice::sliceFromScene empty scene.");
			return;
		}

		cura52::Application app(tracer);
		app.tempDirectory = scene->m_tempDirectory;

		m_debugger.reset(new CacheDebugger(&app, scene));
		app.debugger = m_debugger.get();

		app.runCommulication(m_debugger.get());
	}

	trimesh::box3 CrSCacheSlice::groupBox()
	{
		trimesh::box3 b;
		b += convert(m_debugger->groupData._min);
		b += convert(m_debugger->groupData._max);
		return b;
	}
}