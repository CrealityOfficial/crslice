#ifndef ORCA_SLICE_WRAPPER_H
#define ORCA_SLICE_WRAPPER_H
#include "crslice2/crscene.h"
#include "crslice2/crslice.h"
#include "crslice2/base/parametermeta.h"
#include "libslic3r/Print.hpp"

void orca_slice_impl(crslice2::CrScenePtr scene, ccglobal::Tracer* tracer, Slic3r::GCodeProcessorResult* outResult);

struct OrcaResult
{
	Slic3r::GCodeProcessorResult gcode_result;
	Slic3r::StringObjectException warning;
	Slic3r::Polygons polygons;
	std::vector<std::pair<Slic3r::Polygon, float>> height_polygons;
};

void orca_slice_impl(Slic3r::Print& print, Slic3r::Model& model, Slic3r::DynamicPrintConfig& config, const std::string& out_file
	, Slic3r::ThumbnailsGeneratorCallback callback, Slic3r::Calib_Params& calib_params   
	, OrcaResult& result, ccglobal::Tracer* tracer);

void convert_scene_2_orca(crslice2::CrScenePtr scene, Slic3r::Model& model, Slic3r::DynamicPrintConfig& config, Slic3r::Calib_Params& _calibParams, Slic3r::ThumbnailsList& thumbnailData);
bool detect_multi_color_slice(const Slic3r::DynamicPrintConfig& config, const Slic3r::Model& model, ccglobal::Tracer* tracer);

std::vector<double> orca_layer_height_profile_adaptive(crslice2::SettingsPtr settings, std::vector<TriMeshPtr> triMesh, float quality);
std::vector<double> orca_smooth_height_profile(crslice2::SettingsPtr settings, std::vector<TriMeshPtr> triMesh,
	const std::vector<double>& profile, unsigned int radius, bool keep_min);
std::vector<double> orca_generate_object_layers(crslice2::SettingsPtr settings, std::vector<TriMeshPtr> triMesh,
	const std::vector<double>& profile);
std::vector<double> orca_update_layer_height_profile(crslice2::SettingsPtr settings, std::vector<TriMeshPtr> triMesh,
	const std::vector<double>& profile);
void orca_slice_fromfile_impl(const std::string& file, const std::string& out, ccglobal::Tracer* tracer = nullptr);
void parse_metas_map_impl(crslice2::MetasMap& datas);
void get_meta_keys_impl(crslice2::MetaGroup metaGroup, std::vector<std::string>& keys);
void export_metas_impl();
void _handle_slice_exception(const Slic3r::Print& print, size_t objectId, const char* failMsg, ccglobal::Tracer* tracer);
#endif // 
