#include "cxutil/input/extruderinput.h"

namespace cxutil
{
	ExtruderInput::ExtruderInput()
	{
		m_settings = new Settings();
		m_param = new ExtruderParam();
	}

	ExtruderInput::~ExtruderInput()
	{
		delete m_settings;
		delete m_param;
	}

	Settings* ExtruderInput::settings()
	{
		return m_settings;
	}

	ExtruderParam* ExtruderInput::param()
	{
		return m_param;
	}

	void ExtruderInput::finalize()
	{
		m_param->initialize(this);
	}
}