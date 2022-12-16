//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef INFILL_ONEWALL_CONNECTOR_PROCESSOR_H
#define INFILL_ONEWALL_CONNECTOR_PROCESSOR_H

#include "ZigzagConnectorProcessor.h"

namespace cxutil
{

class Polygons;

/*!
 * This processor adds no connection. This is for line infill pattern.
 */
class OneWallConnectorProcessor : public ZigzagConnectorProcessor
{
public:
   OneWallConnectorProcessor(const PointMatrix& rotation_matrix, Polygons& result)
    : ZigzagConnectorProcessor(rotation_matrix, result,
        false, false, // settings for zig-zag end pieces, no use here
        false, 0) // settings for skipping some zags, no use here
    {
    }
    bool judgelinecrosspoly(const Point& intersection0, const Point& intersection1, Polygons& outline);
    void registerVertex(const Point& vertex);
    void registerScanlineSegmentIntersection(const Point& intersection_1, const Point& intersection0, const Point& intersection1, Polygons& outline, int scanline_index);
    void registerPolyFinished();
    void addZagConnector(std::vector<Point>& points, bool is_endpiece);
};


} // namespace CX


#endif // INFILL_NO_ZIGZAG_CONNECTOR_PROCESSOR_H
