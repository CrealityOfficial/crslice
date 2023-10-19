#ifndef NARROW_INFILL274874
#define NARROW_INFILL274874
#include "Slice3rBase/ClipperUtils.hpp"
#include "utils/polygon.h"



Slic3r::ExPolygon convert(const cura52::Polygons& polygons);
bool result_is_narrow_infill_area(const cura52::Polygons& polygons);
static	bool	is_narrow_infill_area(const cura52::Polygons& polygons) ;

bool result_is_top_area(const cura52::Polygons& area,  cura52::Polygons& polygons);
static	bool	is_top_area(const cura52::Polygons& area, const cura52::Polygons& polygons);






#endif  