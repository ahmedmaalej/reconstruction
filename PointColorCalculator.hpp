#include "common.h"
#include <iostream>
#include <fstream>
#include <CGAL/bounding_box.h>
#include <CGAL/IO/Geomview_stream.h>

enum PointColorCalculatorAlgorithm
{
	OurAlgorithm = 0,
	NN = 1
};

class PointColorCalculator
{
	private:
	Arrangement_2 arr;
	Kernel::Iso_rectangle_2	rect;
	PointColorCalculatorAlgorithm algorithm;
	CGAL::Geomview_stream *gv;

	public:
	PointColorCalculator(std::list<std::pair<Point_2,int> > &points, PointColorCalculatorAlgorithm algorithm)
	{
		this->algorithm = algorithm;
		Colored_segment	cv[points.size()];
		int index = 0;
		std::list<Point_2> points_only;
		std::list<std::pair<Segment_2,int> > segments;
		for (std::list<std::pair<Point_2,int> >::iterator it = points.begin(); it != points.end(); it++)
		{
			std::list< std::pair<Point_2,int> >::iterator inner_it = it;
			std::pair<Point_2,int> current_point = *inner_it;
			points_only.push_back(current_point.first);
			std::pair<Point_2,int> next_point;
			++inner_it;
			if (inner_it == points.end())
			{
				next_point = *(points.begin()); 
			}
			else
			{
				next_point = *inner_it; 
			}
			cv[index++] = Colored_segment( Segment_2(current_point.first,next_point.first), current_point.second);
			segments.push_back(std::pair<Segment_2,int>(Segment_2(current_point.first,next_point.first),current_point.second));
		}

		rect = CGAL::bounding_box<std::list<Point_2>::iterator>(points_only.begin(),points_only.end());
		
		
		CGAL::insert (arr, &cv[0], &cv[points_only.size()]);
		Number_type min_x = rect.xmin();
		Number_type min_y = rect.ymin();
		Number_type  max_x = rect.xmax();
		Number_type  max_y = rect.ymax();	


		gv = new CGAL::Geomview_stream(CGAL::Bbox_3(CGAL::to_double(min_x),CGAL::to_double(min_y),0,CGAL::to_double(max_x),CGAL::to_double(max_y),0));
		(*gv).set_line_width(4);
		(*gv).clear();
		for (std::list<std::pair<Segment_2,int> >::iterator it = segments.begin(); it != segments.end();it++)
		{
			static unsigned int color_rgb[3];
			resolve_int_to_color((*it).second,color_rgb);
			*gv << CGAL::Color(color_rgb[0],color_rgb[1],color_rgb[2]);
			*gv << (*it).first;
		}
	}	

	int GetColor(Point_2 p)
	{
		Naive_pl	naive_pl(arr);
		CGAL::Object obj = naive_pl.locate(p);
		Arrangement_2::Face_const_handle f;
		Arrangement_2::Halfedge_const_handle he;
		Colored_segment *s;
		CGAL::set_pretty_mode(std::cout);
		Traits_2::Compute_squared_distance_2 squared_distance;
		Number_type score[MAX_COLOR];
		int chosen_color = -2;
		if (CGAL::assign(f,obj) && !f->is_unbounded())
		{
			chosen_color = -1; //we're in face, but it shouldn't be used
			if (!f->has_outer_ccb()) return chosen_color;
			Number_type max_score = 0;
			for (int i = 0; i < MAX_COLOR; i++) 
			{
				score[i] = 0;
				Arrangement_2::Ccb_halfedge_const_circulator first, curr;
				curr = first = f->outer_ccb();
				do {
					he = curr;
					curr++;
					s = (Colored_segment*)&he->curve();;
					int data_size = he->curve().data().size();
					if (data_size != 1) {
					}
					Data_traits::Data_container::const_iterator it;
					int color = he->curve().data().front();
					if (color == i)
					{	
						if (this->algorithm == OurAlgorithm)
						{
							Point_2 proj = s->line().projection(p);
							Kernel::Segment_2 segment(s->source(),s->target());
							if (!segment.has_on(proj)) continue;
							Number_type distance_from_projection = squared_distance(p,proj);
							Number_type distance_from_edges = std::min(squared_distance(proj,he->source()->point()),squared_distance(proj,he->target()->point())); 
							Number_type score_from_this_line = distance_from_edges / distance_from_projection;
							score[i] += score_from_this_line;
						}
						if (this->algorithm == NN)
						{
							Point_2 proj = s->line().projection(p);
							Kernel::Segment_2 segment(s->source(),s->target());
							if (!segment.has_on(proj)) continue;
							Number_type distance_from_projection = squared_distance(p,proj);
							score[i] = score[i] == 0 ? 1/distance_from_projection: std::max(1/distance_from_projection,score[i]);
						}
					}
				} while (curr != first);
				double normalized_score = CGAL::to_double(score[i]);
				if (normalized_score > max_score) 
				{
					max_score = normalized_score;
					chosen_color = i;
				}	
			}
		}
		return chosen_color;
	}

	void to_geomview(int resolution_x, int resolution_y)
	{
		int color = -1;
		Number_type min_x = rect.xmin();
		Number_type min_y = rect.ymin();
		Number_type  max_x = rect.xmax();
		Number_type  max_y = rect.ymax();	

		std::cout << "min_x,max_x,min_y,max_y: " << min_x << "," << max_x << "," << min_y << "," << max_y << std::endl;
		Number_type step_x = (max_x-min_x)/resolution_x;
		Number_type step_y = (max_y-min_y)/resolution_y;
		Point_2 *test_point;
		Point_3 *draw_point;
		int counter = 0;
		for ( Number_type i =min_y;i<=max_y;i+=step_y)
		{
			for (Number_type j = min_x; j<=max_x;j+=step_x)
			{
				if (counter++ % 1000 == 0)
					std::cout << "for point " << i << "," << j << " got color " << (color >= 0 ? color_names[color] : "UNKNOWN") << std::endl;

				test_point = new Traits_2::Point_2(j,i);
				color = GetColor(*test_point);	
				static unsigned int color_rgb[3];
				resolve_int_to_color(color,color_rgb);
				if (color_rgb[0]==-1) continue;
				delete test_point;
				draw_point = new Traits_2::Point_3(j,i,0);
				*gv << CGAL::Color(color_rgb[0],color_rgb[1],color_rgb[2]);
				*gv << *draw_point;
				delete draw_point;
			}
		}
		std::cout << "Press any key to finish";
		char ch;
		std::cin >> ch;
	}

	void resolve_int_to_color(int color, unsigned int color_rgb[3])
	{
		switch(color)
		{
			case 0:
				color_rgb[0] = 255;
				color_rgb[1] = 0;
				color_rgb[2] = 0;
				break;
			case 1:
				color_rgb[0] = 0;
				color_rgb[1] = 255;
				color_rgb[2] = 0;
				break;
			case 2:
				color_rgb[0] = 0;
				color_rgb[1] = 0;
				color_rgb[2] = 255;
				break;
			
			default:
				color_rgb[0]=-1;
				color_rgb[1]=-1;
				color_rgb[2]=-1;
		}
	}
};
