__files_group(${CMAKE_CURRENT_SOURCE_DIR} SRC)
source_group(ext FILES ${SRC})
__files_group(${CMAKE_CURRENT_SOURCE_DIR}/cura/ CURA_SRC)
#__cc_find(Clipper)
__cc_find(RapidJson)

set(ENGINE_INCS ${CMAKE_CURRENT_SOURCE_DIR} ${STB_INCLUDE_DIRS} ${RAPIDJSON_INCLUDE_DIRS} )
set(INCS ${ENGINE_INCS})
set(LIBS polyclipping cxutil)
set(INTERFACES ${CMAKE_CURRENT_SOURCE_DIR}/../
	)

set(DEFS _CRT_SECURE_NO_WARNINGS NOMINMAX)

__add_real_target(CuraEngine480 lib SOURCE ${SRC} ${CURA_SRC}
								 LIB ${LIBS}
								 INC ${INCS}
								 DEF ${DEFS}
								 INTERFACE ${INTERFACES}
								 OPENMP
								 )



