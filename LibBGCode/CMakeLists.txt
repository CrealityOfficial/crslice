
set(HEAD_INCLUDES)
__add_real_target(heatshrink lib 
				SOURCE heatshrink/heatshrink_decoder.c
						heatshrink/heatshrink_encoder.c
				INC ${HEAD_INCLUDES}
				INTERFACE_DEF HEATSHRINK_DYNAMIC_ALLOC=0
				)
__add_real_target(heatshrink_dynalloc lib 
				SOURCE heatshrink/heatshrink_decoder.c
						heatshrink/heatshrink_encoder.c
				INC ${HEAD_INCLUDES}
				INTERFACE_DEF HEATSHRINK_DYNAMIC_ALLOC=1
				)

set(INCS ${CMAKE_CURRENT_SOURCE_DIR}/../
		${CMAKE_CURRENT_SOURCE_DIR}/heatshrink/
		)
set(INTERFACES ${CMAKE_CURRENT_SOURCE_DIR}/../)

__cc_find(zlib_static)
__assert_target(zlib_static)
set(SRCS 
    binarize/binarize.cpp
    binarize/binarize.hpp
    binarize/meatpack.cpp
    binarize/meatpack.hpp
	)
	
set(ILIBS heatshrink heatshrink_dynalloc zlib_static)
__add_real_target(libBGCode lib SOURCE ${SRCS}
				INC ${INCS}
				ILIB ${ILIBS}
				INTERFACE ${INTERFACES}
				)