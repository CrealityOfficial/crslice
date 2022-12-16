/** Copyright (C) 2017 Ultimaker - Released under terms of the AGPLv3 License */
#ifndef UTILS_MACROS_H
#define UTILS_MACROS_H

// macro to suppress unused parameter warnings from the compiler
#define UNUSED_PARAM(param) (void)(param)

#include <functional>

typedef std::function<void(float)> cxProgressFunc;
typedef std::function<bool()> cxInterruptFunc;
typedef std::function<void(const char*)> cxFailedFunc;

#endif // UTILS_MACROS_H
