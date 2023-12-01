#include "paintzseam.h"
#include "linearAlg2D.h"


namespace cura52
{
	paintzseam::paintzseam(SliceDataStorage* _storage, const size_t _total_layers)
		:storage(_storage),
		total_layers(_total_layers)
	{
		markerZSeam();
		generateZSeam();
	}

	void paintzseam::markerZSeam()
	{
		for (int layer_nr = 0; layer_nr < total_layers; layer_nr++)
		{
			int icount = 0;
			for (SliceMeshStorage& mesh : storage->meshes)
			{
				for (SliceLayerPart& part : mesh.layers[layer_nr].parts)
				{
					for (VariableWidthLines& path : part.wall_toolpaths)
					{
						for (ExtrusionLine& line : path)
						{
							if (line.inset_idx == 0 && line.is_closed)
							{
								for (ZseamDrawPoint& aPoint : storage->zSeamPoints[layer_nr].ZseamLayers)
								{
									if (aPoint.flag)//已经标记过的涂抹点，跳过
									{
										continue;
									}

									coord_t minPPdis = std::numeric_limits<coord_t>::max();
									int minPPidex=getDisPtoJunctions(aPoint.start, line.junctions, minPPdis);
									if (minPPdis < 1000)//涂抹点到轮廓点的最短距离
									{
										line.junctions[minPPidex].isZSeamDrow = true;
										aPoint.flag = true;
									}
									else
									{
										int preIdx = (minPPidex + line.junctions.size() - 2) % (line.junctions.size() - 1);
										int nextIdx = (minPPidex + 1) % (line.junctions.size() - 1);

										coord_t pre_dis = getDistFromSeg(aPoint.start, line.junctions[minPPidex].p, line.junctions[preIdx].p);
										coord_t next_dis = getDistFromSeg(aPoint.start, line.junctions[minPPidex].p, line.junctions[nextIdx].p);
										if (pre_dis >= 1000 && next_dis >= 1000)//距离大于1000，则认为涂抹点和轮廓没有匹配上，跳过
										{
											continue;
										}

										if (pre_dis < next_dis)
										{
											Point addPoint = getDisPtoSegment(aPoint.start, line.junctions[minPPidex].p, line.junctions[preIdx].p);
											if (std::abs(addPoint.X - line.junctions[minPPidex].p.X) < 500 && std::abs(addPoint.Y - line.junctions[minPPidex].p.Y) < 500)
											{
												line.junctions[minPPidex].isZSeamDrow = true;
												aPoint.flag = true;
												continue;
											}
											ExtrusionJunction addEJ = line.junctions.at(preIdx);
											addEJ.p = addPoint;
											addEJ.isZSeamDrow = true;
											aPoint.flag = true;
											if (minPPidex == 0)
											{
												minPPidex = preIdx + 1;
											}
											line.junctions.insert(line.junctions.begin() + minPPidex, addEJ);
										}
										else
										{
											Point addPoint = getDisPtoSegment(aPoint.start, line.junctions[minPPidex].p, line.junctions[nextIdx].p);
											if (std::abs(addPoint.X - line.junctions[minPPidex].p.X) < 500 && std::abs(addPoint.Y - line.junctions[minPPidex].p.Y) < 500)
											{
												line.junctions[minPPidex].isZSeamDrow = true;
												aPoint.flag = true;
												continue;
											}
											ExtrusionJunction addEJ = line.junctions.at(nextIdx);
											addEJ.p = addPoint;
											addEJ.isZSeamDrow = true;
											aPoint.flag = true;
											if (nextIdx == 0)
											{
												nextIdx = minPPidex + 1;
											}
											line.junctions.insert(line.junctions.begin() + nextIdx, addEJ);
										}

									}
								}
							}
						}
					}
				}
			}
		}
	}

	void paintzseam::generateZSeam()
	{
		std::vector<Point> PreZseamPoints;
		std::vector<Point> curZseamPoints;
		for (int layer_nr = 0; layer_nr < total_layers; layer_nr++)
		{
			for (SliceMeshStorage& mesh : storage->meshes)
			{
				for (SliceLayerPart& part : mesh.layers[layer_nr].parts)
				{
					for (VariableWidthLines& path : part.wall_toolpaths)
					{
						for (ExtrusionLine& line : path)
						{
							if (line.inset_idx == 0 && line.is_closed)
							{
								if (layer_nr == 0)
								{
									float minCornerAngle = M_PI;
									int minCorner_idx = -1;//最尖角索引
									for (int n = 0; n < line.junctions.size() - 1; n++)
									{
										if (line.junctions[n].isZSeamDrow)
										{
											//int preIdx = (n - 1 + line.junctions.size()) % line.junctions.size();
											//int nextIdx = (n + 1) % line.junctions.size();
											int preIdx = (n + line.junctions.size() - 2) % (line.junctions.size() - 1);
											int nextIdx = (n + 1) % (line.junctions.size() - 1);
											float pab = getAngleLeft(line.junctions[preIdx].p, line.junctions[n].p, line.junctions[nextIdx].p);
											if (minCornerAngle >= pab)
											{
												minCornerAngle = pab;
												minCorner_idx = n;
											}
										}
									}
									if (minCorner_idx != -1)
									{
										line.start_idx = minCorner_idx;
										curZseamPoints.push_back(line.junctions[minCorner_idx].p);
									}
								}
								else
								{
									coord_t minPPdis = std::numeric_limits<coord_t>::max();
									int minPPidex = -1;
									Point shortest_point;
									for (int n = 0; n < line.junctions.size() - 1; n++)
									{
										if (line.junctions[n].isZSeamDrow)
										{
											for (Point& apoint : PreZseamPoints)
											{
												coord_t ppdis = vSize(line.junctions[n].p - apoint);
												if (minPPdis > ppdis /*&& ppdis<3000*/)
												{
													minPPdis = ppdis;
													minPPidex = n;//最短距离
													shortest_point = apoint;
												}
											}
										}
									}

									float minAngle = M_PI;
									float shortestAngle = M_PI;
									int sharpCorner_idx = -1;
									for (int n = 0; n < line.junctions.size() - 1; n++)
									{
										if (line.junctions[n].isZSeamDrow)
										{
											//int preIdx = (n - 1 + line.junctions.size()) % line.junctions.size();
											//int nextIdx = (n + 1) % line.junctions.size();
											int preIdx = (n + line.junctions.size() - 2) % (line.junctions.size() - 1);
											int nextIdx = (n + 1) % (line.junctions.size() - 1);
											float pab = getAngleLeft(line.junctions[preIdx].p, line.junctions[n].p, line.junctions[nextIdx].p);
											if (minAngle >= pab)//最尖角
											{
												minAngle = pab;
												sharpCorner_idx = n;
											}
											if (minPPidex == n)
											{
												shortestAngle = pab;
											}
										}
									}

									if ((std::abs(shortestAngle - minAngle) < (M_PI / 6) || minAngle > (M_PI * 3 / 4)) && minPPidex >= 0)
									{

										int preIdx = (minPPidex + line.junctions.size() - 2) % (line.junctions.size() - 1);
										int nextIdx = (minPPidex + 1) % (line.junctions.size() - 1);
										if (line.junctions[preIdx].isZSeamDrow && line.junctions[nextIdx].isZSeamDrow)
										{
											coord_t pre_dis = getDistFromSeg(shortest_point, line.junctions[minPPidex].p, line.junctions[preIdx].p);
											coord_t next_dis = getDistFromSeg(shortest_point, line.junctions[minPPidex].p, line.junctions[nextIdx].p);
											if (pre_dis < next_dis)
											{
												Point addPoint = getDisPtoSegment(shortest_point, line.junctions[minPPidex].p, line.junctions[preIdx].p);
												if (addPoint.X == line.junctions[minPPidex].p.X && addPoint.Y == line.junctions[minPPidex].p.Y)
												{
													line.start_idx = minPPidex;
													curZseamPoints.push_back(line.junctions[minPPidex].p);
													continue;
												}
												ExtrusionJunction addEJ = line.junctions.at(preIdx);
												addEJ.p = addPoint;
												addEJ.isZSeamDrow = true;
												if (minPPidex == 0)
												{
													minPPidex = preIdx + 1;
												}
												line.junctions.insert(line.junctions.begin() + minPPidex, addEJ);
												line.start_idx = minPPidex;
												curZseamPoints.push_back(line.junctions[minPPidex].p);
											}
											else
											{
												Point addPoint = getDisPtoSegment(shortest_point, line.junctions[minPPidex].p, line.junctions[nextIdx].p);
												if (addPoint.X == line.junctions[minPPidex].p.X && addPoint.Y == line.junctions[minPPidex].p.Y)
												{
													line.start_idx = minPPidex;
													curZseamPoints.push_back(line.junctions[minPPidex].p);
													continue;
												}
												ExtrusionJunction addEJ = line.junctions.at(nextIdx);
												addEJ.p = addPoint;
												addEJ.isZSeamDrow = true;
												if (nextIdx == 0)
												{
													nextIdx = minPPidex + 1;
												}
												line.junctions.insert(line.junctions.begin() + nextIdx, addEJ);
												line.start_idx = nextIdx;
												curZseamPoints.push_back(line.junctions[nextIdx].p);
											}
										}
										else if (line.junctions[preIdx].isZSeamDrow)
										{
											Point addPoint = getDisPtoSegment(shortest_point, line.junctions[minPPidex].p, line.junctions[preIdx].p);
											if (addPoint.X == line.junctions[minPPidex].p.X && addPoint.Y == line.junctions[minPPidex].p.Y)
											{
												line.start_idx = minPPidex;
												curZseamPoints.push_back(line.junctions[minPPidex].p);
												continue;
											}
											ExtrusionJunction addEJ = line.junctions.at(preIdx);
											addEJ.p = addPoint;
											addEJ.isZSeamDrow = true;
											if (minPPidex == 0)
											{
												minPPidex = preIdx + 1;
											}
											line.junctions.insert(line.junctions.begin() + minPPidex, addEJ);
											line.start_idx = minPPidex;
											curZseamPoints.push_back(line.junctions[minPPidex].p);
										}
										else if (line.junctions[nextIdx].isZSeamDrow)
										{
											Point addPoint = getDisPtoSegment(shortest_point, line.junctions[minPPidex].p, line.junctions[nextIdx].p);
											if (addPoint.X == line.junctions[minPPidex].p.X && addPoint.Y == line.junctions[minPPidex].p.Y)
											{
												line.start_idx = minPPidex;
												curZseamPoints.push_back(line.junctions[minPPidex].p);
												continue;
											}
											ExtrusionJunction addEJ = line.junctions.at(nextIdx);
											addEJ.p = addPoint;
											addEJ.isZSeamDrow = true;
											if (nextIdx == 0)
											{
												nextIdx = minPPidex + 1;
											}
											line.junctions.insert(line.junctions.begin() + nextIdx, addEJ);
											line.start_idx = nextIdx;
											curZseamPoints.push_back(line.junctions[nextIdx].p);
										}
										else
										{
											line.start_idx = minPPidex;
											curZseamPoints.push_back(line.junctions[minPPidex].p);
										}
									}
									else
									{
										if (sharpCorner_idx != -1)
										{
											line.start_idx = sharpCorner_idx;
											curZseamPoints.push_back(line.junctions[sharpCorner_idx].p);
										}
									}

								}
							}

						}

					}

				}

			}
			PreZseamPoints = curZseamPoints;
			curZseamPoints.clear();
		}
	}



	coord_t paintzseam::getDistFromSeg(const Point& p, const Point& Seg_start, const Point& Seg_end)
	{
		float pab = LinearAlg2D::getAngleLeft(p, Seg_start, Seg_end);
		float pba = LinearAlg2D::getAngleLeft(p, Seg_end, Seg_start);
		if (pab > M_PI / 2 && pab < 3 * M_PI / 2) {
			return vSize(p - Seg_start);
		}
		else if (pba > M_PI / 2 && pba < 3 * M_PI / 2) {
			return vSize(p - Seg_end);
		}
		else {
			return LinearAlg2D::getDistFromLine(p, Seg_start, Seg_end);
		}
	}

	Point paintzseam::getDisPtoSegment(Point& apoint, Point& Segment_start, Point& Segment_end)
	{
		double A = Segment_end.Y - Segment_start.Y;     //y2-y1
		double B = Segment_start.X - Segment_end.X;     //x1-x2;
		double C = Segment_end.X * Segment_start.Y - Segment_start.X * Segment_end.Y;     //x2*y1-x1*y2
		if (A * A + B * B < 1e-13) {
			return Segment_start;   //Segment_start与Segment_end重叠
		}
		else {
			double x = (B * B * apoint.X - A * B * apoint.Y - A * C) / (A * A + B * B);
			double y = (-A * B * apoint.X + A * A * apoint.Y - B * C) / (A * A + B * B);
			Point fpoint = Point();
			fpoint.X = x;
			fpoint.Y = y;

			if (pointOnSegment(fpoint, Segment_start, Segment_end))
			{
				return fpoint;
			}

			if (vSize(Point(Segment_start.X - apoint.X, Segment_start.Y - apoint.Y)) < vSize(Point(Segment_end.X - apoint.X, Segment_end.Y - apoint.Y)))
			{
				return Segment_start;
			}
			else
			{
				return Segment_end;
			}
		}
	}

	bool paintzseam::pointOnSegment(Point p, Point Segment_start, Point Segment_end)
	{
		if (p.X >= std::min(Segment_start.X, Segment_end.X) &&
			p.X <= std::max(Segment_start.X, Segment_end.X) &&
			p.Y >= std::min(Segment_start.Y, Segment_end.Y) &&
			p.Y <= std::max(Segment_start.Y, Segment_end.Y))
		{
			if ((Segment_end.X - Segment_start.X) * (p.Y - Segment_start.Y) == (p.X - Segment_start.X) * (Segment_end.Y - Segment_start.Y))
			{
				return true;
			}
		}
		return false;
	}

	float paintzseam::getAngleLeft(const Point& a, const Point& b, const Point& c)
	{
		float angle = LinearAlg2D::getAngleLeft(a, b, c);
		if (angle > M_PI)
		{
			angle = 2 * M_PI - angle;
		}
		return angle;
	}

	int paintzseam::getDisPtoJunctions(Point& aPoint, std::vector<ExtrusionJunction>& junctions, coord_t& minPPdis)
	{
		int minPPidex = -1;
		for (int n = 0; n < junctions.size() - 1; n++)
		{
			coord_t ppdis = vSize(junctions[n].p - aPoint);
			if (minPPdis > ppdis)
			{
				minPPdis = ppdis;
				minPPidex = n;
			}
		}
		return minPPidex;
	}

}


