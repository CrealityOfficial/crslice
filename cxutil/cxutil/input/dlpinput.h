#ifndef CX_DLPINPUT_1602651441705_H
#define CX_DLPINPUT_1602651441705_H
#include "cxutil/input/meshobject.h"

namespace trimesh {
	class TriMesh;
}

namespace cxutil
{
	
	struct DLPParam
	{
		coord_t initial_layer_thickness;
		coord_t layer_thickness;

		coord_t minimum_polygon_circumference;
		coord_t line_segment_resolution;
		coord_t line_segment_deviation;

		float xy_offset;
		bool enable_xy_offset;

		bool enable_b_offset;
		float sThickness;
		float sh;
		float sLength;
		float sWidth;
		float pixX;
		float pixY;

		DLPParam()
		{
			initial_layer_thickness = 300;
			layer_thickness = 100;

			minimum_polygon_circumference = 1000;
			line_segment_resolution = 50;
			line_segment_deviation = 50;

			xy_offset = 0;
			enable_xy_offset = false;

			enable_b_offset = false;
			sThickness = 0.5 * 1000;
			sh = 100 * 1000;
			sLength = 192 * 1000;
			sWidth = 1000;
			pixX = 3240;
			pixY = 2400;
		}
	};


	class DLPInput
	{
	public:
		DLPInput();
		~DLPInput();

		void addMeshObject(MeshObjectPtr object);
		void addTriMesh(trimesh::TriMesh* Mesh);
		std::vector<trimesh::TriMesh*>& getMeshesSrc();
		const std::vector< MeshObjectPtr>& meshes() const;
		std::vector<MeshObjectPtr>& meshes();

		void addParam(DLPParam param);
		DLPParam& param();
		AABB3D box();
	protected:
		std::vector <trimesh::TriMesh*> m_meshesSrc;
		std::vector<MeshObjectPtr> m_meshes;
		DLPParam m_param;
	};
}

#endif // CX_DLPINPUT_1602651441705_H