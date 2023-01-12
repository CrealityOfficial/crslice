#include "Slice3rBase/ClipperUtils.hpp"

#include "narrow_infill.h"



#define CLIPPER_OFFSET_SHORTEST_EDGE_FACTOR (0.005f)

Slic3r::ExPolygon convert(const cura52::Polygons& polygons)
{
    Slic3r::Polygons Sli3crpolygons;
    Slic3r::Points points_in;
    Slic3r::Point point_in;
    for (int i = 0; i < polygons.size(); i++)
    {
        ClipperLib::Path path = *(polygons.begin() + i);
        for (ClipperLib::IntPoint p : path)
        {
            point_in.x() = p.X * 1000;
            point_in.y() = p.Y * 1000;
            points_in.emplace_back(point_in);
        }
    }

    Slic3r::ExPolygon expolygon;
    expolygon.contour.append(points_in);
    return expolygon;
}

Slic3r::ClipperLib::Paths  Polygons2clipperpaths(Slic3r::Polygons& out)
{
    Slic3r::ClipperLib::Path out2path;
    Slic3r::ClipperLib::Paths out2paths;
    for (int i = 0; i < out.size(); i++)
    {
        Slic3r::Polygon path = out.at(i);
        Slic3r::ClipperLib::IntPoint pointcli;

        for (int j = 0; j < path.size(); j++)
        {
            pointcli.x() = path[j].x();
            pointcli.y() = path[j].y();
            out2path.emplace_back(pointcli);
        }
        out2paths.emplace_back(out2path);
        out2path.clear();
    }
    return out2paths;
}
bool result_is_top_area(const cura52::Polygons& area,  cura52::Polygons& polygons)
{
    return  is_top_area(area, polygons);
}
static bool is_top_area(const cura52::Polygons& area, const cura52::Polygons& polygons)
{
    Slic3r::ExPolygon expolygon;
    expolygon = convert(polygons);
    Slic3r::ExPolygon areaex;
    areaex = convert(area);
    Slic3r::ExPolygons result =  Slic3r::diff_ex(areaex, expolygon, Slic3r::ApplySafetyOffset::Yes);
    if (result.empty())
        return false;

    return true;
}

bool result_is_narrow_infill_area(const cura52::Polygons& polygons) 
{
    return  is_narrow_infill_area( polygons);
}

static bool is_narrow_infill_area(const cura52::Polygons& polygons)
{

    Slic3r::ExPolygon expolygon;
    expolygon = convert(polygons);

    const float delta = -3000000.0;
    double miterLimit = 3.000;
 
    Slic3r::ClipperLib::JoinType joinType = Slic3r::ClipperLib::JoinType::jtMiter;;
    Slic3r::Polygons  out;
    out =  Slic3r::offset(expolygon, delta, joinType, miterLimit);

    bool fillType = false;
    
    Slic3r::ClipperLib::Paths out2paths;
    out2paths = Polygons2clipperpaths(out);

    Slic3r::ExPolygons result;
    result =  Slic3r::ClipperPaths_to_Slic3rExPolygons(out2paths, fillType );

    if (result.empty())
        return true;

    return false;
   
}
