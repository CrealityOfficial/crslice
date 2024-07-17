#include "serialization.h"
#include "libslic3r/Model.hpp"
#include "libslic3r/PrintConfig.hpp"
#include "libslic3r/Print.hpp"

#include <fstream>
#include "ccglobal/log.h"
#include "ccglobal/serial.h"
#include "crslice2/header.h"


void cache_config(const Slic3r::DynamicPrintConfig& config, std::fstream& out)
{
	int count = (int)config.size();
	ccglobal::cxndSaveT(out, count);

	for (std::map<Slic3r::t_config_option_key, std::unique_ptr<Slic3r::ConfigOption>>::const_iterator it = config.cbegin();
		it != config.cend(); ++it)
	{
		ccglobal::cxndSaveStr(out, (*it).first);
		ccglobal::cxndSaveStr(out, (*it).second->serialize());
	}
}

void cache_config(const Slic3r::ModelConfig& config, std::fstream& out)
{
	int count = (int)config.size();
	ccglobal::cxndSaveT(out, count);

	for (std::map<Slic3r::t_config_option_key, std::unique_ptr<Slic3r::ConfigOption>>::const_iterator it = config.cbegin();
		it != config.cend(); ++it)
	{
		ccglobal::cxndSaveStr(out, (*it).first);
		ccglobal::cxndSaveStr(out, (*it).second->serialize());
	}
}

void cache_spread(const Slic3r::FacetsAnnotation& annotation, int faceCount, std::fstream& out)
{
	std::vector<std::string> strs;
	if (faceCount > 0)
	{
		strs.resize(faceCount);
		for (int i = 0; i < faceCount; ++i)
			strs.at(i) = annotation.get_triangle_as_string(i);
	}
	ccglobal::cxndSaveStrs(out, strs);
}

void cache_tranformation(const Slic3r::Geometry::Transformation& transfomr, std::fstream& out)
{
	trimesh::xform data = trimesh::xform::identity();

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			data[i + j * 4] = transfomr.get_matrix()(i, j);
		}
	}

	out.write((const char*)data.data(), 16 * sizeof(double));
}

void cache_volume(const Slic3r::ModelVolume& volume, std::fstream& out)
{
	cache_config(volume.config, out);

	const Slic3r::TriangleMesh& mesh = volume.mesh();
	int have = mesh.its.vertices.size() > 0 && mesh.its.indices.size() > 0;
	ccglobal::cxndSaveT(out, have);

	if (have)
	{
		ccglobal::cxndSaveVectorT(out, mesh.its.indices);
		ccglobal::cxndSaveVectorT(out, mesh.its.vertices);

		std::vector<int> flags;
		ccglobal::cxndSaveVectorT(out, flags);
	}

	int faceCount = (int)mesh.its.indices.size();
	int type = (int)volume.type();
	ccglobal::cxndSaveT(out, type);
	cache_tranformation(volume.get_transformation(), out);
	cache_spread(volume.mmu_segmentation_facets, faceCount, out);
	cache_spread(volume.supported_facets, faceCount, out);
	cache_spread(volume.seam_facets, faceCount, out);
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

	int ver = 101;
	out.write((const char*)&ver, sizeof(int));

	cache_config(config, out);

	int extruderCount = 0;
	ccglobal::cxndSaveT(out, extruderCount);

	int objectsCount = (int)model.objects.size();
	ccglobal::cxndSaveT(out, objectsCount);

	for (int i = 0; i < objectsCount; ++i)
	{
		Slic3r::ModelObject* object = model.objects.at(i);
		Slic3r::ModelInstance* instance = object->instances.at(0);
		if (instance->is_printable())
		{
			cache_config(object->config, out);

			int volumeCount = (int)object->volumes.size();
			ccglobal::cxndSaveT(out, volumeCount);

			for (int j = 0; j < volumeCount; ++j)
			{
				Slic3r::ModelVolume* volume = object->volumes.at(j);
				cache_volume(*volume, out);
			}

			cache_tranformation(instance->get_transformation(), out);
		}
	}
	out.close();
}