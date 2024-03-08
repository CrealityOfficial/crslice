#ifndef ORCA_SLICE_WRAPPER_H
#define ORCA_SLICE_WRAPPER_H
#include "crslice2/crscene.h"
#include "crslice2/crslice.h"
#include "crslice2/base/parametermeta.h"
#include "libslic3r/Print.hpp"

void orca_slice_impl(crslice2::CrScenePtr scene, ccglobal::Tracer* tracer);
std::vector<double> orca_layer_height_profile_adaptive(crslice2::SliceParams& slicing_params, trimesh::TriMesh* triMesh, float quality);
void orca_slice_fromfile_impl(const std::string& file, const std::string& out);
void parse_metas_map_impl(crslice2::MetasMap& datas);
void get_meta_keys_impl(crslice2::MetaGroup metaGroup, std::vector<std::string>& keys);
void export_metas_impl();
void _handle_slice_exception(const Slic3r::Print& print, size_t objectId, const char* failMsg, ccglobal::Tracer* tracer);
#endif // 
