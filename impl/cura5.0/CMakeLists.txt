__enable_cxx17()

if (NOT APPLE)
    option(ENABLE_OPENMP "Use OpenMP for parallel code" ON)
endif ()


set(ILIBS crcommon polyclipping cxutil)

set(SRCS )	 
set(INCS ${CMAKE_CURRENT_SOURCE_DIR})
set(DEFS)
set(INTERFACES ${CMAKE_CURRENT_SOURCE_DIR}/../)

include(cura.cmake)

if(G2G3_ENABLE)
	add_definitions(-DG2G3_CONFIGURE_ENABLE)
	list(APPEND ILIBS slice3rBase)
endif()
set(CURAENGINE520_STATIC ON)
	
__add_common_library(CuraEngine520)

use_threads(CuraEngine520)
enable_sanitizers(CuraEngine520)
								