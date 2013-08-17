#include "common.h"

class PointReader
{
	public:
		static void get_points(std::string file_name,std::list< std::pair<Point_2,int> > &points)
		{
			std::ifstream infile(file_name.c_str());
			float x,y;
			int color;
			while(infile >> x >> y >> color)
			{
				points.push_back(std::pair<Point_2,int>(Point_2(x,y),color));
			}	
		}
};
