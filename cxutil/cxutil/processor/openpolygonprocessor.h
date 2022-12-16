#ifndef CX_OPENPOLYGONPROCESSOR_1600242028483_H
#define CX_OPENPOLYGONPROCESSOR_1600242028483_H
#include "cxutil/math/polygon.h"

namespace cxutil
{
	void connectOpenPolygons(Polygons& openPolygons, Polygons& closedPolygons);

	void stitch(Polygons& openPolygons, Polygons& closedPolygons);
	void stitchExtensive(Polygons& openPolygons, Polygons& closedPolygons);
}

#endif // CX_OPENPOLYGONPROCESSOR_1600242028483_H