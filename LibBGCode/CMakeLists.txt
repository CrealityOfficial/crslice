
set(HEAD_INCLUDES)
__add_real_target(heatshrink lib 
				SOURCE heatshrink/heatshrink_decoder.c
						heatshrink/heatshrink_encoder.c
				INC ${HEAD_INCLUDES}
				INTERFACE_DEF HEATSHRINK_DYNAMIC_ALLOC=0
				FOLDER slice
				)
__add_real_target(heatshrink_dynalloc lib 
				SOURCE heatshrink/heatshrink_decoder.c
						heatshrink/heatshrink_encoder.c
				INC ${HEAD_INCLUDES}
				INTERFACE_DEF HEATSHRINK_DYNAMIC_ALLOC=1
				FOLDER slice
				)

set(INCS ${CMAKE_CURRENT_SOURCE_DIR}/../
		${CMAKE_CURRENT_SOURCE_DIR}/heatshrink/
		)
set(INTERFACES ${CMAKE_CURRENT_SOURCE_DIR}/../)

__cc_find(zlib_static)
__assert_target(zlib_static)
__cc_find(boost_header)

set(SRCS 
    binarize/binarize.cpp
    binarize/binarize.hpp
    binarize/meatpack.cpp
    binarize/meatpack.hpp
	binarize/export.h
	core/core.cpp
	core/export.h
	convert/convert.cpp
	convert/export.h
	)
	
set(ILIBS heatshrink heatshrink_dynalloc zlib_static boost_header)
__add_real_target(libBGCode lib SOURCE ${SRCS}
				INC ${INCS}
				ILIB ${ILIBS}
				DEF LibBGCode_VERSION="1.2.0"
				INTERFACE ${INTERFACES}
				FOLDER slice
				)