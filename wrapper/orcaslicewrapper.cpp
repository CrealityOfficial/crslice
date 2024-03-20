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
#include "libslic3r/Slicing.hpp"
#include "libslic3r/Preset.hpp"
#include "libslic3r/PrintBase.hpp"
#include "nlohmann/json.hpp"
#include <boost/nowide/fstream.hpp>

#include "crslice2/base/parametermeta.h"

#include "GCode/ThumbnailData.hpp"

#include "crgroup.h"
#include "crobject.h"
#include "ccglobal/log.h"

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

void save_nlohmann_json(const std::string& fileName, const nlohmann::ordered_json& j)
{
	boost::nowide::ofstream c;
	c.open(fileName.c_str(), std::ios::out | std::ios::trunc);
	c << std::setw(4) << j << std::endl;
	c.close();
}

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
	//c.open("cx_parameter.json", std::ios::out | std::ios::trunc);
	c.open(fileName.c_str(), std::ios::out | std::ios::trunc);
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

	if (!option)
		option = new Slic3r::ConfigOptionString();

	option->deserialize(value);
	return option;
}

void convert_scene_2_orca(crslice2::CrScenePtr scene, Slic3r::Model& model, Slic3r::DynamicPrintConfig& config, Slic3r::Calib_Params& _calibParams, Slic3r::ThumbnailsList& thumbnailData)
{
	size_t numGroup = scene->m_groups.size();
	assert(numGroup > 0);

	const Slic3r::ConfigDef* _def = config.def();
	for (const std::pair<std::string, std::string> pair : scene->m_settings->settings)
	{
		config.set_key_value(pair.first, _set_key_value(pair.second, _def->get(pair.first)));
	}

	//for (const std::pair<std::string, std::string> pair : scene->m_extruders[0]->settings)
	//{
	//	config.set_key_value(pair.first, _set_key_value(pair.second, _def->get(pair.first)));
	//}

	for (crslice2::CrGroup* aCrgroup : scene->m_groups)
	{
		Slic3r::ModelObject* currentObject = model.add_object();
		Slic3r::ModelInstance* mi = currentObject->add_instance();
		currentObject->name = aCrgroup->m_objects[0].m_objectName;

		//currentObject->config.assign_config(config);
		for (const std::pair<std::string, std::string> pair : aCrgroup->m_settings->settings)
		{
			currentObject->config.set_key_value(pair.first, _set_key_value(pair.second, _def->get(pair.first)));
		}
		for (crslice2::CrObject aObject : aCrgroup->m_objects)
		{
			Slic3r::Transform3d t3d;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					t3d(i, j) = aObject.m_xform[i + j * 4];
				}

			}
			Slic3r::Geometry::Transformation t(t3d);
			mi->set_transformation(t);

			Slic3r::TriangleMesh mesh;
			trimesh2Slic3rTriangleMesh(aObject.m_mesh.get(), mesh);
			Slic3r::ModelVolume* v = currentObject->add_volume(mesh);
			currentObject->layer_height_profile.set(aObject.m_layerHeight);
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

	if ((int)scene->m_calibParams.mode > 0
		&& scene->m_calibParams.mode != crslice2::CalibMode::Calib_None)
	{
		_calibParams.start = scene->m_calibParams.start;
		_calibParams.end = scene->m_calibParams.end;
		_calibParams.step = scene->m_calibParams.step;
		_calibParams.print_numbers = scene->m_calibParams.print_numbers;
		_calibParams.mode = (Slic3r::CalibMode)scene->m_calibParams.mode;
	}

	//thumbnailData
	for (auto& thunm : scene->thumbnailDatas)
	{
		Slic3r::ThumbnailData data;
		data.height = thunm.height;
		data.width = thunm.width;
		data.pixels = thunm.pixels;
		data.pos_s = thunm.pos_s; //for cr_png
		data.pos_e = thunm.pos_e; //for cr_png
		data.pos_h = thunm.pos_h; //for cr_png
		thumbnailData.push_back(data);
	}

	//set multi plate param
	//plates_custom_gcodes;
	auto iter = scene->plates_custom_gcodes.begin();
	while (iter != scene->plates_custom_gcodes.end())
	{
		Slic3r::CustomGCode::Info info;
		info.mode = Slic3r::CustomGCode::Mode(iter->second.mode);
		for (auto& item : iter->second.gcodes)
		{
			Slic3r::CustomGCode::Item _item;
			_item.color = item.color;
			_item.print_z = item.print_z;
			_item.type = Slic3r::CustomGCode::Type(item.type);
			_item.extruder = item.extruder;
			_item.color = item.color;
			_item.extra = item.extra;
			info.gcodes.push_back(_item);
		}
		model.plates_custom_gcodes.emplace(iter->first, info);
		iter++;
	}
}


void slice_impl(const Slic3r::Model& model, const Slic3r::DynamicPrintConfig& config,
	bool is_bbl_printer, int plate_index, const Slic3r::Vec3d& plate_origin,
	const std::string& out, const std::string& out_json, Slic3r::Calib_Params& _calibParams, Slic3r::ThumbnailsList thumbnailDatas, ccglobal::Tracer* tracer)
{
#if 1
	if (!out_json.empty())
	{
		save_parameter_2_json(out_json + "cx_parameter.json", model, config);
	}
#endif

	Slic3r::PrintBase::status_callback_type callback = [&tracer](const Slic3r::PrintBase::SlicingStatus& _status) {
		if (tracer)
		{
			tracer->progress((float)_status.percent * 0.01);
			tracer->message(_status.text.c_str());

			if (tracer->interrupt())
			{
				throw Slic3r::SlicingError("User Cancelled", 0);
			}
		}
	};

	Slic3r::GCodeProcessorResult result;
	Slic3r::Print print;
	print.set_callback(callback);

	print.set_calib_params(_calibParams);
	print.apply(model, config);

	print.is_BBL_printer() = is_bbl_printer;
	print.set_plate_origin(plate_origin);

	print.set_plate_index(plate_index);

	Slic3r::StringObjectException warning;
	//BBS: refine seq-print logic
	Slic3r::Polygons polygons;
	std::vector<std::pair<Slic3r::Polygon, float>> height_polygons;
	print.validate(&warning, &polygons, &height_polygons);

	//BBS: reset the gcode before reload_print in slicing_completed event processing
	//FIX the gcode rename failed issue
	BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(" %1%: will start slicing, reset gcode_result firstly") % __LINE__;
	result.reset();

	BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(" %1%: gcode_result reseted, will start print::process") % __LINE__;

	try {
		print.process();
	}
	catch (const Slic3r::SlicingError& e1)
	{
		return _handle_slice_exception(print, e1.objectId(), e1.what(), tracer);
	}
	catch (const Slic3r::SlicingErrors& e2) {

		return  _handle_slice_exception(print, e2.errors_[0].objectId(), e2.errors_[0].what(), tracer);
	}

	BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(" %1%: after print::process, send slicing complete event to gui...") % __LINE__;

	auto g_Minus = [&](const Slic3r::ThumbnailsParams&) { return thumbnailDatas; };
	Slic3r::ThumbnailsGeneratorCallback thumbnail_cb = g_Minus;

	try
	{
		print.export_gcode(out, &result, thumbnail_cb);
	}
	catch (const std::exception& ex)
	{
		tracer->failed("export gcode failed@");
		return;
	}
	
	BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(": export gcode finished");
}

Slic3r::SlicingParameters getSliceParam(crslice2::SettingsPtr settings,  Slic3r::ModelObject* currentObject)
{
	Slic3r::DynamicPrintConfig config;
	const Slic3r::ConfigDef* _def = config.def();
	for (const std::pair<std::string, std::string> pair : settings->settings)
	{
		config.set_key_value(pair.first, _set_key_value(pair.second, _def->get(pair.first)));
	}

	return Slic3r::PrintObject::slicing_parameters(config, *currentObject, 0.f);
}

std::vector<double> orca_layer_height_profile_adaptive(crslice2::SettingsPtr settings, trimesh::TriMesh* triMesh, float quality)
{
	if (!triMesh)
		return std::vector<double>();

	Slic3r::TriangleMesh mesh;
	trimesh2Slic3rTriangleMesh(triMesh, mesh);
	Slic3r::Model model;
	Slic3r::ModelObject* currentObject = model.add_object();
	currentObject->add_instance();
	Slic3r::ModelVolume* v = currentObject->add_volume(mesh);

	Slic3r::SlicingParameters m_slicing_params = getSliceParam(settings, currentObject);
	return Slic3r::layer_height_profile_adaptive(m_slicing_params, *currentObject, quality);
}

std::vector<double> orca_smooth_height_profile(crslice2::SettingsPtr settings, trimesh::TriMesh* triMesh,
	const std::vector<double>& profile, unsigned int radius, bool keep_min)
{
	Slic3r::TriangleMesh mesh;
	trimesh2Slic3rTriangleMesh(triMesh, mesh);
	Slic3r::Model model;
	Slic3r::ModelObject* currentObject = model.add_object();
	currentObject->add_instance();
	Slic3r::ModelVolume* v = currentObject->add_volume(mesh);

	Slic3r::SlicingParameters m_slicing_params = getSliceParam(settings, currentObject);
	Slic3r::HeightProfileSmoothingParams smoothing_params_orca(radius, keep_min);

	return Slic3r::smooth_height_profile(profile, m_slicing_params, smoothing_params_orca);
}

std::vector<double> orca_generate_object_layers(crslice2::SettingsPtr settings, trimesh::TriMesh* triMesh,
	const std::vector<double>& profile)
{
	Slic3r::TriangleMesh mesh;
	trimesh2Slic3rTriangleMesh(triMesh, mesh);
	Slic3r::Model model;
	Slic3r::ModelObject* currentObject = model.add_object();
	currentObject->add_instance();
	Slic3r::ModelVolume* v = currentObject->add_volume(mesh);

	Slic3r::SlicingParameters m_slicing_params = getSliceParam(settings, currentObject);

	return Slic3r::generate_object_layers(m_slicing_params, profile);
}

void orca_slice_impl(crslice2::CrScenePtr scene, ccglobal::Tracer* tracer)
{
	if (!scene)
		return;

	Slic3r::Model model;
	Slic3r::DynamicPrintConfig config;
	Slic3r::Calib_Params calibParams;
	calibParams.end = 0.0;
	calibParams.start = 0.0;
	calibParams.step = 0.0;
	calibParams.print_numbers = false;


	Slic3r::ThumbnailsList thumbnailData;

	convert_scene_2_orca(scene, model, config, calibParams, thumbnailData);

	slice_impl(model, config, scene->m_isBBLPrinter, scene->m_plate_index, Slic3r::Vec3d(0.0, 0.0, 0.0), scene->m_gcodeFileName, scene->m_tempDirectory, calibParams, thumbnailData, tracer);
}

void orca_slice_fromfile_impl(const std::string& file, const std::string& out)
{
	std::ifstream in(file, std::ios::in | std::ios::binary);

	bool is_bbl_printer = false;
	int plate_index = 0;
	Slic3r::Vec3d plate_origin = Slic3r::Vec3d(0.0, 0.0, 0.0);
	Slic3r::Model model;
	Slic3r::DynamicPrintConfig config;

	std::string out_json = "cx_parameter.json";

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
	Slic3r::ThumbnailsList thumbnailDatas;
	slice_impl(model, config, is_bbl_printer, plate_index, plate_origin, out, out_json, calibParams, thumbnailDatas, nullptr);
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
					meta.options.push_back(crslice2::OptionValue(optDef.enum_values.at(i),
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

void export_metas_keys()
{
	using namespace Slic3r;
	using namespace nlohmann;

	{
		json j;
		std::vector<std::string> keys = Preset::printer_options();
		j["keys"] = json(keys);
		save_nlohmann_json("machine_keys.json", j);
	}

	{
		json j;
		std::vector<std::string> keys = Preset::print_options();
		j["keys"] = json(keys);
		save_nlohmann_json("profile_keys.json", j);
	}

	{
		json j;
		std::vector<std::string> keys = Preset::filament_options();
		j["keys"] = json(keys);
		save_nlohmann_json("material_keys.json", j);
	}

	{
		json j;
		std::vector<std::string> keys = print_config_def.extruder_option_keys();
		keys.insert(keys.end(), print_config_def.extruder_retract_keys().begin(), print_config_def.extruder_retract_keys().end());
		j["keys"] = json(keys);
		save_nlohmann_json("extruder_keys.json", j);
	}
}

void export_metas_impl()
{
	using namespace Slic3r;
	using namespace nlohmann;
	DynamicPrintConfig new_full_config;
	const ConfigDef* _def = new_full_config.def();
	{
		ordered_json j;
		std::vector<std::string> printer_keys = Preset::print_options();

		//record all the key-values
		for (const std::string& opt_key : _def->keys())
		{
			const ConfigOptionDef* optDef = _def->get(opt_key);
			if (!optDef || (optDef->printer_technology == Slic3r::ptSLA))
				continue;

			ordered_json item;
			Slic3r::ConfigOption* option = optDef->create_default_option();
			item["default_value"] = option->serialize();
			item["description"] = optDef->tooltip;
			item["enabled"] = "true";
			item["label"] = optDef->label;

			std::string type = "coNone";

			switch (optDef->type)
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


			if (optDef->enum_values.size() > 0)
			{
				size_t size = optDef->enum_values.size();
				ordered_json options;
				bool have = optDef->enum_labels.size() == optDef->enum_values.size();
				for (size_t i = 0; i < size; ++i)
				{
					options[optDef->enum_values.at(i)] = have ? optDef->enum_labels.at(i) : optDef->enum_values.at(i);
				}
				item["options"] = options;
			}

			bool is_print_key = std::find(printer_keys.begin(), printer_keys.end(), opt_key) != printer_keys.end();

			if (optDef->max != INT_MAX)
			{
				item["maximum_value"] = std::to_string(optDef->max);
			}
			if (optDef->min != INT_MIN)
			{
				item["minimum_value"] = std::to_string(optDef->min);
			}
			item["settable_globally"] = is_print_key ? "true" : "false";
			item["settable_per_extruder"] = is_print_key ? "true" : "false";
			item["settable_per_mesh"] = is_print_key ? "true" : "false";
			item["settable_per_meshgroup"] = "false";

			item["type"] = type;
			item["unit"] = optDef->sidetext;
			j[opt_key] = item;
		}

		save_nlohmann_json("fdm_machine_common.json", j);
	}

	export_metas_keys();
}


void _handle_slice_exception(const Slic3r::Print& print, size_t objectId, const char* failMsg, ccglobal::Tracer* tracer)
{
	std::string failStr;

	if (0 == objectId)
	{
		failStr = std::string(failMsg) + "@";
		tracer->failed(failStr.c_str());
		return;
	}

	Slic3r::ObjectID model_object_id(objectId);
	std::string modelObjectName = "";
	const Slic3r::ModelObject* mo = print.get_object(model_object_id)->model_object();
	if (nullptr != mo)
	{
		modelObjectName = mo->name;
	}

	failStr = std::string(failMsg) + "@" + modelObjectName;

	tracer->failed(failStr.c_str());
}
