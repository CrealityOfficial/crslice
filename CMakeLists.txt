__enable_cxx17()
include(StandardProjectSettings.cmake)

if (NOT APPLE)
    option(ENABLE_OPENMP "Use OpenMP for parallel code" ON)
endif ()

set(ILIBS crcommon polyclipping)

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
		 
set(INCS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(DEFS)
set(INTERFACES ${CMAKE_CURRENT_SOURCE_DIR})

include(impl/cura5.0/cura.cmake)

__add_common_library(crslice)

use_threads(crslice)
enable_sanitizers(crslice)
								