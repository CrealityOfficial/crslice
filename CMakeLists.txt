__enable_cxx17()
include(StandardProjectSettings.cmake)

__build_crslice_info_header()

__assert_target(slice3r_base)

__cc_find(trimesh2)
__assert_target(trimesh2)

__cc_find(polyclipping)
__assert_target(polyclipping)

set(ILIBS trimesh2) 
set(LIBS polyclipping slice3r_base) 

__files_group(${CMAKE_CURRENT_SOURCE_DIR}/crslice/ SRCS1)
__files_group(${CMAKE_CURRENT_SOURCE_DIR}/src/ SRCS2)

set(SRCS ${SRCS1} ${SRCS2})		 
set(INCS ${CMAKE_CURRENT_SOURCE_DIR}
		 ${CMAKE_CURRENT_SOURCE_DIR}/src
		 )
set(DEFS BOOST_ALL_NO_LIB)
set(INTERFACES ${CMAKE_CURRENT_SOURCE_DIR})

include(impl/cura5.0/cura.cmake)

__add_common_library(crslice FOLDER slice OPENMP)

use_threads(crslice)
enable_sanitizers(crslice)
								