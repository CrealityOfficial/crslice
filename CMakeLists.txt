__cc_find(Clipper)

set(SRCS crslice/interface.h
		 crslice/slice.h
		 crslice/slice.cpp
		 crslice/Settings.h
		 crslice/Settings.cpp
		 )

add_subdirectory(impl)

set(LIBS _CuraEngine)

__add_common_library(crslice)

__set_folder_targets(crslice TARGET crslice _CuraEngine)
								