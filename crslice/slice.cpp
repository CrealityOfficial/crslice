#include "slice.h"
#include "Application.h"

namespace crslice
{
	Slice::Slice()
	{

	}

	Slice::~Slice()
	{

	}
	void Slice::init(crcommon::Settings* settingsPtr)
	{
		if(settingsPtr !=nullptr)
			m_settingsCfg = *settingsPtr;
	}
	void Slice::process()
	{
		 int argc = 10;
		 char* argv[] =
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