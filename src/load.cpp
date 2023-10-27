#include "crslice/load.h"

namespace crslice
{
	int SerailSlicedLayer::version()
	{
		return 0;
	}

	bool SerailSlicedLayer::save(std::fstream& out, ccglobal::Tracer* tracer)
	{
		return true;
	}

	bool SerailSlicedLayer::load(std::fstream& in, int ver, ccglobal::Tracer* tracer)
	{
		return true;
	}



	/// file name
	std::string sliced_layer_name(const std::string& root, int meshId, int layer)
	{
		char buffer[1024] = { 0 };

		sprintf_s(buffer, 1024, "%s/mesh%d_layer%d.sliced_layer", root.c_str(), meshId, layer);
		return std::string(buffer);
	}
}