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

#include "crslice2/base/parametermeta.h"

#include "crgroup.h"
#include "crobject.h"

#include <sstream>

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

void removeSpace(std::string& str)
{
	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
}

void Stringsplit(std::string str, const char split, std::vector<std::string>& res)
{
	std::istringstream iss(str);	// 输入流
	std::string token;			// 接收缓冲区
	while (getline(iss, token, split))	// 以split为分隔符
	{
		removeSpace(token);
		res.push_back(token);
	}
}

Slic3r::ConfigOption* _set_key_value(crslice2::MetasMap& datas, const std::string& key, const std::string& value)
{
	std::string type = "";
	auto iter = datas.find(key);
	if (iter == datas.end() || value.empty())
	{
		return new Slic3r::ConfigOptionString(value);
	}

	type = iter->second->type;

	if (type.find("coFloats") != std::string::npos)
	{
		std::vector<std::string> res;
		std::vector<double> vec1;
		Stringsplit(value, ',', res);
		for (auto& v : res)
			vec1.push_back(std::stof(v));
		return new Slic3r::ConfigOptionFloats(vec1);
	}
	else if (type.find("coFloat") != std::string::npos)
	{
		return new Slic3r::ConfigOptionFloat(std::stof(value));
	}
	else if (type.find("coInts") != std::string::npos)
	{
		std::vector<std::string> res;
		std::vector<int> vec_i;
		Stringsplit(value, ',', res);
		for (auto& v : res)
			vec_i.push_back(std::stoi(v));
		return new Slic3r::ConfigOptionInts(vec_i);
	}
	else if (type.find("coInt") != std::string::npos)
	{
		return new Slic3r::ConfigOptionInt(std::stoi(value));
	}
	else if (type.find("coStrings") != std::string::npos)
	{
		std::vector<std::string> res;
		Stringsplit(value, ',', res);
		return new Slic3r::ConfigOptionStrings(res);
	}
	else if (type.find("coString") != std::string::npos)
	{
		return new Slic3r::ConfigOptionString(value);
	}
	else if (type.find("coPercents") != std::string::npos)
	{
		std::vector<std::string> res;
		std::vector<double> vec1;
		Stringsplit(value, ',', res);
		for (auto& v : res)
			vec1.push_back(std::stof(v));
		return new Slic3r::ConfigOptionPercents(vec1);
	}
	else if (type.find("coPercent") != std::string::npos)
	{
		return new Slic3r::ConfigOptionPercent(std::stof(value));
	}
	else if (type.find("coFloatsOrPercents") != std::string::npos)
	{
		std::vector<std::string> res;
		Slic3r::FloatOrPercent floatOrPercent;
		std::vector<Slic3r::FloatOrPercent> vec_p;
		Stringsplit(value, ',', res);
		for (auto& v : res) {
			floatOrPercent.value = std::stof(v);
			floatOrPercent.percent = false;
			vec_p.push_back(floatOrPercent);
		}
		return new Slic3r::ConfigOptionFloatsOrPercents(vec_p);
	}
	else if (type.find("coFloatOrPercent") != std::string::npos)
	{
		return new Slic3r::ConfigOptionFloatOrPercent(std::stof(value), false);
	}
	else if (type.find("coPoint3") != std::string::npos)
	{
		std::vector<std::string> res;
		Slic3r::Vec3d vec3d;
		Stringsplit(value, ',', res);
		if (res.size() > 2) {
			vec3d.x() = std::stof(res[0]);
			vec3d.y() = std::stof(res[1]);
			vec3d.z() = std::stof(res[2]);
		}
		return new Slic3r::ConfigOptionPoint3(vec3d);
	}
	else if (type.find("coPoints") != std::string::npos)
	{
		std::vector<std::string> res;
		Slic3r::Vec2d vec2d;
		std::vector<Slic3r::Vec2d> vec2ds;
		Stringsplit(value, ',', res);
		for (int i = 0; i < res.size() - 1; i++, i++) {
			vec2d.x() = std::stof(res[i]);
			vec2d.y() = std::stof(res[i + 1]);
			vec2ds.push_back(vec2d);
		}
		return new Slic3r::ConfigOptionPoints(vec2ds);
	}
	else if (type.find("coPoint") != std::string::npos)
	{
		std::vector<std::string> res;
		Slic3r::Vec2d vec2d;
		Stringsplit(value, ',', res);
		if (res.size() > 1) {
			vec2d.x() = std::stof(res[0]);
			vec2d.y() = std::stof(res[1]);
		}
		return new Slic3r::ConfigOptionPoint(vec2d);
	}
	//else if (type.find("coPoint3s") != std::string::npos)
	//{
	//	return  new Slic3r::ConfigOptionString(value);
	//}
	else if (type.find("coBools") != std::string::npos)
	{
	std::vector<std::string> res;
	std::vector<unsigned char> vec_bs;
	Stringsplit(value, ',', res);
	for (auto& v : res)
		vec_bs.push_back(std::stoi(v) > 0 ? true : false);
	return new Slic3r::ConfigOptionBools(vec_bs);
	}
	else if (type.find("coBool") != std::string::npos)
	{
		bool vec_b = std::stoi(value) > 0 ? true : false;
		return new Slic3r::ConfigOptionBool(vec_b);
	}
	else if ((type.find("coEnum") != std::string::npos)
		|| (type.find("coEnums") != std::string::npos))
	{
		std::unordered_map<std::string, std::string>& options = iter->second->options;
		Slic3r::t_config_enum_values config;
		auto iter = options.begin();
		int i = 0;
		int current = 0;
		while (iter != options.end())
		{
			config.insert(std::make_pair(iter->first, i));
			if (iter->first == value)
				current = i;
			iter++;
			i++;
		}
		return new Slic3r::ConfigOptionEnumGeneric(&config, current);
	}
}

void convert_scene_2_orca(crslice2::CrScenePtr scene, Slic3r::Model& model, Slic3r::DynamicPrintConfig& config)
{
	size_t numGroup = scene->m_groups.size();
	assert(numGroup > 0);

	crslice2::MetasMap datas;
	parseMetasMap(datas);

	for (const std::pair<std::string, std::string> pair : scene->m_settings->settings)
	{
		config.set_key_value(pair.first, _set_key_value(datas, pair.first, pair.second));
	}

	std::vector<Slic3r::ModelObject*> objects;
	for (crslice2::CrGroup* aCrgroup : scene->m_groups)
	{
		Slic3r::ModelObject* currentObject = model.add_object();
		objects.push_back(currentObject);

		currentObject->config.assign_config(config);
		for (const std::pair<std::string, std::string> pair : aCrgroup->m_settings->settings)
		{
			currentObject->config.set_key_value(pair.first, _set_key_value(datas, pair.first, pair.second));
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
				v->config.set_key_value(pair.first, _set_key_value(datas, pair.first, pair.second));
			}
		}
	}
}


void slice_impl(const Slic3r::Model& model, const Slic3r::DynamicPrintConfig& config, 
	bool is_bbl_printer, const Slic3r::Vec3d& plate_origin,
	const std::string& out)
{
	Slic3r::GCodeProcessorResult result;
	Slic3r::Print print;
	print.apply(model, config);
	
	print.is_BBL_printer() = is_bbl_printer;
	print.set_plate_origin(plate_origin);

	Slic3r::StringObjectException warning;
	//BBS: refine seq-print logic
	Slic3r::Polygons polygons;
	std::vector<std::pair<Slic3r::Polygon, float>> height_polygons;
	print.validate(&warning, &polygons, &height_polygons);

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

	slice_impl(model, config, false, Slic3r::Vec3d(0.0, 0.0, 0.0), scene->m_gcodeFileName);
}

void orca_slice_fromfile_impl(const std::string& file, const std::string& out)
{
	std::ifstream in(file, std::ios::in | std::ios::binary);

	cereal::BinaryInputArchive iarchive(in);

	bool is_bbl_printer = false;
	Slic3r::Vec3d plate_origin = Slic3r::Vec3d(0.0, 0.0, 0.0);
	Slic3r::Model model;
	Slic3r::DynamicPrintConfig config;

	iarchive(is_bbl_printer);
	iarchive(plate_origin);
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
			int order = 0;
			iarchive(order);
			o->instances.at(j)->arrange_order = order;
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
	slice_impl(model, config, is_bbl_printer, plate_origin, out);
}
