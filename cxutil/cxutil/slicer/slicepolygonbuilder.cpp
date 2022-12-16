#include "cxutil/slicer/slicepolygonbuilder.h"
#include "cxutil/math/utilconst.h"
#include "cxutil/slicer/slicehelper.h"
#include "trimesh2/TriMesh.h"

#include <fstream>

namespace cxutil
{
	SlicePolygonBuilder::SlicePolygonBuilder()
	{

	}

	SlicePolygonBuilder::~SlicePolygonBuilder()
	{

	}

	void SlicePolygonBuilder::makePolygon(Polygons* polygons, Polygons* openPolygons)
	{
		size_t segment_size = segments.size();
		if (segment_size > 0) visitFlags.resize(segment_size, false);
		for (size_t start_segment_idx = 0; start_segment_idx < segment_size; start_segment_idx++)
		{
			if (!visitFlags[start_segment_idx])
			{
				ClipperLib::Path poly;
				poly.push_back(segments[start_segment_idx].start);

				bool closed = false;
				for (int segment_idx = start_segment_idx; segment_idx != -1; )
				{
					const SlicerSegment& segment = segments[segment_idx];
					poly.push_back(segment.end);

					visitFlags[segment_idx] = true;
					segment_idx = getNextSegmentIdx(segment, start_segment_idx);
					if (segment_idx == static_cast<int>(start_segment_idx))
					{ // polyon is closed
						closed = true;
						break;
					}
				}

				if (closed)
				{
					polygons->add(poly);
				}
				else
				{
					// polygon couldn't be closed
					openPolygons->add(poly);
				}
			}
		}
	}

	coord_t interpolate_my(const coord_t x, const coord_t x0, const coord_t x1, const coord_t y0, const coord_t y1)
	{
		const coord_t dx_01 = x1 - x0;
		coord_t num = (y1 - y0) * (x - x0);
		num += num > 0 ? dx_01 / 2 : -dx_01 / 2; // add in offset to round result
		return y0 + num / dx_01;
	}

	void project2D(const Point3& p0, const Point3& p1, const Point3& p2, const coord_t z, SlicerSegment& seg)
	{
		if (seg.faceIndex == -1)
		{
			seg.start.X = interpolate_my(z, p0.z, p1.z, p0.x, p1.x);
			seg.start.Y = interpolate_my(z, p0.z, p1.z, p0.y, p1.y);
		}
		else
		{
			seg.start.X = seg.end.X;
			seg.start.Y = seg.end.Y;
		}
		seg.end.X = interpolate_my(z, p0.z, p2.z, p0.x, p2.x);
		seg.end.Y = interpolate_my(z, p0.z, p2.z, p0.y, p2.y);

	}

	bool zCrossFace(trimesh::TriMesh* mesh, cxutil::SliceHelper* helper, int faceId, int z, SlicerSegment& s)
	{
		const trimesh::TriMesh::Face& face = mesh->faces[faceId];

		// get all vertices represented as 3D point
		Point3 p0 = Point3(MM2INT(mesh->vertices[face[0]].x), MM2INT(mesh->vertices[face[0]].y), MM2INT(mesh->vertices[face[0]].z));
		Point3 p1 = Point3(MM2INT(mesh->vertices[face[1]].x), MM2INT(mesh->vertices[face[1]].y), MM2INT(mesh->vertices[face[1]].z));
		Point3 p2 = Point3(MM2INT(mesh->vertices[face[2]].x), MM2INT(mesh->vertices[face[2]].y), MM2INT(mesh->vertices[face[2]].z));

		int end_edge_idx = -1;

		if (p0.z < z && p1.z >= z && p2.z >= z)
		{
			project2D(p0, p2, p1, z, s);
			if (p1.z == z)
			{
				if (s.end != s.start)
				{
					s.end.X = p1.x;
					s.end.Y = p1.y;
					s.endOtherFaceIdx = -1;
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				end_edge_idx = 0;
			}
		}
		else if (p0.z > z && p1.z < z && p2.z < z)
		{
			project2D(p0, p1, p2, z, s);
			end_edge_idx = 2;
		}
		else if (p1.z < z && p0.z >= z && p2.z >= z)
		{
			project2D(p1, p0, p2, z, s);
			if (p2.z == z)
			{
				if (s.end != s.start)
				{
					s.end.X = p2.x;
					s.end.Y = p2.y;
					s.endOtherFaceIdx = -1;
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				end_edge_idx = 1;
			}
		}
		else if (p1.z > z && p0.z < z && p2.z < z)
		{
			project2D(p1, p2, p0, z, s);
			end_edge_idx = 0;
		}
		else if (p2.z < z && p1.z >= z && p0.z >= z)
		{
			project2D(p2, p1, p0, z, s);
			if (p0.z == z)
			{
				if (s.end != s.start)
				{
					s.end.X = p0.x;
					s.end.Y = p0.y;
					s.endOtherFaceIdx = -1;
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				end_edge_idx = 2;
			}
		}
		else if (p2.z > z && p1.z < z && p0.z < z)
		{
			project2D(p2, p0, p1, z, s);
			end_edge_idx = 1;
		}
		else
		{
			return false;
			//Not all cases create a segment, because a point of a face could create just a dot, and two touching faces
			//  on the slice would create two segments
		}

		s.faceIndex = faceId;
		if (end_edge_idx != -1)
		{
			s.endOtherFaceIdx = helper->faces[faceId].connected_face_index[end_edge_idx];
		}
		return true;
	}

	void SlicePolygonBuilder::sliceOneLayer_dst(cxutil::SliceHelper* helper, int z, Polygons* polygons, Polygons* openPolygons)
	{
		trimesh::TriMesh* meshSrc = helper->getMeshSrc();
		std::vector<Point2>* faceRanges = helper->getFaceRanges();
		int faceNum = meshSrc->faces.size();

		if (faceNum > 0) visitFlags.resize(faceNum, false);
		for (int i = 0; i < faceNum; ++i)
		{
			if ((z < faceRanges->at(i).x) || (z > faceRanges->at(i).y))
				continue;
			if (visitFlags[i]) continue;
			bool bFirstSegment = true;
			bool bClosed = false;
			SlicerSegment s;
			ClipperLib::Path poly;
			poly.reserve(100);
			for (int faceid = i; faceid != -1; )
			{
				if (visitFlags[faceid]) //开轮廓
				{
					break;
				}
				if (zCrossFace(meshSrc, helper, faceid, z, s))
				{
					if (bFirstSegment) poly.push_back(s.start);
					poly.push_back(s.end);
					visitFlags[faceid] = true;

					faceid = s.endOtherFaceIdx;
					if (bFirstSegment) bFirstSegment = false;
				}
				else
				{
					break; //开轮廓
				}

				if (faceid == i) //闭轮廓
				{
					bClosed = true;
					break;
				}
			}
			if (bClosed)
			{
				ClipperLib::CleanPolygon(poly);
				polygons->add(poly);
			}
			else if (!poly.empty())
			{////开轮廓不能在这里清理轮廓，否则在后续开轮廓闭合时会出错
				openPolygons->add(poly);
			}
		}
	}

	int SlicePolygonBuilder::tryFaceNextSegmentIdx(const SlicerSegment& segment,
		const int face_idx, const size_t start_segment_idx)
	{
		decltype(face_idx_to_segment_idx.begin()) it;
		auto it_end = face_idx_to_segment_idx.end();
		it = face_idx_to_segment_idx.find(face_idx);
		if (it != it_end)
		{
			const int segment_idx = (*it).second;
			Point p1 = segments[segment_idx].start;
			Point diff = segment.end - p1;
			if (shorterThen(diff, largest_neglected_gap_first_phase))
			{
				if (segment_idx == static_cast<int>(start_segment_idx))
				{
					return start_segment_idx;
				}
				if (visitFlags[segment_idx])
				{
					return -1;
				}
				return segment_idx;
			}
		}

		return -1;
	}

	int SlicePolygonBuilder::getNextSegmentIdx(const SlicerSegment& segment, const size_t start_segment_idx)
	{
		int next_segment_idx = -1;

		const bool segment_ended_at_edge = segment.endVertex == nullptr;
		if (segment_ended_at_edge)
		{
			const int face_to_try = segment.endOtherFaceIdx;
			if (face_to_try == -1)
			{
				return -1;
			}
			return tryFaceNextSegmentIdx(segment, face_to_try, start_segment_idx);
		}
		else
		{
			// segment ended at vertex

			const std::vector<uint32_t>& faces_to_try = segment.endVertex->connected_faces;
			for (int face_to_try : faces_to_try)
			{
				const int result_segment_idx =
					tryFaceNextSegmentIdx(segment, face_to_try, start_segment_idx);
				if (result_segment_idx == static_cast<int>(start_segment_idx))
				{
					return start_segment_idx;
				}
				else if (result_segment_idx != -1)
				{
					// not immediately returned since we might still encounter the start_segment_idx
					next_segment_idx = result_segment_idx;
				}
			}
		}

		return next_segment_idx;
	}

	void SlicePolygonBuilder::write(std::fstream& out)
	{
		for (size_t j = 0; j < segments.size(); ++j)
		{
			const SlicerSegment& segment = segments.at(j);
			out << segment.start << " " << segment.endOtherFaceIdx << std::endl;
			out << segment.start << " " << segment.end << std::endl;
		}
		for (const std::pair<int, int>& p : face_idx_to_segment_idx)
		{
			out << p.first << " " << p.second << std::endl;
		}
	}
}