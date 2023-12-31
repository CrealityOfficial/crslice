# Copyright (c) 2022 Ultimaker B.V.
# CuraEngine is released under the terms of the AGPLv3 or higher.
set(PREFIX5.2 impl/cura5.0/)
set(engine_SRCS # Except main.cpp.
        ${PREFIX5.2}src/Application.cpp
        ${PREFIX5.2}src/bridge.cpp
        ${PREFIX5.2}src/ConicalOverhang.cpp
        ${PREFIX5.2}src/ExtruderTrain.cpp
        ${PREFIX5.2}src/FffGcodeWriter.cpp
        ${PREFIX5.2}src/FffPolygonGenerator.cpp
        ${PREFIX5.2}src/FffProcessor.cpp
        ${PREFIX5.2}src/gcodeExport.cpp
        ${PREFIX5.2}src/GCodePathConfig.cpp
        ${PREFIX5.2}src/infill.cpp
        ${PREFIX5.2}src/InsetOrderOptimizer.cpp
        ${PREFIX5.2}src/layerPart.cpp
        ${PREFIX5.2}src/LayerPlan.cpp
        ${PREFIX5.2}src/LayerPlanBuffer.cpp
        ${PREFIX5.2}src/mesh.cpp
        ${PREFIX5.2}src/MeshGroup.cpp
        ${PREFIX5.2}src/Mold.cpp
        ${PREFIX5.2}src/multiVolumes.cpp
		${PREFIX5.2}src/narrow_infill.cpp
        ${PREFIX5.2}src/PathOrderPath.cpp
        ${PREFIX5.2}src/Preheat.cpp
        ${PREFIX5.2}src/PrimeTower.cpp
        ${PREFIX5.2}src/raft.cpp
        ${PREFIX5.2}src/Scene.cpp
        ${PREFIX5.2}src/SkeletalTrapezoidation.cpp
        ${PREFIX5.2}src/SkeletalTrapezoidationGraph.cpp
        ${PREFIX5.2}src/skin.cpp
        ${PREFIX5.2}src/SkirtBrim.cpp
        ${PREFIX5.2}src/SupportInfillPart.cpp
        ${PREFIX5.2}src/Slice.cpp
        ${PREFIX5.2}src/sliceDataStorage.cpp
        ${PREFIX5.2}src/slicer.cpp
        ${PREFIX5.2}src/support.cpp
        ${PREFIX5.2}src/timeEstimate.cpp
		${PREFIX5.2}src/timeestimateklipper.cpp
		${PREFIX5.2}src/timeestimatemarlin.cpp
        ${PREFIX5.2}src/TopSurface.cpp
		${PREFIX5.2}src/TreeModelVolumes.cpp
        ${PREFIX5.2}src/TreeSupport.cpp
		${PREFIX5.2}src/ThomasTreeModelVolumes.cpp
        ${PREFIX5.2}src/ThomasTreeSupport.cpp
        ${PREFIX5.2}src/WallsComputation.cpp
        ${PREFIX5.2}src/Weaver.cpp
        ${PREFIX5.2}src/Wireframe2gcode.cpp
        ${PREFIX5.2}src/WallToolPaths.cpp
        ${PREFIX5.2}src/BeadingStrategy/BeadingStrategy.cpp
        ${PREFIX5.2}src/BeadingStrategy/BeadingStrategyFactory.cpp
        ${PREFIX5.2}src/BeadingStrategy/DistributedBeadingStrategy.cpp
        ${PREFIX5.2}src/BeadingStrategy/LimitedBeadingStrategy.cpp
        ${PREFIX5.2}src/BeadingStrategy/RedistributeBeadingStrategy.cpp
        ${PREFIX5.2}src/BeadingStrategy/WideningBeadingStrategy.cpp
        ${PREFIX5.2}src/BeadingStrategy/OuterWallInsetBeadingStrategy.cpp
        ${PREFIX5.2}src/infill/ImageBasedDensityProvider.cpp
        ${PREFIX5.2}src/infill/NoZigZagConnectorProcessor.cpp
        ${PREFIX5.2}src/infill/ZigzagConnectorProcessor.cpp
        ${PREFIX5.2}src/infill/LightningDistanceField.cpp
        ${PREFIX5.2}src/infill/LightningGenerator.cpp
        ${PREFIX5.2}src/infill/LightningLayer.cpp
        ${PREFIX5.2}src/infill/LightningTreeNode.cpp
        ${PREFIX5.2}src/infill/SierpinskiFill.cpp
        ${PREFIX5.2}src/infill/SierpinskiFillProvider.cpp
        ${PREFIX5.2}src/infill/SubDivCube.cpp
        ${PREFIX5.2}src/infill/GyroidInfill.cpp
        ${PREFIX5.2}src/pathPlanning/Comb.cpp
        ${PREFIX5.2}src/pathPlanning/GCodePath.cpp
        ${PREFIX5.2}src/pathPlanning/LinePolygonsCrossings.cpp
        ${PREFIX5.2}src/pathPlanning/NozzleTempInsert.cpp
        ${PREFIX5.2}src/pathPlanning/TimeMaterialEstimates.cpp
        ${PREFIX5.2}src/progress/Progress.cpp
        ${PREFIX5.2}src/progress/ProgressStageEstimator.cpp
        ${PREFIX5.2}src/settings/AdaptiveLayerHeights.cpp
        ${PREFIX5.2}src/settings/FlowTempGraph.cpp
        ${PREFIX5.2}src/settings/PathConfigStorage.cpp
        ${PREFIX5.2}src/settings/Settings.cpp
        ${PREFIX5.2}src/settings/ZSeamConfig.cpp
        ${PREFIX5.2}src/utils/AABB.cpp
        ${PREFIX5.2}src/utils/AABB3D.cpp
        ${PREFIX5.2}src/utils/Date.cpp
        ${PREFIX5.2}src/utils/ExtrusionJunction.cpp
        ${PREFIX5.2}src/utils/ExtrusionLine.cpp
        ${PREFIX5.2}src/utils/ExtrusionSegment.cpp
        ${PREFIX5.2}src/utils/FMatrix4x3.cpp
        ${PREFIX5.2}src/utils/gettime.cpp
        ${PREFIX5.2}src/utils/LinearAlg2D.cpp
        ${PREFIX5.2}src/utils/ListPolyIt.cpp
        ${PREFIX5.2}src/utils/MinimumSpanningTree.cpp
        ${PREFIX5.2}src/utils/Point3.cpp
        ${PREFIX5.2}src/utils/PolygonConnector.cpp
        ${PREFIX5.2}src/utils/PolygonsPointIndex.cpp
        ${PREFIX5.2}src/utils/PolygonsSegmentIndex.cpp
        ${PREFIX5.2}src/utils/polygonUtils.cpp
        ${PREFIX5.2}src/utils/polygon.cpp
        ${PREFIX5.2}src/utils/PolylineStitcher.cpp
        ${PREFIX5.2}src/utils/ProximityPointLink.cpp
        ${PREFIX5.2}src/utils/Simplify.cpp
        ${PREFIX5.2}src/utils/SVG.cpp
        ${PREFIX5.2}src/utils/socket.cpp
        ${PREFIX5.2}src/utils/SquareGrid.cpp
        ${PREFIX5.2}src/utils/ThreadPool.cpp
        ${PREFIX5.2}src/utils/ToolpathVisualizer.cpp
        ${PREFIX5.2}src/utils/VoronoiUtils.cpp
		${PREFIX5.2}src/utils/VoronoiUtilsCgal.cpp
		${PREFIX5.2}src/utils/inputserial.cpp
		${PREFIX5.2}src/generateVaryingXYDisallowedArea.cpp

		${PREFIX5.2}src/TreeSupportT.cpp
		${PREFIX5.2}src/TreeSupportTipGenerator.cpp
        ${PREFIX5.2}src/TreeModelVolumesT.cpp

		${PREFIX5.2}include/progress/Progress.h
		${PREFIX5.2}include/progress/ProgressEstimator.h
		${PREFIX5.2}include/progress/ProgressEstimatorLinear.h
		${PREFIX5.2}include/progress/ProgressStageEstimator.h
		${PREFIX5.2}include/utils/VoronoiUtilsCgal.h
		${PREFIX5.2}include/TreeSupportElement.h
        ${PREFIX5.2}include/TreeSupportElementT.h
		${PREFIX5.2}include/TreeSupportSettings.h
        ${PREFIX5.2}include/TreeSupportSettingsT.h
		${PREFIX5.2}include/utils/inputserial.h
		${PREFIX5.2}include/debugger.h
        ${PREFIX5.2}include/TreeModelVolumesT.h
        ${PREFIX5.2}include/TreeModelVolumes.h



        ${PREFIX5.2}include/utils/section_type.h
        ${PREFIX5.2}include/utils/TreeSupportBaseCircle.h
        ${PREFIX5.2}include/utils/TreeSupportBaseCircleT.h
        ${PREFIX5.2}include/TreeSupportEnums.h
        ${PREFIX5.2}include/TreeSupportTipGenerator.h
        ${PREFIX5.2}include/TreeSupportUtils.h
        ${PREFIX5.2}include/TreeSupportT.h
        )

__cc_find(RapidJSON)
__cc_find(boost)
__cc_find(stb)
__cc_find(fmt)

list(APPEND SRCS ${engine_SRCS})
list(APPEND LIBS fmt rapidjson stb
			 boost_system boost_filesystem boost_iostreams 
			)
			
if(CC_BC_WIN)
	list(APPEND LIBS Bcrypt Ws2_32.lib)
endif()

list(APPEND INCS ${CMAKE_CURRENT_LIST_DIR}/include)
		 
set(CVERSION "CURA_ENGINE_VERSION=\"5.2.0\"")
list(APPEND DEFS ${CVERSION} NOMINMAX ASSERT_INSANE_OUTPUT USE_CPU_TIME)

if (ENABLE_OPENMP)
	#find_package(OpenMP REQUIRED)
	#list(APPEND LIBS OpenMP::OpenMP_CXX)
endif ()
								  



