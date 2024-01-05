#include "orcaslicewrapper.h"

#include <cereal/types/polymorphic.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#define CEREAL_FUTURE_EXPERIMENTAL
#include <cereal/archives/adapters.hpp>

#include "libslic3r/Model.hpp"
#include "libslic3r/PrintConfig.hpp"
#include "libslic3r/Print.hpp"

#include "crgroup.h"
#include "crobject.h"

namespace cereal
{
	// Let cereal know that there are load / save non-member functions declared for ModelObject*, ignore serialization of pointers triggering
	// static assert, that cereal does not support serialization of raw pointers.
	template <class Archive> struct specialize<Archive, Slic3r::Model*, cereal::specialization::non_member_load_save> {};
	template <class Archive> struct specialize<Archive, Slic3r::ModelObject*, cereal::specialization::non_member_load_save> {};
	template <class Archive> struct specialize<Archive, Slic3r::ModelVolume*, cereal::specialization::non_member_load_save> {};
	template <class Archive> struct specialize<Archive, Slic3r::ModelInstance*, cereal::specialization::non_member_load_save> {};
	template <class Archive> struct specialize<Archive, Slic3r::ModelMaterial*, cereal::specialization::non_member_load_save> {};
	template <class Archive> struct specialize<Archive, std::shared_ptr<Slic3r::TriangleMesh>, cereal::specialization::non_member_load_save> {};
}

void trimesh2Slic3rTriangleMesh(trimesh::TriMesh* mesh, Slic3r::TriangleMesh& tmesh)
{
	if (!mesh)
		return;
	int pointSize = (int)mesh->vertices.size();
	int facesSize = (int)mesh->faces.size();
	if (pointSize < 3 || facesSize < 1)
		return;

	indexed_triangle_set indexedTriangleSet;
	indexedTriangleSet.vertices.resize(pointSize);
	indexedTriangleSet.indices.resize(facesSize);
	for (int i = 0; i < pointSize; i++)
	{
		const trimesh::vec3& v = mesh->vertices.at(i);
		indexedTriangleSet.vertices.at(i) = stl_vertex(v.x, v.y, v.z);
	}

	for (int i = 0; i < facesSize; i++)
	{
		const trimesh::TriMesh::Face& f = mesh->faces.at(i);
		stl_triangle_vertex_indices faceIndex(f[0], f[1], f[2]);
		indexedTriangleSet.indices.at(i) = faceIndex;
	}

	stl_file stl;
	stl.stats.type = inmemory;
	// count facets and allocate memory
	stl.stats.number_of_facets = uint32_t(indexedTriangleSet.indices.size());
	stl.stats.original_num_facets = int(stl.stats.number_of_facets);
	stl_allocate(&stl);

#pragma omp parallel for
	for (int i = 0; i < (int)stl.stats.number_of_facets; ++i) {
		stl_facet facet;
		facet.vertex[0] = indexedTriangleSet.vertices[size_t(indexedTriangleSet.indices[i](0))];
		facet.vertex[1] = indexedTriangleSet.vertices[size_t(indexedTriangleSet.indices[i](1))];
		facet.vertex[2] = indexedTriangleSet.vertices[size_t(indexedTriangleSet.indices[i](2))];
		facet.extra[0] = 0;
		facet.extra[1] = 0;

		stl_normal normal;
		stl_calculate_normal(normal, &facet);
		stl_normalize_vector(normal);
		facet.normal = normal;

		stl.facet_start[i] = facet;
	}

	stl_get_size(&stl);
	tmesh.from_stl(stl);
}

void convert_scene_2_orca(crslice2::CrScenePtr scene, Slic3r::Model& model, Slic3r::DynamicPrintConfig& config)
{
	size_t numGroup = scene->m_groups.size();
	assert(numGroup > 0);

	for (const std::pair<std::string, std::string> pair : scene->m_settings->settings)
	{
		config.set_key_value(pair.first, config.optptr(pair.second));
	}

	std::vector<Slic3r::ModelObject*> objects;
	for (crslice2::CrGroup* aCrgroup : scene->m_groups)
	{
		Slic3r::ModelObject* currentObject = model.add_object();
		objects.push_back(currentObject);

		currentObject->config.assign_config(config);
		for (const std::pair<std::string, std::string> pair : aCrgroup->m_settings->settings)
		{
			currentObject->config.set_key_value(pair.first, config.optptr(pair.second));
		}
		for (crslice2::CrObject aObject : aCrgroup->m_objects)
		{
			Slic3r::TriangleMesh mesh;
			trimesh2Slic3rTriangleMesh(aObject.m_mesh.get(), mesh);
			Slic3r::ModelVolume* v = currentObject->add_volume(mesh);

			if (aObject.m_mesh->faces.size() == aObject.m_colors2Facets.size())
				for (size_t i = 0; i < aObject.m_mesh->faces.size(); i++) {
					if (!aObject.m_colors2Facets[i].empty())
						v->mmu_segmentation_facets.set_triangle_from_string(i, aObject.m_colors2Facets[i]);
				}

			if (aObject.m_mesh->faces.size() == aObject.m_seam2Facets.size())
				for (size_t i = 0; i < aObject.m_mesh->faces.size(); i++) {
					if (!aObject.m_seam2Facets[i].empty())
						v->seam_facets.set_triangle_from_string(i, aObject.m_seam2Facets[i]);
				}

			if (aObject.m_mesh->faces.size() == aObject.m_support2Facets.size())
				for (size_t i = 0; i < aObject.m_mesh->faces.size(); i++) {
					if (!aObject.m_support2Facets[i].empty())
						v->supported_facets.set_triangle_from_string(i, aObject.m_support2Facets[i]);
				}

			v->config.assign_config(currentObject->config);
			for (const std::pair<std::string, std::string> pair : aObject.m_settings->settings)
			{
				v->config.set_key_value(pair.first, config.optptr(pair.second));
			}
			currentObject->volumes.push_back(v);
		}
	}
}


void slice_impl(const Slic3r::Model& model, const Slic3r::DynamicPrintConfig& config, const std::string& out)
{
	Slic3r::GCodeProcessorResult result;
	Slic3r::Print print;
	print.apply(model, config);
	
	print.is_BBL_printer() = false;
	print.set_plate_origin(Slic3r::Vec3d(0.0, 0.0, 0.0));

	//BBS: reset the gcode before reload_print in slicing_completed event processing
	//FIX the gcode rename failed issue
	BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(" %1%: will start slicing, reset gcode_result firstly") % __LINE__ ;
	result.reset();

	BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(" %1%: gcode_result reseted, will start print::process") % __LINE__;
	print.process();
	BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(" %1%: after print::process, send slicing complete event to gui...") % __LINE__;

	print.export_gcode(out, &result, nullptr);
	BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(": export gcode finished");
}

void orca_slice_impl(crslice2::CrScenePtr scene, ccglobal::Tracer* tracer)
{
	if (!scene)
		return;

	Slic3r::Model model;
	Slic3r::DynamicPrintConfig config;
	convert_scene_2_orca(scene, model, config);

	slice_impl(model, config, scene->m_gcodeFileName);
}

void orca_slice_fromfile_impl(const std::string& file, const std::string& out)
{
	std::ifstream in(file, std::ios::in | std::ios::binary);

	cereal::BinaryInputArchive iarchive(in);

	Slic3r::Model model;
	Slic3r::DynamicPrintConfig config;

	size_t count;
	iarchive(count);

	std::vector<Slic3r::ModelObject*> objects;
	for (size_t i = 0; i < count; ++i) {
		objects.push_back(model.add_object());
	}
	for (size_t i = 0; i < count; ++i) {
		Slic3r::ModelObject* o = objects.at(i);

		iarchive(o->name, o->module_name, o->config, o->layer_config_ranges, o->layer_height_profile, o->printable,
			o->origin_translation);

		size_t i_count = o->instances.size();
		iarchive(i_count);
		for (size_t j = 0; j < i_count; ++j) {
			o->add_instance();
		}
		for (size_t j = 0; j < i_count; ++j) {
			iarchive(*o->instances.at(j));
		}

		size_t v_count = o->volumes.size();
		iarchive(v_count);

		std::vector<Slic3r::TriangleMesh> meshes;
		if (v_count > 0)
			meshes.resize(v_count);
		for (size_t j = 0; j < v_count; ++j) {
			Slic3r::TriangleMesh mesh;
			iarchive(mesh);

			Slic3r::ModelVolume* v = o->add_volume(mesh);
			iarchive(v->config);
			iarchive(v->supported_facets);
			iarchive(v->seam_facets);
			iarchive(v->mmu_segmentation_facets);
		}
	}
	iarchive >> config;
	 
#if 0
	//in.seekg(0, std::ios::end);
	//int len = in.tellg();
	//in.seekg(0, std::ios::beg);
	//std::vector<size_t> buff(len / sizeof(size_t));
	//in.read((char*)buff.data(), len);

	//size_t cnt;
	//iarchive(cnt);
	//for (size_t i = 0; i < cnt; ++i) {
	//	size_t serialization_key_ordinal;
	//	iarchive(serialization_key_ordinal);
	//	std::cout << serialization_key_ordinal << std::endl;
	//}
#endif
	slice_impl(model, config, out);
}
