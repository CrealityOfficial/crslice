# Copyright (c) 2022 Ultimaker B.V.
# CuraEngine is released under the terms of the AGPLv3 or higher.

__enable_cxx17()
include(StandardProjectSettings.cmake)

option(EXTENSIVE_WARNINGS "Build with all warnings" ON)
option(ENABLE_MORE_COMPILER_OPTIMIZATION_FLAGS "Enable more optimization flags" ON)
option(USE_SYSTEM_LIBS "Use the system libraries if available" OFF)

if (NOT APPLE)
    option(ENABLE_OPENMP "Use OpenMP for parallel code" ON)
endif ()

set(engine_SRCS # Except main.cpp.
        src/Application.cpp
        src/bridge.cpp
        src/ConicalOverhang.cpp
        src/ExtruderTrain.cpp
        src/FffGcodeWriter.cpp
        src/FffPolygonGenerator.cpp
        src/FffProcessor.cpp
        src/gcodeExport.cpp
        src/GCodePathConfig.cpp
        src/infill.cpp
        src/InsetOrderOptimizer.cpp
        src/layerPart.cpp
        src/LayerPlan.cpp
        src/LayerPlanBuffer.cpp
        src/mesh.cpp
        src/MeshGroup.cpp
        src/Mold.cpp
        src/multiVolumes.cpp
        src/PathOrderPath.cpp
        src/Preheat.cpp
        src/PrimeTower.cpp
        src/raft.cpp
        src/Scene.cpp
        src/SkeletalTrapezoidation.cpp
        src/SkeletalTrapezoidationGraph.cpp
        src/skin.cpp
        src/SkirtBrim.cpp
        src/SupportInfillPart.cpp
        src/Slice.cpp
        src/sliceDataStorage.cpp
        src/slicer.cpp
        src/support.cpp
        src/timeEstimate.cpp
        src/TopSurface.cpp
        src/TreeModelVolumes.cpp
        src/TreeSupport.cpp
        src/WallsComputation.cpp
        src/Weaver.cpp
        src/Wireframe2gcode.cpp
        src/WallToolPaths.cpp

        src/BeadingStrategy/BeadingStrategy.cpp
        src/BeadingStrategy/BeadingStrategyFactory.cpp
        src/BeadingStrategy/DistributedBeadingStrategy.cpp
        src/BeadingStrategy/LimitedBeadingStrategy.cpp
        src/BeadingStrategy/RedistributeBeadingStrategy.cpp
        src/BeadingStrategy/WideningBeadingStrategy.cpp
        src/BeadingStrategy/OuterWallInsetBeadingStrategy.cpp

        src/communication/ArcusCommunication.cpp
        src/communication/ArcusCommunicationPrivate.cpp
        src/communication/CommandLine.cpp
        src/communication/Listener.cpp

        src/infill/ImageBasedDensityProvider.cpp
        src/infill/NoZigZagConnectorProcessor.cpp
        src/infill/ZigzagConnectorProcessor.cpp
        src/infill/LightningDistanceField.cpp
        src/infill/LightningGenerator.cpp
        src/infill/LightningLayer.cpp
        src/infill/LightningTreeNode.cpp
        src/infill/SierpinskiFill.cpp
        src/infill/SierpinskiFillProvider.cpp
        src/infill/SubDivCube.cpp
        src/infill/GyroidInfill.cpp

        src/pathPlanning/Comb.cpp
        src/pathPlanning/GCodePath.cpp
        src/pathPlanning/LinePolygonsCrossings.cpp
        src/pathPlanning/NozzleTempInsert.cpp
        src/pathPlanning/TimeMaterialEstimates.cpp

        src/progress/Progress.cpp
        src/progress/ProgressStageEstimator.cpp

        src/settings/AdaptiveLayerHeights.cpp
        src/settings/FlowTempGraph.cpp
        src/settings/PathConfigStorage.cpp
        src/settings/Settings.cpp
        src/settings/ZSeamConfig.cpp

        src/utils/AABB.cpp
        src/utils/AABB3D.cpp
        src/utils/Date.cpp
        src/utils/ExtrusionJunction.cpp
        src/utils/ExtrusionLine.cpp
        src/utils/ExtrusionSegment.cpp
        src/utils/FMatrix4x3.cpp
        src/utils/gettime.cpp
        src/utils/LinearAlg2D.cpp
        src/utils/ListPolyIt.cpp
        src/utils/MinimumSpanningTree.cpp
        src/utils/Point3.cpp
        src/utils/PolygonConnector.cpp
        src/utils/PolygonsPointIndex.cpp
        src/utils/PolygonsSegmentIndex.cpp
        src/utils/polygonUtils.cpp
        src/utils/polygon.cpp
        src/utils/PolylineStitcher.cpp
        src/utils/ProximityPointLink.cpp
        src/utils/Simplify.cpp
        src/utils/SVG.cpp
        src/utils/socket.cpp
        src/utils/SquareGrid.cpp
        src/utils/ThreadPool.cpp
        src/utils/ToolpathVisualizer.cpp
        src/utils/VoronoiUtils.cpp
        )

__cc_find(RapidJSON)
__cc_find(boost)
__cc_find(stb)
__cc_find(fmt)
__cc_find(polyclipping)
#find_package(range-v3 REQUIRED)

set(SRCS ${engine_SRCS})
set(LIBS fmt rapidjson polyclipping stb
			 boost_system boost_filesystem boost_iostreams 
			)
			
if(CC_BC_WIN)
	list(APPEND LIBS Bcrypt)
endif()

set(INCS ${CMAKE_CURRENT_SOURCE_DIR}/include
		 ${CMAKE_CURRENT_BINARY_DIR}
		 )
		 
set(CVERSION "CURA_ENGINE_VERSION=\"5.2.0\"")
set(DEFS ${CVERSION} NOMINMAX ASSERT_INSANE_OUTPUT USE_CPU_TIME)

if (ENABLE_OPENMP)
    find_package(OpenMP REQUIRED)
    list(APPEND LIBS OpenMP::OpenMP_CXX)
endif ()

__add_real_target(_CuraEngine lib SOURCE ${SRCS}
								  LIB ${LIBS}
								  DEF ${DEFS}
								  INC ${INCS}
								  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
								  )
								  
use_threads(_CuraEngine)
enable_sanitizers(_CuraEngine)

if (${EXTENSIVE_WARNINGS})
    set_project_warnings(_CuraEngine)
endif ()


