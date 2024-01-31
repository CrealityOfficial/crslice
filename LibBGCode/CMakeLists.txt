__add_real_target(heatshrink lib 
				SOURCE heatshrink_decoder.c heatshrink_encoder.c
				INTERFACE_DEF HEATSHRINK_DYNAMIC_ALLOC=0
				)
__add_real_target(heatshrink_dynalloc lib 
				SOURCE heatshrink_decoder.c heatshrink_encoder.c
				INTERFACE_DEF HEATSHRINK_DYNAMIC_ALLOC=1
				)

set(INCS ${CMAKE_CURRENT_SOURCE_DIR}/../
		${CMAKE_CURRENT_SOURCE_DIR}/heatshrink/
		)
set(INTERFACES ${CMAKE_CURRENT_SOURCE_DIR}/../)

__add_real_target(libBGCode lib SOURCE ${SRCS}
				INC ${INCS}
				INTERFACE ${INTERFACES}
				)