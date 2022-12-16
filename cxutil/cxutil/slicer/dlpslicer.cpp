#include "cxutil/slicer/dlpslicer.h"
#include "cxutil/input/dlpinput.h"

#include "cxutil/slicer/slicedmesh.h"
#include "cxutil/slicer/meshslice.h"
#include "cxutil/slicer/preslice.h"

#include "cxutil/processor/openpolygonprocessor.h"
#include "cxutil/util/slicetracer.h"
#include "trimesh2/TriMesh.h"

#include <assert.h>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace cxutil
{
	DLPSlicer::DLPSlicer()
	{

	}

	DLPSlicer::~DLPSlicer()
	{

	}

	DLPData* DLPSlicer::compute(DLPInput* input, SliceTracer* tracer)
	{
		if (!input)
			return nullptr;
#ifndef USE_TRIMESH_SLICE
		std::vector<MeshObjectPtr>& meshptres = input->meshes();
		size_t meshCount = meshptres.size();
		if (meshCount == 0)
			return nullptr;
#endif

		std::vector<int> z;
		buildSliceInfos(input, z);

		size_t layerCount = z.size();
		if (layerCount == 0)
			return nullptr;

		if (tracer)
		{
			tracer->traceProgress(0.3f);
			if (tracer->traceInterrupt())
			{
				tracer->traceProgress(1.0f);
				return nullptr;
			}
		}

#ifdef _OPENMP
		omp_set_num_threads(omp_get_num_procs());
#endif
#ifndef USE_TRIMESH_SLICE
		std::vector<MeshObject*> meshes;
		for (size_t i = 0; i < meshCount; ++i)
		{
			meshes.push_back(&*meshptres.at(i));
		}
		std::vector<SlicedMesh> slicedMeshes(meshCount);
		sliceMeshes(meshes, slicedMeshes, z);
#else
		std::vector<trimesh::TriMesh*> meshes_src = input->getMeshesSrc();
		size_t meshCount = meshes_src.size();
		std::vector<SlicedMesh> slicedMeshes(meshCount);
		sliceMeshes_src(meshes_src, slicedMeshes, z);
#endif

		if (tracer)
		{
			tracer->traceProgress(0.6f);
			if (tracer->traceInterrupt())
			{
				tracer->traceProgress(1.0f);
				return nullptr;
			}
		}
		DLPParam& param = input->param();
		//////////
		for (size_t i = 0; i < meshCount; ++i)
		{
			SlicedMesh& slicedMesh = slicedMeshes.at(i);
#pragma omp parallel for
			for (int j = 0; j < (int)layerCount; ++j)
			{
				SlicedMeshLayer& layer = slicedMesh.m_layers.at(j);

				Polygons closedPolygons;
				connectOpenPolygons(layer.openPolylines, closedPolygons);

				for (size_t k = 0; k < closedPolygons.size(); ++k)
				{
					layer.polygons.add(closedPolygons[k]);
				}

				{ 
					Polygons stitchClosedPolygons;
					stitch(layer.openPolylines, stitchClosedPolygons);

					for (size_t k = 0; k < stitchClosedPolygons.size(); ++k)
					{
						layer.polygons.add(stitchClosedPolygons[k]);
					}
				}

				if (false)
				{
					stitchExtensive(layer.openPolylines, layer.polygons);
				}

				if (false)
				{
					for (PolygonRef polyline : layer.openPolylines)
					{
						if (polyline.size() > 0)
							layer.polygons.add(polyline);
					}
				}

				Polygons resultPolygons;
				for (PolygonRef polyline : layer.openPolylines)
				{
					if (polyline.size() > 0)
					{
						resultPolygons.add(polyline);
					}
				}
				layer.openPolylines = resultPolygons;
			}
		}

		if (tracer)
		{
			tracer->traceProgress(0.7f);
			if (tracer->traceInterrupt())
			{
				tracer->traceProgress(1.0f);
				return nullptr;
			}
		}

		for (size_t i = 0; i < meshCount; ++i)
		{
			SlicedMesh& slicedMesh = slicedMeshes.at(i);

#pragma omp parallel for
			for (int j = 0; j < (int)layerCount; ++j)
			{
				SlicedMeshLayer& layer = slicedMesh.m_layers.at(j);
				//const coord_t snap_distance = std::max(param.minimum_polygon_circumference, static_cast<coord_t>(1));
				//auto it = std::remove_if(layer.polygons.begin(), layer.polygons.end(), [snap_distance](PolygonRef poly) {
				//	return poly.shorterThan(snap_distance);
				//	});
				//layer.polygons.erase(it, layer.polygons.end());

				//Finally optimize all the polygons. Every point removed saves time in the long run.
				const coord_t line_segment_resolution = param.line_segment_resolution;
				const coord_t line_segment_deviation = param.line_segment_deviation;
				layer.polygons.simplify(line_segment_resolution, line_segment_deviation);
				layer.polygons.removeDegenerateVerts(); // remove verts connected to overlapping line segments

				float xy_offset = input->param().xy_offset;
				bool enable_xy_offset = input->param().enable_xy_offset;

				if (enable_xy_offset && abs(xy_offset) > 0.000001)
				{
					layer.polygons = layer.polygons.offset((int)(xy_offset * 1000));
				}
			}
		}

		if (tracer)
		{
			tracer->traceProgress(0.9f);
			if (tracer->traceInterrupt())
			{
				tracer->traceProgress(1.0f);
				return nullptr;
			}
		}

		DLPData* data = new DLPData();
		data->m_dlpmeshsgroup.dlpmeshsgroup.resize(1);

		DLPmeshs& dataMeshes = data->m_dlpmeshsgroup.dlpmeshsgroup.at(0);
		dataMeshes.dlpmeshs.resize(meshCount);
		for (size_t meshIdx = 0; meshIdx < meshCount; meshIdx++)
		{
			SlicedMesh& slicedMesh = slicedMeshes.at(meshIdx);
			DLPmesh& meshData = dataMeshes.dlpmeshs.at(meshIdx);
			meshData.layers.resize(layerCount);

#pragma omp parallel for
			for (int layer_nr = 0; layer_nr < static_cast<int>(layerCount); layer_nr++)
			{
				SlicedMeshLayer& layer = slicedMesh.m_layers.at(layer_nr);

				DLPLayer& dlpplayer = meshData.layers.at(layer_nr);
				dlpplayer.printZ = z.at(layer_nr);
				dlpplayer.parts.push_back((layer.polygons.splitIntoPolyTree(true)));
				layer.polygons.deletePaths();
			}
		}

		if (tracer)
		{
			tracer->traceProgress(1.0f);
			if (tracer->traceInterrupt())
			{
				delete data;
				data = nullptr;
			}
		}
		return data;
	}
}