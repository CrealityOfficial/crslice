#include "cxutil/slicer/slicehelper.h"
#include "cxutil/math/linearAlg2D.h"
#include "trimesh2/TriMesh.h"

namespace cxutil
{
	SliceHelper::SliceHelper()
		:mesh(nullptr)
	{

	}

	SliceHelper::~SliceHelper()
	{

	}

	void SliceHelper::prepare(MeshObject* _mesh)
	{
		mesh = _mesh;
		buildMeshFaceHeightsRange(mesh, faceRanges);
	}

	void SliceHelper::prepare(trimesh::TriMesh* _meshSrc)
	{
		meshSrc = _meshSrc;
		getMeshFace();
		buildMeshFaceHeightsRange(_meshSrc, faceRanges);
	}

	SlicerSegment SliceHelper::project2D(const Point3& p0, const Point3& p1, const Point3& p2, const coord_t z)
	{
		SlicerSegment seg;

		seg.start.X = interpolate(z, p0.z, p1.z, p0.x, p1.x);
		seg.start.Y = interpolate(z, p0.z, p1.z, p0.y, p1.y);
		seg.end.X = interpolate(z, p0.z, p2.z, p0.x, p2.x);
		seg.end.Y = interpolate(z, p0.z, p2.z, p0.y, p2.y);

		return seg;
	}

	void SliceHelper::buildMeshFaceHeightsRange(const MeshObject* mesh, std::vector<Point2>& heightRanges)
	{
		int faceSize = (int)mesh->faces.size();
		if (faceSize > 0) heightRanges.resize(faceSize);
		for (int i = 0; i < faceSize; ++i)
		{
			const MeshFace& face = mesh->faces[i];
			const MeshVertex& v0 = mesh->vertices[face.vertex_index[0]];
			const MeshVertex& v1 = mesh->vertices[face.vertex_index[1]];
			const MeshVertex& v2 = mesh->vertices[face.vertex_index[2]];

			// get all vertices represented as 3D point
			Point3 p0 = v0.p;
			Point3 p1 = v1.p;
			Point3 p2 = v2.p;

			// find the minimum and maximum z point		
			int32_t minZ = p0.z;
			if (p1.z < minZ)
			{
				minZ = p1.z;
			}
			if (p2.z < minZ)
			{
				minZ = p2.z;
			}

			int32_t maxZ = p0.z;
			if (p1.z > maxZ)
			{
				maxZ = p1.z;
			}
			if (p2.z > maxZ)
			{
				maxZ = p2.z;
			}

			heightRanges.at(i) = Point2(minZ, maxZ);
		}
	}

	void SliceHelper::sliceOneLayer(int z,
		std::vector<SlicerSegment>& segments, std::unordered_map<int, int>& face_idx_to_segment_idx)
	{
		segments.reserve(100);

		int faceNum = mesh->faces.size();
		// loop over all mesh faces
		for (int faceIdx = 0; faceIdx < faceNum; ++faceIdx)
		{
			if ((z < faceRanges[faceIdx].x) || (z > faceRanges[faceIdx].y))
				continue;

			// get all vertices per face
			const MeshFace& face = mesh->faces[faceIdx];
			const MeshVertex& v0 = mesh->vertices[face.vertex_index[0]];
			const MeshVertex& v1 = mesh->vertices[face.vertex_index[1]];
			const MeshVertex& v2 = mesh->vertices[face.vertex_index[2]];

			// get all vertices represented as 3D point
			Point3 p0 = v0.p;
			Point3 p1 = v1.p;
			Point3 p2 = v2.p;

			SlicerSegment s;
			s.endVertex = nullptr;
			int end_edge_idx = -1;

			if (p0.z < z && p1.z >= z && p2.z >= z)
			{
				s = project2D(p0, p2, p1, z);
				end_edge_idx = 0;
				if (p1.z == z)
				{
					s.endVertex = &v1;
				}
			}
			else if (p0.z > z && p1.z < z && p2.z < z)
			{
				s = project2D(p0, p1, p2, z);
				end_edge_idx = 2;
			}
			else if (p1.z < z && p0.z >= z && p2.z >= z)
			{
				s = project2D(p1, p0, p2, z);
				end_edge_idx = 1;
				if (p2.z == z)
				{
					s.endVertex = &v2;
				}
			}
			else if (p1.z > z && p0.z < z && p2.z < z)
			{
				s = project2D(p1, p2, p0, z);
				end_edge_idx = 0;
			}
			else if (p2.z < z && p1.z >= z && p0.z >= z)
			{
				s = project2D(p2, p1, p0, z);
				end_edge_idx = 2;
				if (p0.z == z)
				{
					s.endVertex = &v0;
				}
			}
			else if (p2.z > z && p1.z < z && p0.z < z)
			{
				s = project2D(p2, p0, p1, z);
				end_edge_idx = 1;
			}
			else
			{
				//Not all cases create a segment, because a point of a face could create just a dot, and two touching faces
				//  on the slice would create two segments
				continue;
			}

			// store the segments per layer
			face_idx_to_segment_idx.insert(std::make_pair(faceIdx, segments.size()));
			s.faceIndex = faceIdx;
			s.endOtherFaceIdx = face.connected_face_index[end_edge_idx];
			segments.push_back(s);
		}
	}

	void SliceHelper::buildMeshFaceHeightsRange(const trimesh::TriMesh* _meshSrc, std::vector<Point2>& heightRanges)
	{
		int faceSize = (int)_meshSrc->faces.size();
		if (faceSize > 0) heightRanges.resize(faceSize);
		for (int i = 0; i < faceSize; ++i)
		{
			const trimesh::TriMesh::Face& face = _meshSrc->faces[i];

			// get all vertices represented as 3D point
			Point3 p0 = Point3(MM2INT(_meshSrc->vertices[face[0]].x), MM2INT(_meshSrc->vertices[face[0]].y), MM2INT(_meshSrc->vertices[face[0]].z));
			Point3 p1 = Point3(MM2INT(_meshSrc->vertices[face[1]].x), MM2INT(_meshSrc->vertices[face[1]].y), MM2INT(_meshSrc->vertices[face[1]].z));
			Point3 p2 = Point3(MM2INT(_meshSrc->vertices[face[2]].x), MM2INT(_meshSrc->vertices[face[2]].y), MM2INT(_meshSrc->vertices[face[2]].z));

			// find the minimum and maximum z point		
			int32_t minZ = p0.z;
			if (p1.z < minZ)
			{
				minZ = p1.z;
			}
			if (p2.z < minZ)
			{
				minZ = p2.z;
			}

			int32_t maxZ = p0.z;
			if (p1.z > maxZ)
			{
				maxZ = p1.z;
			}
			if (p2.z > maxZ)
			{
				maxZ = p2.z;
			}

			heightRanges.at(i) = Point2(minZ, maxZ);
		}
	}

	void getConnectFaceData(std::vector<std::vector<uint32_t>>& vertexConnectFaceData, std::vector<trimesh::TriMesh::Face> allFaces)
	{
		std::vector<trimesh::TriMesh::Face> nearFaces;
		int faceId = 0;
		for (trimesh::TriMesh::Face face : allFaces)
		{
			vertexConnectFaceData[face[0]].push_back(faceId);
			vertexConnectFaceData[face[1]].push_back(faceId);
			vertexConnectFaceData[face[2]].push_back(faceId);
			faceId++;
		}
	}

	int getNearFaceId(std::vector<std::vector<uint32_t>>& vertexConnectFaceData, int curFaceId, int vertexId_1, int vertexId_2)
	{
		std::vector<uint32_t> firstVertexFaceId = vertexConnectFaceData[vertexId_1];
		std::vector<uint32_t> secondVertexFaceId = vertexConnectFaceData[vertexId_2];
		for (int i = 0; i < firstVertexFaceId.size(); i++)
		{
			int MatchFaceId = firstVertexFaceId[i];
			if (MatchFaceId == curFaceId) continue;
			for (int j = 0; j < secondVertexFaceId.size(); j++)
			{
				if (MatchFaceId == secondVertexFaceId[j])
				{
					return MatchFaceId;
				}
			}
		}
		return -1;
	}

	trimesh::TriMesh* SliceHelper::getMeshSrc()
	{
		return meshSrc;
	}

	std::vector<Point2>* SliceHelper::getFaceRanges()
	{
		return &faceRanges;
	}

	void SliceHelper::getMeshFace()
	{
		size_t faceSize = meshSrc->faces.size();
		size_t vertexSize = meshSrc->vertices.size();
		vertexConnectFaceData.clear();
		vertexConnectFaceData.resize(vertexSize);
		getConnectFaceData(vertexConnectFaceData, meshSrc->faces);
		faces.clear();
		faces.reserve(faceSize);
		for (int i = 0; i < faceSize; i++)
		{
			trimesh::TriMesh::Face face = meshSrc->faces[i];
			MeshFace tmpFace;
			tmpFace.vertex_index[0] = face[0];
			tmpFace.vertex_index[1] = face[1];
			tmpFace.vertex_index[2] = face[2];
			tmpFace.connected_face_index[0] = getNearFaceId(vertexConnectFaceData, i, face[0], face[1]);
			tmpFace.connected_face_index[1] = getNearFaceId(vertexConnectFaceData, i, face[1], face[2]);
			tmpFace.connected_face_index[2] = getNearFaceId(vertexConnectFaceData, i, face[2], face[0]);
			faces.push_back(tmpFace);
		}
	}
}