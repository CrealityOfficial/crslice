__enable_cxx17()
include(StandardProjectSettings.cmake)

if (NOT APPLE)
    option(ENABLE_OPENMP "Use OpenMP for parallel code" ON)
endif ()

set(ILIBS crcommon polyclipping)

set(SRCS crslice/interface.h
		 crslice/header.h
		 crslice/slice.h
		 crslice/slice.cpp
		 crslice/scene.h
		 crslice/scene.cpp
		 )
		 
set(INCS)
set(DEFS)
set(INTERFACES ${CMAKE_CURRENT_SOURCE_DIR})

include(impl/cura5.0/cura.cmake)

__add_common_library(crslice)

use_threads(crslice)
enable_sanitizers(crslice)
								