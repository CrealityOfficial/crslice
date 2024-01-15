#include "orcaslicewrapper.h"
#include <sstream>

#include "crslice2/base/parametermeta.h"
#include <boost/nowide/fstream.hpp>

#include "crgroup.h"
#include "crobject.h"
#include "ccglobal/log.h"

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
#include "nlohmann/json.hpp"

void save_parameter_2_json(const std::string& fileName, const Slic3r::Model& model, const Slic3r::DynamicPrintConfig& config)
{
	using namespace Slic3r;
	using namespace nlohmann;

	json j;

	auto save_dynamic_config = [](const Slic3r::DynamicPrintConfig& config, json& j) {
		t_config_option_keys ks = config.keys();
		for (const t_config_option_key& k : ks)
		{
			const ConfigOption* option = config.optptr(k);
			j[k] = option->serialize();
		}
	};
	{
		json G;
		save_dynamic_config(config, G);
		j["global"] = G;
	}

	{
		json M;
		int size = (int)model.objects.size();
		for (int i = 0; i < size; ++i)
		{
			json MO;
			ModelObject* object = model.objects.at(i);
			save_dynamic_config(object->config.get(), MO);
			int vsize = (int)object->volumes.size();
			for (int j = 0; j < vsize; ++j)
			{
				json MOV;
				ModelVolume* volume = object->volumes.at(j);
				save_dynamic_config(volume->config.get(), MOV);

				MO[std::to_string(j)] = MOV;
			}

			M[std::to_string(i)] = MO;
		}
		j["model"] = M;
	}

	boost::nowide::ofstream c;
	c.open("parameter.json", std::ios::out | std::ios::trunc);
	c << std::setw(4) << j << std::endl;
	c.close();
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

Slic3r::ConfigOption* _set_key_value(const std::string& value, const Slic3r::ConfigOptionDef* cDef)
{
	Slic3r::ConfigOption* option = nullptr;
	if (cDef)
	{
		option = cDef->create_empty_option();
	}
	else {
		option = new Slic3r::ConfigOptionString();
	}

	if(!option)
		option = new Slic3r::ConfigOptionString();

	option->deserialize(value);
	return option;
}

void convert_scene_2_orca(crslice2::CrScenePtr scene, Slic3r::Model& model, Slic3r::DynamicPrintConfig& config,Slic3r::Calib_Params& _calibParams)
{
	size_t numGroup = scene->m_groups.size();
	assert(numGroup > 0);

	const Slic3r::ConfigDef* _def = config.def();
	for (const std::pair<std::string, std::string> pair : scene->m_settings->settings)
	{
		config.set_key_value(pair.first, _set_key_value(pair.second, _def->get(pair.first)));
	}

	for (crslice2::CrGroup* aCrgroup : scene->m_groups)
	{
		Slic3r::ModelObject* currentObject = model.add_object();
		currentObject->add_instance();

		//currentObject->config.assign_config(config);
		for (const std::pair<std::string, std::string> pair : aCrgroup->m_settings->settings)
		{
			currentObject->config.set_key_value(pair.first, _set_key_value(pair.second, _def->get(pair.first)));
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

			//v->config.assign_config(currentObject->config);
			for (const std::pair<std::string, std::string> pair : aObject.m_settings->settings)
			{
				v->config.set_key_value(pair.first, _set_key_value(pair.second, _def->get(pair.first)));
			}
		}
	}

	if (scene->m_calibParams.mode != crslice2::CalibMode::Calib_None)
	{
		_calibParams.start = scene->m_calibParams.start;
		_calibParams.end = scene->m_calibParams.end;
		_calibParams.step = scene->m_calibParams.step;
		_calibParams.print_numbers = scene->m_calibParams.print_numbers;
		_calibParams.mode = (Slic3r::CalibMode)scene->m_calibParams.mode;
	}
}


void slice_impl(const Slic3r::Model& model, const Slic3r::DynamicPrintConfig& config, 
	bool is_bbl_printer, const Slic3r::Vec3d& plate_origin,
	const std::string& out, Slic3r::Calib_Params& _calibParams)
{
#if _DEBUG
	save_parameter_2_json("", model, config);
#endif

	Slic3r::GCodeProcessorResult result;
	Slic3r::Print print;
	print.set_calib_params(_calibParams);
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
	Slic3r::Calib_Params calibParams;
	convert_scene_2_orca(scene, model, config, calibParams);

	slice_impl(model, config, false, Slic3r::Vec3d(0.0, 0.0, 0.0), scene->m_gcodeFileName, calibParams);
}

void orca_slice_fromfile_impl(const std::string& file, const std::string& out)
{
	std::ifstream in(file, std::ios::in | std::ios::binary);

	bool is_bbl_printer = false;
	Slic3r::Vec3d plate_origin = Slic3r::Vec3d(0.0, 0.0, 0.0);
	Slic3r::Model model;
	Slic3r::DynamicPrintConfig config;

#if 1
	cereal::BinaryInputArchive iarchive(in);
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
#endif 

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
	Slic3r::Calib_Params calibParams;
	slice_impl(model, config, is_bbl_printer, plate_origin, out, calibParams);
}

void parse_metas_map_impl(crslice2::MetasMap& datas)
{
	using namespace Slic3r;

	datas.clear();
	DynamicPrintConfig full_config;
	const ConfigDef* _def = full_config.def();

	for (Slic3r::t_optiondef_map::const_iterator it = _def->options.begin(); it != _def->options.end(); ++it)
	{
		const Slic3r::ConfigOptionDef& optDef = it->second;
		if (optDef.printer_technology != Slic3r::ptSLA)
		{
			crslice2::ParameterMeta meta;
			
			meta.name = it->first;
			meta.label = optDef.label;
			meta.description = optDef.tooltip;
			
			std::string type = "coNone";
			switch (optDef.type)
			{
			case coFloat:
				type = "coFloat";
				break;
			case coFloats:
				type = "coFloats";
				break;
			case coInt:
				type = "coInt";
				break;
			case coInts:
				type = "coInts";
				break;
			case coString:
				type = "coString";
				break;
			case coStrings:
				type = "coStrings";
				break;
			case coPercent:
				type = "coPercent";
				break;
			case coPercents:
				type = "coPercents";
				break;
			case coFloatOrPercent:
				type = "coFloatOrPercent";
				break;
			case coFloatsOrPercents:
				type = "coFloatsOrPercents";
				break;
			case coPoint:
				type = "coPoint";
				break;
			case coPoints:
				type = "coPoints";
				break;
			case coPoint3:
				type = "coPoint3";
				break;
			case coBool:
				type = "coBool";
				break;
			case coBools:
				type = "coBools";
				break;
			case coEnum:
				type = "coEnum";
				break;
			case coEnums:
				type = "coEnums";
				break;
			};
			
			meta.type = type;
			meta.enabled = "true";
			Slic3r::ConfigOption* option = optDef.create_default_option();
			meta.default_value = option->serialize();
			delete option;

			meta.settable_per_mesh = "false";
			meta.unit = optDef.sidetext;
			//meta.settable_per_extruder = "false";
			//meta.settable_per_meshgroup = "false";
			//meta.settable_globally = "false";
			if (optDef.enum_values.size() > 0)
			{
				size_t size = optDef.enum_values.size();
				
				bool have = optDef.enum_labels.size() == optDef.enum_values.size();
				for (size_t i = 0; i < size; ++i)
				{
					meta.options.insert(crslice2::OptionValue(optDef.enum_values.at(i),
						(have ? optDef.enum_labels.at(i) : optDef.enum_values.at(i))));
				}
			}
			
			datas.insert(crslice2::MetasMap::value_type(it->first, new crslice2::ParameterMeta(meta)));
		}
	}
}

void get_meta_keys_impl(crslice2::MetaGroup metaGroup, std::vector<std::string>& keys)
{
	keys.clear();
	Slic3r::DynamicPrintConfig full_config;
	const Slic3r::ConfigDef* _def = full_config.def();

	for (Slic3r::t_optiondef_map::const_iterator it = _def->options.begin(); it != _def->options.end(); ++it)
	{
		const Slic3r::ConfigOptionDef& optionDef = it->second;
		if (optionDef.printer_technology != Slic3r::ptSLA)
		{
			keys.push_back(it->first);
		}
	}
}
