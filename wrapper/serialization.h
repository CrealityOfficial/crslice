#ifndef DEBUG_CACHE_SERIALIZATION_H
#define DEBUG_CACHE_SERIALIZATION_H
#include "libslic3r/Print.hpp"

void cache_slice_scene(const Slic3r::Print& print, const Slic3r::Model& model, const Slic3r::DynamicPrintConfig& config, const std::string& file_name);

#endif // DEBUG_CACHE_SERIALIZATION_H
