#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "triangle.h"
#include <vector>
#include <queue>

Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;

Window win;
int border_width;
unsigned int win_width, win_height;
int win_x, win_y;

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "Example Window";
char *icon_name_string = "Icon for Example Window";

XEvent report;

GC gc, gc_yellow, gc_red, gc_grey, gc_white;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_red_values, gc_grey_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;

XFontStruct *font;
XTextItem ti[1];

void createWindow(int argc, char **argv);
void drawObstacles(std::vector<Triangle> obstacles);
int collisionCheck(Triangle A, Triangle B);

int main(int argc, char **argv)
{
	int qx,qy,px,py,rx,ry;
	int startX, startY, startAngle, targetX, targetY, targetAngle;
	std::vector<Triangle> obstacles;
	FILE * fp;

	int xOffset, yOffset;

	if(argc != 2){
		printf("Please pass input file as an arguement\n");
		return 1;
	}

	fp = fopen (argv[1],"r");
	fscanf (fp,"V (%d,%d) (%d,%d) (%d,%d)\n", &px,&py,&qx,&qy,&rx,&ry);
	fscanf (fp,"S (%d,%d) %d\n", &startX, &startY, &startAngle);
	fscanf (fp,"T (%d,%d) %d", &targetX, &targetY, &targetAngle);
	Vehicle V(px,py,qx,qy,rx,ry,startX,startY,startAngle);
	Vehicle target(px,py,qx,qy,rx,ry,targetX,targetY,targetAngle);

	while(fscanf(fp,"\nO (%d,%d) (%d,%d) (%d,%d)", &px,&py,&qx,&qy,&rx,&ry) != EOF)
	{
		obstacles.push_back(Triangle(px,py,qx,qy,rx,ry));
		printf("obstacles[%d]: q = (%d,%d), p = (%d,%d), r = (%d,%d)\n",obstacles.size()-1,
										px,py,qx,qy,rx,ry);
	}
	printf("Number of obstacles = %d\n", obstacles.size());

	fclose (fp);

	printf ("I have read: q = (%d,%d), p = (%d,%d), r = (%d,%d)\n",px,py,qx,qy,rx,ry);
	printf("startX = %d, startY = %d, startAngle = %d\n", startX, startY, startAngle);
	printf("targetX = %d, targetY = %d, targetAngle = %d\n", targetX, targetY, targetAngle);

	createWindow(argc, argv);
	do
	{
		XNextEvent( display_ptr, &report);
	}while(report.type != Expose);

	drawObstacles(obstacles);
	V.setPosition(startX,startY,startAngle);
	XFillPolygon(display_ptr,win, gc, V.points, 3, 2,0);
	XDrawPoint(display_ptr, win, gc_yellow,  V.center.x, V.center.y);
	XDrawLine(display_ptr, win, gc_red, target.points[0].x, target.points[0].y,
                   				target.points[1].x, target.points[1].y);
	XDrawLine(display_ptr, win, gc_red, target.points[1].x, target.points[1].y,
                   				target.points[2].x, target.points[2].y);
	XDrawLine(display_ptr, win, gc_red, target.points[2].x, target.points[2].y,
                   				target.points[0].x, target.points[0].y);
	XFlush(display_ptr);
	XFillPolygon(display_ptr,win, gc_white, V.points, 3, 2,0);

	// Search for collisions
	struct Position
	{
		short i, j, k;
		Position(int i, int j, int k){this->i = i;this->j = j;this->k = k;};
	};
	struct Vertex{bool visited = 0; short prevI, prevJ, prevK;};
	Vertex visited[100][100][36];
	short startPosI = startX / 5;
	short startXshift = startX % 5;
	short startPosJ = startY / 5;
	short startYshift = startY % 5;
	short startPosK = (startAngle + 360)%360 / 10;
	short startAngleshift = (startAngle + 360)%360 % 10;
	short targetPosI = targetX / 5;
	short targetPosJ = targetY / 5;
	short targetPosK = targetAngle%360 / 10;

	int collisionFound;
	for(int i = 0; i < 100; i++)
	{
		for(int j = 0; j < 100; j++)
		{
			for(int k = 0; k < 36; k++)
			{
				collisionFound = 0;
				// Check for collisions at the current vertex
				for(int ob = 0; ob < obstacles.size()  &&  collisionFound == 0; ob++)
				{
					V.setPosition(i*5 + startXshift,j*5 + startYshift,k*10 + startAngleshift);
					if(collisionCheck(V,obstacles[ob]))
					{
						visited[i][j][k].visited = 1;
						collisionFound = 1;
					}
				}
			}
		}
	}
	// End of collision check

	// BFS START
	std::queue<Position> BFSqueue;

	printf("startPosI = %d, startPosJ = %d, startPosK = %d\n", startPosI,startPosJ,startPosK);
	printf("targetPosI = %d, targetPosJ = %d, targetPosK = %d\n",targetPosI,targetPosJ,targetPosK);

	int found = 0;
	Position currPos(startPosI,startPosJ,startPosK);
	Position prevPos = currPos;
	BFSqueue.push(currPos);
	BFSqueue.push(currPos);
	while(found == 0  &&  !BFSqueue.empty())
	{
		prevPos = BFSqueue.front();
		BFSqueue.pop();
		currPos = BFSqueue.front();
		BFSqueue.pop();

		if(visited[currPos.i][currPos.j][currPos.k].visited == 0)
		{
			visited[currPos.i][currPos.j][currPos.k].prevI = prevPos.i;
			visited[currPos.i][currPos.j][currPos.k].prevJ = prevPos.j;
			visited[currPos.i][currPos.j][currPos.k].prevK = prevPos.k;

			if(currPos.i == targetPosI  &&  currPos.j == targetPosJ  &&  currPos.k == targetPosK)
			{
				found = 1;
			}
			else
			{
				visited[currPos.i][currPos.j][currPos.k].visited = 1;

				if(currPos.i > 0)
				{
					BFSqueue.push(currPos);
					BFSqueue.push(Position(currPos.i - 1,currPos.j,currPos.k));
				}
				if(currPos.i < 99)
				{
					BFSqueue.push(currPos);
					BFSqueue.push(Position(currPos.i + 1,currPos.j,currPos.k));
				}
				if(currPos.j > 0)
				{
					BFSqueue.push(currPos);
					BFSqueue.push(Position(currPos.i,currPos.j - 1,currPos.k));
				}
				if(currPos.j < 99)
				{
					BFSqueue.push(currPos);
					BFSqueue.push(Position(currPos.i,currPos.j + 1,currPos.k));
				}

				BFSqueue.push(currPos);
				BFSqueue.push(Position(currPos.i,currPos.j,(currPos.k-1+36) % 36));
				BFSqueue.push(currPos);
				BFSqueue.push(Position(currPos.i,currPos.j,(currPos.k+1+36) % 36));
			}
		}
	}
	printf("PATHFOUND STATUS = %d\n", found);
	// BFS END

	std::vector<Position> shortestPath;
	Vertex currVertex = visited[currPos.i][currPos.j][currPos.k];
	shortestPath.push_back(currPos);
	while(currPos.i != startPosI  ||  currPos.j != startPosJ  ||  currPos.k != startPosK)
	{
		currPos = Position(currVertex.prevI, currVertex.prevJ, currVertex.prevK);
		currVertex = visited[currPos.i][currPos.j][currPos.k];
		shortestPath.push_back(currPos);
	}

	if(found == 0)
	{
		font = XLoadQueryFont(display_ptr, "7x14");
		ti[0].chars = "Path cannot be found.";
		ti[0].nchars = 21;
		ti[0].delta = 0;
		ti[0].font = font->fid;
		XDrawText(display_ptr, win, gc_red, 
					(win_width-XTextWidth(font, ti[0].chars, ti[0].nchars))/2,
					(win_height-(font->ascent+font->descent))/2+font->ascent, ti, 1);
		XUnloadFont(display_ptr, font->fid);
		XFlush(display_ptr);
	}
	else
	{
		// Draw animation
		sleep(1);
		for(int i = shortestPath.size()-1; i >= 0; i--)
		{
			printf("Vehicle: I = %d, J = %d, K = %d\n", shortestPath[i].i,shortestPath[i].j,shortestPath[i].k);
			V.setPosition(shortestPath[i].i*5 + startXshift,shortestPath[i].j*5 + startYshift,shortestPath[i].k*10 + startAngleshift);
			XFillPolygon(display_ptr,win, gc, V.points, 3, 2,0);
			XDrawPoint(display_ptr, win, gc_yellow,  V.center.x, V.center.y);
			XFlush(display_ptr);
			usleep(200000);
			XFillPolygon(display_ptr,win, gc_white, V.points, 3, 2,0);
			XDrawLine(display_ptr, win, gc_red, target.points[0].x, target.points[0].y,
	                   				target.points[1].x, target.points[1].y);
			XDrawLine(display_ptr, win, gc_red, target.points[1].x, target.points[1].y,
	                   				target.points[2].x, target.points[2].y);
			XDrawLine(display_ptr, win, gc_red, target.points[2].x, target.points[2].y,
	                   				target.points[0].x, target.points[0].y);
		}
	}
	sleep(4);

	XDestroyWindow(display_ptr, win);
    XCloseDisplay(display_ptr);	

	return 0;
}

//////////////////////////////////////////////////////////////////
////////////////////////// MAIN END //////////////////////////////
//////////////////////////////////////////////////////////////////

int orientation(XPoint A, XPoint B, XPoint C)
{
	int determinent = A.x*B.y + B.x*C.y + C.x*A.y - A.y*B.x - B.y*C.x - C.y*A.x;

	if (determinent < 0)
		// CLOCKWISE
		return -1;
	else if (determinent > 0)
		// COUNTER-CLOCKWISE
		return 1;
	else
		// COLINEAR
		return 0;
}

int intersectionCheck(XPoint A1, XPoint A2, XPoint B1, XPoint B2)
{
	// Check if line segments A and B intersect
	if(orientation(A1,A2,B1) * orientation(A1,A2,B2) < 0 && 
	   orientation(B1,B2,A1) * orientation(B1,B2,A2) < 0)
		return 1;
	else
		return 0;
}

int collisionCheck(Triangle A, Triangle B)
{
	// Check all sides for intersections
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			if(intersectionCheck(A.points[i],A.points[(i+1)%3],B.points[j],B.points[(j+1)%3]))
			{
				return 1;
			}
		}
	}

	// Check if one triangle contains the other
	if(orientation(A.points[0],A.points[1],B.points[0]) == orientation(A.points[1],A.points[2],B.points[0])  &&
	   orientation(A.points[0],A.points[1],B.points[0]) == orientation(A.points[2],A.points[0],B.points[0]))
	{
		return 1;
	}
	if(orientation(B.points[0],B.points[1],A.points[0]) == orientation(B.points[1],B.points[2],A.points[0])  &&
	   orientation(B.points[0],B.points[1],A.points[0]) == orientation(B.points[2],B.points[0],A.points[0]))
	{
		return 1;
	}

	return 0;
}

void drawObstacles(std::vector<Triangle> obstacles)
{
	for(int i = 0; i < obstacles.size(); i++)
	{
		XFlush(display_ptr);
		XFillPolygon(display_ptr,win, gc_grey, obstacles[i].points, 3, 2,0);
	}
}

void createWindow(int argc, char **argv)
{
	// opening display: basic connection to X Server 
	if( (display_ptr = XOpenDisplay(display_name)) == NULL )
	{
		printf("Could not open display. \n");
		exit(-1);
	}

	printf("Connected to X server  %s\n", XDisplayName(display_name) );
	screen_num = DefaultScreen( display_ptr );
	screen_ptr = DefaultScreenOfDisplay( display_ptr );
	color_map  = XDefaultColormap( display_ptr, screen_num );
	display_width  = DisplayWidth( display_ptr, screen_num );
	display_height = DisplayHeight( display_ptr, screen_num );

	printf("Width %d, Height %d, Screen Number %d\n", 
			display_width, display_height, screen_num);

	// creating the window 
	border_width = 10;
	win_x = 0; win_y = 0;
	win_width = 500;
	win_height = 500;
  
	win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num),
                              win_x, win_y, win_width, win_height, border_width,
                              BlackPixel(display_ptr, screen_num),
                              WhitePixel(display_ptr, screen_num) );

	// now try to put it on screen, this needs cooperation of window manager
	size_hints = XAllocSizeHints();
	wm_hints = XAllocWMHints();
	class_hints = XAllocClassHint();
	if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
	{
		printf("Error allocating memory for hints. \n"); exit(-1);
	}

	size_hints -> flags = PPosition | PSize | PMinSize  ;
	size_hints -> min_width = 60;
	size_hints -> min_height = 60;

	XStringListToTextProperty( &win_name_string,1,&win_name);
	XStringListToTextProperty( &icon_name_string,1,&icon_name);
	  
	wm_hints -> flags = StateHint | InputHint ;
	wm_hints -> initial_state = NormalState;
	wm_hints -> input = False;

	class_hints -> res_name = "x_use_example";
	class_hints -> res_class = "examples";

	XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc,
                      size_hints, wm_hints, class_hints );

	// what events do we want to receive
	XSelectInput( display_ptr, win, 
            	  ExposureMask);// | StructureNotifyMask | ButtonPressMask );
  
	// finally: put window on screen 
	XMapWindow( display_ptr, win );

	XFlush(display_ptr);

	// create graphics context, so that we may draw in this window
	gc = XCreateGC( display_ptr, win, valuemask, &gc_values);
	XSetForeground( display_ptr, gc, BlackPixel( display_ptr, screen_num ) );
	XSetLineAttributes( display_ptr, gc, 4, LineSolid, CapRound, JoinRound);

	gc_white = XCreateGC( display_ptr, win, valuemask, &gc_values);
	XSetForeground( display_ptr, gc_white, WhitePixel( display_ptr, screen_num ) );
	XSetLineAttributes( display_ptr, gc_white, 4, LineSolid, CapRound, JoinRound);

	// and three other graphics contexts, to draw in yellow and red and grey
 	gc_yellow = XCreateGC( display_ptr, win, valuemask, &gc_yellow_values);
	XSetLineAttributes(display_ptr, gc_yellow, 6, LineSolid,CapRound, JoinRound);
	if( XAllocNamedColor(display_ptr, color_map, "yellow", &tmp_color1, &tmp_color2 ) == 0 )
	{
    	printf("failed to get color yellow\n"); exit(-1);
	}
	else
		XSetForeground( display_ptr, gc_yellow, tmp_color1.pixel );

	gc_red = XCreateGC( display_ptr, win, valuemask, &gc_red_values);
	if( XAllocNamedColor( display_ptr, color_map, "red", &tmp_color1, &tmp_color2 ) == 0 )
	{
    	printf("failed to get color red\n"); exit(-1);
	}
	else
    	XSetForeground( display_ptr, gc_red, tmp_color1.pixel );

	gc_grey = XCreateGC( display_ptr, win, valuemask, &gc_grey_values);
	if( XAllocNamedColor( display_ptr, color_map, "light grey", &tmp_color1, &tmp_color2 ) == 0 )
	{
    	printf("failed to get color grey\n"); exit(-1);
	}
	else
    	XSetForeground( display_ptr, gc_grey, tmp_color1.pixel );
}

