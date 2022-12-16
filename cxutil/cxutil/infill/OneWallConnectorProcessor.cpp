
//Copyright (c) 2017 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#include "cxutil/util/macros.h"

#include "OneWallConnectorProcessor.h"


namespace cxutil
{

    void OneWallConnectorProcessor::registerVertex(const Point& vertex)
    {
        //No need to add anything.
        if (is_first_connector)
        {
            first_connector.push_back(vertex);
            current_connector.push_back(vertex);
        }
        else
        { // it's yet unclear whether the polygon segment should be included, so we store it until we know
            current_connector.push_back(vertex);
        }
    }

    bool OneWallConnectorProcessor::judgelinecrosspoly(const Point& startPoint, const Point& endPoint, Polygons& outline)
    {
        Point diff = endPoint - startPoint;

        PointMatrix transformation_matrix = PointMatrix(diff);
        Point transformed_startPoint = transformation_matrix.apply(startPoint);
        Point transformed_endPoint = transformation_matrix.apply(endPoint);

        for (ConstPolygonRef poly : outline)
        {
            Point p0 = transformation_matrix.apply(poly.back());
            for (Point p1_ : poly)
            {
                Point p1 = transformation_matrix.apply(p1_);
                // when the boundary just touches the line don't disambiguate between the boundary moving on to actually cross the line
                // and the boundary bouncing back, resulting in not a real collision - to keep the algorithm simple.
                //
                // disregard overlapping line segments; probably the next or previous line segment is not overlapping, but will give a collision
                // when the boundary line segment fully overlaps with the line segment this edge case is not viewed as a collision
                if (p1.Y != p0.Y && ((p0.Y >= transformed_startPoint.Y && p1.Y <= transformed_startPoint.Y) || (p1.Y >= transformed_startPoint.Y && p0.Y <= transformed_startPoint.Y)))
                {
                    int64_t x = p0.X + (p1.X - p0.X) * (transformed_startPoint.Y - p0.Y) / (p1.Y - p0.Y);

                    if (x > transformed_startPoint.X + 100 && x < transformed_endPoint.X - 100)
                    {
                        return true;
                    }
                }
                p0 = p1;
            }
        }

        return false;
    }


    void OneWallConnectorProcessor::registerScanlineSegmentIntersection(const Point& intersection_1, const Point& intersection0, const Point& intersection1, Polygons& outline, int scanline_index)
    {

        //if (is_first_connector)
        //{
        //    // process as the first connector if we haven't found one yet
        //    // this will be processed with the last remaining piece at the end (when the polygon finishes)
        //    const bool is_this_endpiece = false;// scanline_index == last_connector_index;
        //    current_connector.push_back(intersection);
        //    //current_connector.push_back(intersection2);
        //    addZagConnector(current_connector, is_this_endpiece);  //Ã»ÓÐÕâ¸öÅµÅ®  5%infill µ¥±Ú  ²»Á¬½Ó   ÓÐÕâ¸ö£¬Æ¨¹É²úÉú¶àÓàÁ¬½Ó
        //    first_connector_end_scanline_index = scanline_index;
        //    is_first_connector = false;

        //}
        //else
        //{
        if (shouldAddCurrentConnector(last_connector_index, scanline_index))
        {
            int dis1 = int(sqrt(pow((intersection1.X - intersection_1.X), 2) + pow((intersection1.Y - intersection_1.Y), 2)));
            int dis2 = int(sqrt(pow((intersection0.X - intersection_1.X), 2) + pow((intersection0.Y - intersection_1.Y), 2)));


            current_connector.push_back(intersection_1);
            if (intersection1.Y - intersection0.Y > 2000)
            {
                if (!judgelinecrosspoly(intersection_1, intersection1, outline))
                {
                    if (dis1 > 20000)
                        current_connector.push_back(intersection1);
                }

            }
            else
            {
                if (!judgelinecrosspoly(intersection_1, intersection0, outline))
                {
                    if (dis2 > 20000)
                        current_connector.push_back(intersection0);
                }

            }
        }
        //}


        const bool is_this_endpiece = false;
        addZagConnector(current_connector, is_this_endpiece);
        current_connector.clear();


        // update state
        // we're starting a new (odd) zigzag connector, so clear the old one
        //current_connector.push_back(intersection);
        last_connector_index = scanline_index;

    }

    void OneWallConnectorProcessor::registerPolyFinished()
    {
        //No need to add anything.
        int scanline_start_index = last_connector_index;
        int scanline_end_index = first_connector_end_scanline_index;
        const bool is_endpiece = is_first_connector || (!is_first_connector && scanline_start_index == scanline_end_index);

        // decides whether to add this zag according to the following rules
        if ((is_endpiece && use_endpieces)
            || (!is_endpiece && shouldAddCurrentConnector(scanline_start_index, scanline_end_index)))
        {
            // for convenience, put every point in one vector
            for (const Point& point : first_connector)
            {
                current_connector.push_back(point);
            }
            first_connector.clear();

            addZagConnector(current_connector, is_endpiece);
        }

        // reset member variables
        reset();
    }

    void OneWallConnectorProcessor::addZagConnector(std::vector<Point>& points, bool is_endpiece)
    {
        // don't include the last line yet
        if (points.size() >= 3)
        {
            for (size_t point_idx = 1; point_idx <= points.size() - 2; ++point_idx)
            {
                addLine(points[point_idx - 1], points[point_idx]);
            }
        }
        // only add the last line if:
        //  - it is not an end piece, or
        //  - it is an end piece and "connected end pieces" is enabled
        if ((!is_endpiece || (is_endpiece && connected_endpieces)) && points.size() >= 2)
        {
            addLine(points[points.size() - 2], points[points.size() - 1]);
        }
    }


} // namespace CX 