//Copyright (c) 2020 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef String_List_GRAPH
#define String_List_GRAPH

#include <cassert>
#include <vector>

namespace cura52
{

/*!
 * Class representing a graph matching a flow to a temperature.
 * The graph generally consists of several linear line segments between points at which the temperature and flow are matched.
 */
class   LimitGraph
{
public:
    struct LimitData
    {
        double value1;
        double value2;
        double speed1;
        double speed2; 
        double Acc1;
        double Acc2;
        double Temp1; 
        double Temp2;
    };

    LimitData data; 
};

} // namespace cura52

#endif // String_List_GRAPH