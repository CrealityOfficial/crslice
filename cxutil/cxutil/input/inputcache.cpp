#include "cxutil/input/inputcache.h"
#include "cxutil/input/meshobject.h"
#include "cxutil/input/dlpinput.h"
#include "cxutil/util/meshbuilder.h"
#include <fstream>

namespace cxutil
{
	InputCache::InputCache()
	{

	}

	InputCache::~InputCache()
	{

	}

	void InputCache::save(const char* file, std::vector<float*>& vData, std::vector<int>& vNr)
	{
		if (vData.size() == vNr.size())
		{
			size_t size = vData.size();
			if (size > 0)
			{
				m_data.resize(size);
				for (size_t i = 0; i < size; ++i)
				{
					std::vector<float>& data = m_data.at(i);
					int Nr = vNr.at(i);
					float* v = vData.at(i);

					if (Nr > 0)
					{
						data.resize(3 * Nr);
						memcpy(&data.at(0), v, sizeof(float) * 3 * Nr);
					}
				}
			}
		}
		save(file);
	}

	void InputCache::save(const char* file)
	{
		std::fstream out;
		out.open(file, std::ios::out | std::ios::binary);
		if (out.is_open())
		{
			int size = (int)m_data.size();
			out.write((const char*)&size, sizeof(int));
			for (int i = 0; i < size; ++i)
			{
				std::vector<float>& data = m_data.at(i);
				int dSize = (int)data.size();
				out.write((const char*)&dSize, sizeof(int));
				if (dSize > 0)
					out.write((const char*)&data.at(0), dSize * sizeof(float));
			}
		}
		out.close();
	}

	void InputCache::load(const char* file)
	{
		std::fstream in;
		in.open(file, std::ios::in | std::ios::binary);
		if (in.is_open())
		{
			int size = 0;
			in.read((char*)&size, sizeof(int));
			if (size > 0) m_data.resize(size);
			for (int i = 0; i < size; ++i)
			{
				std::vector<float>& data = m_data.at(i);
				int dSize = 0;
				in.read((char*)&dSize, sizeof(int));
				if (dSize > 0)
				{
					data.resize(dSize);
					in.read((char*)&data.at(0), dSize * sizeof(float));
				}
			}
		}
		in.close();
	}

	void InputCache::fill(DLPInput* input)
	{
		if (!input)
			return;

		size_t size = m_data.size();
		for(size_t i = 0; i < size; ++i)
		{
			std::vector<float>& data = m_data.at(i);

			int vertexNum = data.size() / 3;
			float* vertex = vertexNum > 0 ? &data.at(0) : nullptr;

			if (vertexNum < 3 || !vertex)
				continue;

			MeshObject* object = meshFromTriangleSoup(vertex, vertexNum);
			if (object)
				input->addMeshObject(MeshObjectPtr(object));
		}
	}
}