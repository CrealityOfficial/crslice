#ifndef PAINTZSEAM_274874
#define PAINTZSEAM_274874
#include "communication/sliceDataStorage.h"
namespace cura52
{
	enum PointType
	{
		pre,
		next,
		add
	};

	struct paintPointResult
	{
		Point aPoint;
		int idx = -1;
		double distance = -1;
		PointType type;
	};

	class SliceDataStorage;
	class MeshGroup;
	class paintzseam
	{
	public:
		paintzseam(SliceDataStorage* _storage, const size_t _total_layers);
		void paint();
		void intercept();

		void markerZSeam(std::vector<ZseamDrawlayer>& _layersPoints, ExtrusionJunction::paintFlag _flag);
		void generateZSeam();
		void processZSeam(MeshGroup* mesh_group, AngleDegrees& z_seam_min_angle_diff, AngleDegrees& z_seam_max_angle, coord_t& wall_line_width_0, coord_t& wall_line_count);

		coord_t getDistFromSeg(const Point& p, const Point& Seg_start, const Point& Seg_end);
		Point getDisPtoSegment(Point& apoint, Point& Seg_start, Point& Seg_end);
		Point getDisPtoSegmentEX(Point& apoint, Point& Segment_start, Point& Segment_end, PointType& _type);

		bool pointOnSegment(Point p, Point Segment_start, Point Segment_end);

		float getAngleLeft(const Point& a, const Point& b, const Point& c);

		paintPointResult getPtoJunctions_disAndidx(Point& p, std::vector<ExtrusionJunction>& junctions);

	private:
		SliceDataStorage* storage;
		size_t total_layers;
	};
}

#endif //PAINTZSEAM_274874

