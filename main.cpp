#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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

GC gc, gc_yellow, gc_red, gc_grey;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_red_values, gc_grey_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;

void createWindow(int argc, char **argv);

int main(int argc, char **argv){
	int qx,qy,px,py,rx,ry;
	FILE * fp;

	if(argc != 2){
		printf("Please pass input file as an arguement\n");
		return 1;
	}

	fp = fopen (argv[1],"r");
	fscanf (fp,"V (%d,%d) (%d,%d) (%d,%d)", &qx,&qy,&px,&py,&rx,&ry);
	fclose (fp);
	printf ("I have read: q = (%d,%d), p = (%d,%d), r = (%d,%d)\n",qx,qy,px,py,rx,ry);

	createWindow(argc, argv);

	return 0;
}

void createWindow(int argc, char **argv){
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
	win_width = display_width/2;
	win_height = (int) (win_width / 1.7); //rectangular window
  
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
            	  ExposureMask | StructureNotifyMask | ButtonPressMask );
  
	// finally: put window on screen 
	XMapWindow( display_ptr, win );

	XFlush(display_ptr);
	sleep(10);
}

