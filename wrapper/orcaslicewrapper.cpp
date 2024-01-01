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

void convert_scene_2_orca(CrScenePtr scene, Slic3r::Model& model, Slic3r::DynamicPrintConfig& config)
{

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

void orca_slice_impl(CrScenePtr scene, ccglobal::Tracer* tracer)
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
