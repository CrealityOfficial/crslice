__enable_cxx17()
include(StandardProjectSettings.cmake)

__build_crslice_info_header()

if (NOT APPLE)
    option(ENABLE_OPENMP "Use OpenMP for parallel code" ON)
endif ()

#__cc_find(slice3rBase)

#add_definitions(-DG2G3_CONFIGURE_ENABLE)
#add_subdirectory(slice3rBase)

__cc_find(polyclipping)
__cc_find(range)
set(ILIBS crcommon polyclipping Slice3rBase mmesh range) 

set(SRCS crslice/interface.h
		 crslice/header.h
		 crslice/crslice.h
		 crslice/crscene.h
		 crslice/crcacheslice.h
		 
		 src/crslice.cpp
		 src/crcacheslice.cpp
		 src/crscene.cpp
		 src/crobject.h
		 src/crobject.cpp
		 src/crextruder.h
		 src/crextruder.cpp
		 src/crgroup.h
		 src/crgroup.cpp
		 src/crslicefromscene.h
		 src/crslicefromscene.cpp
		 src/cachedebugger.h
		 src/cachedebugger.cpp
		 src/create.h
		 src/create.cpp
		 src/conv.h
		 src/conv.cpp
		 )
		 
set(INCS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(DEFS BOOST_ALL_NO_LIB)
set(INTERFACES ${CMAKE_CURRENT_SOURCE_DIR})

include(impl/cura5.0/cura.cmake)

__add_common_library(crslice)
__set_folder_targets(slice TARGET crslice)

use_threads(crslice)
enable_sanitizers(crslice)
								