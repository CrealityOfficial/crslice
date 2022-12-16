/** Copyright (C) 2013 Ultimaker - Released under terms of the AGPLv3 License */
#include "gettime.h"

namespace cxutil
{

    TimeKeeper::TimeKeeper()
    {
        restart();
    }

    double TimeKeeper::restart()
    {
        double ret = getTime() - startTime;
        startTime = getTime();
        return ret;
    }

}//namespace cxutil