
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(WIN32)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

__cc_find(Eigen)
__cc_find(Boost)
__cc_find(Cereal)

set(SRCS	Slice3rBase/Int128.hpp
			Slice3rBase/libslic3r.h
			Slice3rBase/Point.cpp
			Slice3rBase/Point.hpp
			Slice3rBase/Polygon.cpp
			Slice3rBase/Polygon.hpp
			Slice3rBase/Polyline.cpp
			Slice3rBase/Polyline.hpp
			Slice3rBase/Line.cpp
			Slice3rBase/Line.hpp
			Slice3rBase/LocalesUtils.cpp
			Slice3rBase/LocalesUtils.hpp
			Slice3rBase/MultiPoint.cpp
			Slice3rBase/MultiPoint.hpp
			Slice3rBase/BoundingBox.cpp
			Slice3rBase/BoundingBox.hpp
			Slice3rBase/Geometry.cpp
			Slice3rBase/Geometry.hpp
			Slice3rBase/ExPolygon.cpp
			Slice3rBase/ExPolygon.hpp
			Slice3rBase/clipper.cpp
			Slice3rBase/clipper.hpp
			Slice3rBase/ClipperUtils.cpp
			Slice3rBase/ClipperUtils.hpp
			Slice3rBase/Surface.cpp
			Slice3rBase/Surface.hpp
			
			Slice3rBase/fast_float/fast_float.h
			
			
			Slice3rBase/Circle.hpp
			Slice3rBase/Circle.cpp
			Slice3rBase/ArcFitter.hpp
			Slice3rBase/ArcFitter.cpp
			
			Slice3rBase/Geometry/MedialAxis.hpp
			Slice3rBase/Geometry/MedialAxis.cpp
			
			Slice3rBase/clipper/clipper_z.cpp
			Slice3rBase/clipper/clipper_z.hpp
			)
			
set(ILIBS boost_system boost_filesystem boost_iostreams eigen cereal)
			
set(INCS)
set(DEFS)
set(INTERFACES)

set(SLICE3RBASE_STATIC ON)
__add_common_library(slice3rBase)

