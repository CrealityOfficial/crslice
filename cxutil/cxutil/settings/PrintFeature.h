#ifndef CXUTIL_PRINT_FEATURE
#define CXUTIL_PRINT_FEATURE

namespace cxutil
{
    enum class PrintFeatureType : unsigned char
    {
        NoneType = 0, // used to mark unspecified jumps in polygons. libArcus depends on it
        OuterWall = 1,
        InnerWall = 2,
        Skin = 3,
        Support = 4,
        SkirtBrim = 5,
        Infill = 6,
        SupportInfill = 7,
        MoveCombing = 8,
        MoveRetraction = 9,
        SupportInterface = 10,
        PrimeTower = 11,
		SlowFlowTypes = 12,
		FlowAdanceTypes = 13,
        NumPrintFeatureTypes = 14, // this number MUST be the last one because other modules will
                                  // use this symbol to get the total number of types, which can
                                  // be used to create an array or so
        SwitchExtruderType =15
    };
} // namespace cxutil

#endif // CXUTIL_PRINT_FEATURE
