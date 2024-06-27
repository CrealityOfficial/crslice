#include "orcaslicewrapper.h"
#include "libslic3r/Model.hpp"
#include "libslic3r/PrintConfig.hpp"
#include "libslic3r/Print.hpp"

#include <fstream>
#include "ccglobal/log.h"
#include "ccglobal/serial.h"

namespace Slic3r
{
	void Print::debug(const std::string& name)
	{
		if (!m_debug)
			return;
	}

	void cache_config(const Slic3r::DynamicPrintConfig& config, std::fstream& out)
	{

	}

	void cache_config(const Slic3r::ModelConfig& config, std::fstream& out)
	{

	}

	void cache_volume(const Slic3r::ModelVolume& volume, std::fstream& out)
	{

	}

	void cache_slice_scene(const Slic3r::Print& print, const Slic3r::Model& model, const Slic3r::DynamicPrintConfig& config, const std::string& file_name)
	{
		std::fstream out(file_name, std::ios::out | std::ios::binary);
		if (!out.is_open())
		{
			LOGE("cxndSave error. [%s]", file_name.c_str());
			out.close();
			return;
		}

		cache_config(config, out);
		int ver = 101;
		out.write((const char*)&ver, sizeof(int));
		
		int extruderCount = 0;
		ccglobal::cxndSaveT(out, extruderCount);

		for (Slic3r::ModelObject* object : model.objects)
		{
			Slic3r::ModelInstance* instance = object->instances.at(0);
			if (instance->is_printable())
			{
				for (Slic3r::ModelVolume* volume : object->volumes)
				{
					cache_volume(*volume, out);
				}
			}
		}
		out.close();
	}
}
