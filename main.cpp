#include "common.h"
#include "PointColorCalculator.hpp"

int main(int argc, char** argv)
{
	Arrangement_2 arr;
	Point_2	p1(0,0), p2(1,0), p3(0.5,sqrt(3)/2);

	std::list<std::pair<Point_2,int> > points;
	points.push_back(std::pair<Point_2,int>(p1,0));
	points.push_back(std::pair<Point_2,int>(p2,2));
	points.push_back(std::pair<Point_2,int>(p3,1));
	int resolution_x;
	std::istringstream(std::string(argv[1])) >> resolution_x;
	int resolution_y;
	std::istringstream(std::string(argv[2])) >> resolution_y;

	PointColorCalculator calc(points);
	calc.to_ppm("result.ppm",resolution_x,resolution_y);
	return 0;
}

