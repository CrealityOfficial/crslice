#ifndef SLICE_SLICEHELPER_1598712992630_H
#define SLICE_SLICEHELPER_1598712992630_H
#include "cxutil/input/meshobject.h"
#include "cxutil/math/point2.h"
#include <unordered_map>

namespace trimesh {
	class TriMesh;
}

namespace cxutil
{
	class SliceHelper
	{
	public:
		SliceHelper();
		~SliceHelper();

		std::vector<MeshFace> faces;
		std::vector<std::vector<uint32_t>> vertexConnectFaceData;
		trimesh::TriMesh* getMeshSrc();
		void prepare(MeshObject* mesh);
		void prepare(trimesh::TriMesh* _mesh);
		void getMeshFace();
		std::vector<Point2>* getFaceRanges();
		void sliceOneLayer(int z,
			std::vector<SlicerSegment>& segments, std::unordered_map<int, int>& face_idx_to_segment_idx);
		
		static SlicerSegment project2D(const Point3& p0, const Point3& p1, const Point3& p2, const coord_t z);
		static void buildMeshFaceHeightsRange(const MeshObject* mesh, std::vector<Point2>& heightRanges);
		static void buildMeshFaceHeightsRange(const trimesh::TriMesh* meshSrc, std::vector<Point2>& heightRanges);
	protected:
		MeshObject* mesh;
		trimesh::TriMesh* meshSrc;
		std::vector<Point2> faceRanges;
	};
}

#endif // SLICE_SLICEHELPER_1598712992630_H