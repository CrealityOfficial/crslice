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

	void CrSlice::sliceFromFakeArguments(int argc, const char* argv[])
	{
		cura::Application::getInstance().run(argc, argv);
	}

	void CrSlice::sliceFromScene(CrScenePtr scene)
	{
		if (scene)
		{
			LOGM("CrSlice::sliceFromScene empty scene.");
			SliceInitCfg sliceCfg;
			//sliceCfg.initCfgFile =  "G:/Work/CuraEngine/Visualization/testUnit/testData/creality_cr10.def.json";
			//sliceCfg.gcodeOutFile = "G:/Work/CuraEngine/Visualization/testUnit/testData/output/curaOut.gcode";
			//sliceCfg.testMeshFile = "G:/Work/CuraEngine/Visualization/testUnit/testData/test.stl";
			CRSliceFromScene crsliceHandler(scene, sliceCfg);
			crsliceHandler.sliceNext();
			return;
		}


	}

	void CrSlice::init(crcommon::Settings* settingsPtr)
	{
		if(settingsPtr !=nullptr)
			m_settingsCfg = *settingsPtr;
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

		cura::Application::getInstance().run(argc, argv);
	}
}