#ifndef ORCA_SLICE_WRAPPER_H
#define ORCA_SLICE_WRAPPER_H
#include "crslice/crscene.h"

void orca_slice_impl(CrScenePtr scene, ccglobal::Tracer* tracer);
void orca_slice_fromfile_impl(const std::string& file, const std::string& out);
#endif // 
