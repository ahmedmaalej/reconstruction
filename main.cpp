#include "common.h"
#include "PointColorCalculator.hpp"
#include "PointReader.hpp"

int main(int argc, char** argv)
{
	Arrangement_2 arr;
	std::list<std::pair<Point_2,int> > points;
	PointReader::get_points("points.dat",points);
/*	Point_2	p1(0,0), p2(1,0), p3(0.5,sqrt(3)/2);
//	Point_2	p1(0,0), p2(1,0), p3(0.8,0.8),p4(0.2,0.8);

	points.push_back(std::pair<Point_2,int>(p1,1));
	points.push_back(std::pair<Point_2,int>(p2,2));
	points.push_back(std::pair<Point_2,int>(p3,1));
	points.push_back(std::pair<Point_2,int>(p4,0));
	*/
	int resolution_x;
	std::istringstream(std::string(argv[1])) >> resolution_x;
	int resolution_y;
	std::istringstream(std::string(argv[2])) >> resolution_y;
	int algorithm_num;
	std::istringstream(std::string(argv[3])) >> algorithm_num;
	PointColorCalculatorAlgorithm algorithm;
	switch (algorithm_num)
	{
		case 0:
			algorithm = OurAlgorithm;
			break;
		case 1:
			algorithm = NN;
			break;
	}
	PointColorCalculator calc(points, algorithm);
	calc.to_qt(resolution_x,resolution_y);
	return 0;
}

