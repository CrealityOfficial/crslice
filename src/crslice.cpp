#include "crslice/crslice.h"
#include "Application.h"

#include "ccglobal/log.h"
#include "crslicefromscene.h"

namespace crslice
{
	CrSlice::CrSlice()
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
		app.runCommulication(new CRSliceFromScene(scene));
        sliceResult = { app.sliceResult.print_time,app.sliceResult.filament_len ,app.sliceResult.filament_volume,app.sliceResult.layer_count,
            app.sliceResult.x,app.sliceResult.y,app.sliceResult.z };
	}
}