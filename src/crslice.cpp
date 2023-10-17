#include "crslice/crslice.h"
#include "Application.h"

#include "ccglobal/log.h"
#include "crslicefromscene.h"

namespace crslice
{
	CrSlice::CrSlice()
		:sliceResult({0})
	{

	}

	CrSlice::~CrSlice()
	{

	}

	void CrSlice::sliceFromScene(CrScenePtr scene, ccglobal::Tracer* tracer)
	{
		if (!scene)
		{
			LOGM("CrSlice::sliceFromScene empty scene.");
			return;
		}

		cura52::Application app(tracer);
		app.tempDirectory = scene->m_tempDirectory;
		app.fDebugger = scene->m_debugger;

		CRSliceFromScene factory(&app, scene);
		cura52::SliceResult result = app.runSceneFactory(&factory);
        sliceResult = { result.print_time, result.filament_len, result.filament_volume, result.layer_count,
			result.x, result.y, result.z };
	}
}