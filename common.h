#ifndef _COMMON_H
#define _COMMON_H

#include <CGAL/Cartesian.h>

#include <CGAL/MP_Float.h>
#include <CGAL/Quotient.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/Arrangement_2.h>
#include "point_location_utils.h"
#include <CGAL/basic.h>
#include <CGAL/Arr_consolidated_curve_data_traits_2.h>
#include <CGAL/Segment_2.h>

typedef CGAL::Quotient<CGAL::MP_Float>	Number_type;
typedef CGAL::Cartesian<Number_type>	Kernel;
typedef CGAL::Arr_segment_traits_2<Kernel>	Traits_2;
typedef CGAL::Arr_consolidated_curve_data_traits_2<Traits_2,int> Data_traits;
typedef Traits_2::Curve_2 Segment_2;
typedef Traits_2::Point_2	Point_2;
typedef Traits_2::Point_3	Point_3;
typedef Data_traits::Curve_2	Colored_segment;
typedef CGAL::Arrangement_2<Data_traits>	Arrangement_2;
typedef CGAL::Arr_naive_point_location<Arrangement_2> Naive_pl;

const int MAX_COLOR = 6;
std::string color_names[MAX_COLOR] = { "0", "1", "2", "3", "4", "5"};

#include <list>
#include <utility>

#endif
