#include "common.h"

class ColoredPointsGraphicsItem : public QGraphicsRectItem
{
	typedef std::list<std::pair<QLineF,QColor> > list_type_lines;
	typedef std::list<std::pair<QPointF,QColor> > list_type_points;
	public:
	ColoredPointsGraphicsItem(list_type_lines *l_lines, list_type_points *l_points, Kernel::Iso_rectangle_2 * rect)
	{
		lines = l_lines;
		points = l_points;
		this->rect = rect;
	}

	protected:
	list_type_lines *lines;
	list_type_points *points;
	Kernel::Iso_rectangle_2 * rect;

	void paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
	{
		for (list_type_lines::iterator lines_it = lines->begin(); lines_it != lines->end(); lines_it++)
		{
			QPen pen((*lines_it).second);
			painter->setPen(pen);
			painter->drawLine((*lines_it).first);
		}
		for(list_type_points::iterator it = points->begin(); it != points->end(); it++)
		{
			QPen pen((*it).second);
			painter->setPen(pen);
			painter->drawPoint((*it).first);
		}
	}	
};
