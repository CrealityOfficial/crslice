#ifndef CXUTIL_SPACE_FILL_TYPE
#define CXUTIL_SPACE_FILL_TYPE

namespace cxutil
{
    /*!
     * Enum class enumerating the strategies with which an area can be occupied with filament
     *
     * The walls/perimeters are Polygons
     * ZigZag infill is PolyLines, and so is following mesh surface mode for non-polygon surfaces
     * Grid, Triangles and lines infill is Lines
     */
    enum class SpaceFillType
    {
        None,
        Polygons,
        PolyLines,
        Lines
    };

} // namespace cxutil

#endif // CXUTIL_SPACE_FILL_TYPE