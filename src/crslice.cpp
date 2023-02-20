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

	void CrSlice::sliceFromFakeArguments(int argc, const char* argv[], ccglobal::Tracer* tracer)
	{
		cura52::Application app;
		app.run(argc, argv, tracer);
	}

	void CrSlice::sliceFromScene(CrScenePtr scene, ccglobal::Tracer* tracer)
	{
		if (!scene)
		{
			LOGM("CrSlice::sliceFromScene empty scene.");
			return;
		}

		cura52::Application app;
		app.runCommulication(new CRSliceFromScene(scene, tracer));
	}

	void CrSlice::process()
	{
		int argc = 10;
		const char* argv[] =
		{
		"self_exe",
		"slice" ,
		"-v",
		"-j",
		"creality_cr10.def.json",
		"-e0",
		"-l",
		"testModel.stl", 
		"-o",
		"cura.gcode"
		};

		cura52::Application app;
		app.run(argc, argv);
	}
}