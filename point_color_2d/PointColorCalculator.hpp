#include "common.h"
#include <iostream>
#include <fstream>
#include <CGAL/bounding_box.h>
#include <CGAL/IO/Geomview_stream.h>
#include <QApplication>
#include <CGAL/Qt/GraphicsViewInput.h>
#include <CGAL/Qt/PointsGraphicsItem.h>
#include <CGAL/Qt/SegmentsGraphicsItem.h>
#include <QGraphicsView>
#include <QPixmap>
#include "ColoredPointsGraphicsItem.hpp"
#include <CGAL/Arr_batched_point_location.h>

enum PointColorCalculatorAlgorithm
{
	OurAlgorithm = 0,
	NN = 1
};

typedef std::pair<Point_2, CGAL::Object>              Query_result;

class PointColorCalculator
{
	private:
	Arrangement_2 arr;
	Kernel::Iso_rectangle_2	rect;
	PointColorCalculatorAlgorithm algorithm;
	CGAL::Geomview_stream *gv;
	std::list<std::pair<Segment_2,int> > segments;

	public:
	PointColorCalculator(std::list<std::pair<Point_2,int> > &points, PointColorCalculatorAlgorithm algorithm)
	{
		this->algorithm = algorithm;
		Colored_segment	cv[points.size()];
		int index = 0;
		std::list<Point_2> points_only;
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
	}	


	std::list<std::pair<Point_2,int> > GetColors(std::list<Point_2> points)
	{
		std::list<std::pair<Point_2,int> > result_points; 
		std::list<Query_result> results;
		locate(arr, points.begin(), points.end(), std::back_inserter(results));
		Arrangement_2::Face_const_handle f;
		Arrangement_2::Halfedge_const_handle he;
		Traits_2::Compute_squared_distance_2 squared_distance;
		std::list<Query_result>::const_iterator  res_iter;
		for (res_iter = results.begin(); res_iter != results.end(); ++res_iter)
		{
			Colored_segment *s;
			Number_type score[MAX_COLOR];
			int chosen_color = -2;
			if (CGAL::assign(f,res_iter->second) && !f->is_unbounded())
			{
				Point_2 p = res_iter->first; 
				chosen_color = -1; //we're in face, but it shouldn't be used
				if (!f->has_outer_ccb()) continue;
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
				result_points.push_back(std::pair<Point_2,int>(p, chosen_color));
			}
		}
		return result_points;
	}

	int to_qt(std::string file_name, int resolution_x, int resolution_y)
	{
		int color = -1;
		Number_type min_x = rect.xmin();
		Number_type min_y = rect.ymin();
		Number_type  max_x = rect.xmax();
		Number_type  max_y = rect.ymax();	
		
		QPointF q_topLeft(CGAL::to_double(min_x),CGAL::to_double(min_y));
		QPointF q_bottomRight(CGAL::to_double(max_x),CGAL::to_double(max_y));
		QRectF q_rect(q_topLeft,q_bottomRight);
		
		int argc = 1;
		char argv_name[] = "test";
		char* argv[] = {&argv_name[0]};
		QApplication app(argc,argv);

		QGraphicsScene scene(q_rect);
		QGraphicsView view(&scene);
		view.scale(resolution_x/q_rect.width(),resolution_y/q_rect.height());

		Number_type step_x = (max_x-min_x)/resolution_x;
		Number_type step_y = (max_y-min_y)/resolution_y;
		std::cout << "step_x: " << step_x << std::endl;
		std::cout << "step_y: " << step_y << std::endl;

		std::list<std::pair<QLineF,QColor> > q_lines;
		for (std::list<std::pair<Segment_2,int> >::iterator it = segments.begin(); it != segments.end();it++)
		{
			static int color_rgb[3];
			resolve_int_to_color((*it).second,color_rgb);
			QPointF source(CGAL::to_double((*it).first.source().x()), CGAL::to_double(((*it).first.source().y())));
			QPointF target(CGAL::to_double((*it).first.target().x()), CGAL::to_double(((*it).first.target().y())));
			QLineF line(source,target);
//			scene.addLine(line,QPen(QColor(color_rgb[0],color_rgb[1],color_rgb[2])));
			q_lines.push_back(std::pair<QLineF,QColor>(QLineF(source,target),QColor(color_rgb[0],color_rgb[1],color_rgb[2])));
		}
		std::cout << "min_x,max_x,min_y,max_y: " << min_x << "," << max_x << "," << min_y << "," << max_y << std::endl;
		std::list<Point_2> points;
		std::list<std::pair<QPointF,QColor> > q_points;
		for ( Number_type i = min_y;i<=max_y;i+=step_y)
		{
			for (Number_type j = min_x; j<=max_x;j+=step_x)
			{
				points.push_back(Point_2(j,i));
			}
		}
		
		std::list<std::pair<Point_2,int> > result_points = GetColors(points);

		std::cout << "got color" << std::endl;
		for (std::list<std::pair<Point_2, int> >::iterator iter = result_points.begin(); iter != result_points.end(); ++iter)
		{
			static int color_rgb[3];
			resolve_int_to_color(iter->second,color_rgb);
			if (color_rgb[0]==-1) continue;
			QColor color(color_rgb[0],color_rgb[1],color_rgb[2]);
			q_points.push_back(std::pair<QPointF,QColor>(QPointF(CGAL::to_double((*iter).first.x()),CGAL::to_double((*iter).first.y())),QColor(color_rgb[0],color_rgb[1],color_rgb[2])));
		}
		ColoredPointsGraphicsItem q_pointsItem(&q_lines, &q_points, &rect);
		scene.addItem(&q_pointsItem);
		view.show();
		QString q_file_name = file_name.c_str();
		QPixmap pix_map = QPixmap::grabWidget(&view);
		pix_map.save(q_file_name);
		return app.exec();


	}

	void resolve_int_to_color(int color, int color_rgb[3])
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
			
			case 3:
				color_rgb[0] = 255;
				color_rgb[1] = 255;
				color_rgb[2] = 0;
				break;
			
			case 4:
				color_rgb[0] = 255;
				color_rgb[1] = 105;
				color_rgb[2] = 180;
				break;

			case 5:
				color_rgb[0] = -1;
				color_rgb[1] = -1;
				color_rgb[2] = -1;
				break;

			default:
				color_rgb[0]=-1;
				color_rgb[1]=-1;
				color_rgb[2]=-1;
		}
	}
};
