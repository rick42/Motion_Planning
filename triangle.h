#include <math.h>

#define PI 3.14159265

struct Triangle
{
 	Triangle(int, int, int, int, int, int);
 	XPoint points[3];
};

Triangle::Triangle (int px, int py, int qx, int qy, int rx, int ry)
{
 	points[0].x = px;
 	points[0].y = py;
 	points[1].x = qx;
 	points[1].y = qy;
 	points[2].x = rx;
 	points[2].y = ry;
}

/*******************************************************/

struct Vehicle : Triangle
{
	Vehicle(int, int, int, int, int, int, int, int, int);
 	XPoint center;
 	int angle = 0;
	XPoint initialPoints[3];
	void translate(int, int);
	void rotate(int);
	void setPosition(int,int,int);
};

Vehicle::Vehicle(int px, int py, int qx, int qy, int rx, int ry,
				 int startX, int startY, int startAngle) : Triangle(px, py, qx, qy, rx, ry)
{
	for(int i = 0; i < 3; i++)
	{
		initialPoints[i] = points[i];
	}

	center.x = 0;
	center.y = 0;

	translate(startX,startY);
	rotate(startAngle);

	printf("Before: %f     After: %d\n", 3.57, (int)round(3.57));
}

void Vehicle::translate(int xShift, int yShift)
{
	center.x += xShift;
	center.y += yShift;

	for (int i = 0; i < 3; i++)
	{
		points[i].x += xShift;
		points[i].y += yShift;
	}
}

void Vehicle::rotate(int angleShift)
{
	angle = (angle + angleShift);

	if(angle >= 360  ||  angle < 0)
		angle = (angle+360) % 360;

	double radian = angle * (PI / 180.0);

	if(angle == 0)
	{
		for(int i = 0; i < 3; i++)
		{
			points[i].x = initialPoints[i].x;
			points[i].y = initialPoints[i].y;
		}
	}
	else
	{
		for(int i = 0; i < 3; i++)
		{
			points[i].x = (int)round(initialPoints[i].x * cos(radian) - initialPoints[i].y * sin(radian));
			points[i].y = (int)round(initialPoints[i].x * sin(radian) + initialPoints[i].y * cos(radian));
		}
	}

	for(int i = 0; i < 3; i++)
	{
		points[i].x += center.x;
		points[i].y += center.y;
	}
}

void Vehicle::setPosition(int centerX, int centerY, int theta)
{
	for(int i = 0; i < 3; i++)
	{
		points[i] = initialPoints[i];
	}
	angle = 0;

	center.x = 0;
	center.y = 0;

	translate(centerX,centerY);
	rotate(theta);
}

