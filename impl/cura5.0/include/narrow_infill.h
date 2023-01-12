#ifndef NARROW_INFILL274874
#define NARROW_INFILL274874

#include "utils/polygon.h"




bool result_is_narrow_infill_area(const cura52::Polygons& polygons);
static	bool	is_narrow_infill_area(const cura52::Polygons& polygons) ;

bool result_is_top_area(const cura52::Polygons& area,  cura52::Polygons& polygons);
static	bool	is_top_area(const cura52::Polygons& area, const cura52::Polygons& polygons);






#endif  