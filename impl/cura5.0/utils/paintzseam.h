#ifndef PAINTZSEAM_274874
#define PAINTZSEAM_274874
#include "communication/sliceDataStorage.h"

namespace cura52
{
	class paintzseam
	{
	public:
		paintzseam(SliceDataStorage* _storage, const size_t _total_layers);
		void markerZSeam();
		void generateZSeam();

		coord_t getDistFromSeg(const Point& p, const Point& Seg_start, const Point& Seg_end);
		Point getDisPtoSegment(Point& apoint, Point& Seg_start, Point& Seg_end);

		bool pointOnSegment(Point p, Point Segment_start, Point Segment_end);

		float getAngleLeft(const Point& a, const Point& b, const Point& c);

		int getDisPtoJunctions(Point& p, std::vector<ExtrusionJunction>& junctions, coord_t& minPPdis);

	private:
		SliceDataStorage* storage;
		size_t total_layers;
	};
}

#endif //PAINTZSEAM_274874

