__enable_cxx17()
include(StandardProjectSettings.cmake)

if (NOT APPLE)
    option(ENABLE_OPENMP "Use OpenMP for parallel code" ON)
endif ()

option(G2G3_ENABLE "enable G2/G3 gcode" ON)
if(G2G3_ENABLE)
	add_subdirectory(slice3rBase)
endif()

add_subdirectory(impl/cura5.0)
add_subdirectory(cxutil)
add_subdirectory(impl/cura_C3D/cura-cloud)

set(SRCS crslice/interface.h
		 crslice/header.h
		 crslice/crslice.h
		 crslice/crscene.h
		 
		 src/crslice.cpp
		 src/crscene.cpp
		 src/crobject.h
		 src/crobject.cpp
		 src/crextruder.h
		 src/crextruder.cpp
		 src/crgroup.h
		 src/crgroup.cpp
		 src/crslicefromscene.h
		 src/crslicefromscene.cpp
		 )
		 
set(ILIBS crcommon polyclipping  CuraEngine520 CuraEngine480 cxutil)
		 
set(INCS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(DEFS)
set(INTERFACES ${CMAKE_CURRENT_SOURCE_DIR})

__add_common_library(crslice)

use_threads(crslice)
enable_sanitizers(crslice)
								