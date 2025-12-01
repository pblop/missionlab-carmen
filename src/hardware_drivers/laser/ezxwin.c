/*EZ - X server interface*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>

#include "ezxwin.h" 
#include "data_structures.h" 
#include "laser_protocol.h"

// ENDO
//#define X_DISPLAY_TRANSFORM(x) ((width/2.0 + (x)) * 1.0)
//#define Y_DISPLAY_TRANSFORM(y) ((height - (y) - 250) * 1.0)
#define X_DISPLAY_TRANSFORM(x) ((width/2.0 + (x)) * 0.75)
#define Y_DISPLAY_TRANSFORM(y) ((height - (y) - 250) * 0.75)

#define SCREEN_DEPTH 16
int IDLE_USECS = 100;

int GRAPHICAL_DISPLAY = 1;
extern char *optarg;
char gsInputFile[128];

/* These are used as arguments to nearly every Xlib routine, so it saves 
 * routine arguments to declare them global.  If there were 
 * additional source files, they would be declared extern there. 
 * -jdiaz- I prefer to declare them static and pass them along */
Display *display;
int screen_num;
Window win;
GC gc;
static XFontStruct *font_info;
unsigned int width, height;	/* window size */
static char *progname; /* name this program was invoked by */
Widget          drawing_area;
Colormap colormap;


char *colors[] = {"white", "black", "red", "blue", "green", "brown",
		  "cyan", "purple", "orange", "plum"};

unsigned long   thePixels[EZ_XWIN_NUM_COLORS];

void ezXWinDrawText(unsigned int win_width, unsigned int win_height, char *txt)
{
  int len1;
  int width1;
  int font_height;
   
  /* need length for both XTextWidth and XDrawString */
  len1 = strlen(txt);
  
  /* get string widths for centering */
  width1 = XTextWidth(font_info, txt, len1);
  
  font_height = font_info->ascent + font_info->descent;
  
  /* output text, centered on each line */
  XDrawString(display, win, gc, win_width - width1/2, 
	      win_height + font_height, txt, len1);
}

void initColors()
{
   XColor       theRGBColor, theHardwareColor;
   int          status;
   int          i;
   
   for(i = 0; i < EZ_XWIN_NUM_COLORS; i++) {
     status = XLookupColor(display, colormap, colors[i],
			   &theRGBColor, &theHardwareColor);
     if (status != 0) {
       status = XAllocColor(display, colormap, &theHardwareColor);
       thePixels[i] = theHardwareColor.pixel;
     }
     else thePixels[i] = BlackPixel(display,screen_num);
   }
}

void ezXWinSetColor(int color)                    
{
   if (color >= EZ_XWIN_NUM_COLORS || color < 0) {
      printf("Wrong color: %d\n", color);
      return;
   }
   XSetForeground(display, gc, thePixels[color]);
}


void ezXWinDrawLineSegments(lineseg2d_t *linesegs, int nlines, int color)
{
    int i;
    XSegment *segments;
    segments = (XSegment *)malloc(sizeof(XSegment)*nlines);
    for (i = 0; i < nlines; i++) {
        segments[i].x1 = (int)(X_DISPLAY_TRANSFORM(linesegs->x1));
        segments[i].y1 = (int)(Y_DISPLAY_TRANSFORM(linesegs->y1));
        segments[i].x2 = (int)(X_DISPLAY_TRANSFORM(linesegs->x2));
        segments[i].y2 = (int)(Y_DISPLAY_TRANSFORM(linesegs->y2));
        linesegs++;
    }
  
    ezXWinSetColor(color);

    XDrawSegments(display, win, gc, segments, nlines);
    free(segments);
}

void ezXWinDrawCircles(double *pts, double radius, int ncircles, int color)
{
    int i;
    XArc *arcs;
    arcs = (XArc *)malloc(sizeof(XArc)*ncircles);
  
    for (i = 0; i < ncircles; i++) {
        arcs[i].x = (int)(X_DISPLAY_TRANSFORM(*pts++ - radius));
        arcs[i].y = (int)(Y_DISPLAY_TRANSFORM(*pts++ + radius));
        arcs[i].width = arcs[i].height = (int)(radius * 2.0);
        arcs[i].angle1 = 0;
        arcs[i].angle2 = 23040; /* degrees * 64 = 360 degrees*/ 
    }

    ezXWinSetColor(color);
    XDrawArcs(display, win, gc, arcs, ncircles);
    free(arcs);
}

/*Same as above but allows specification of individual radii */
void ezXWinDrawCircles2(double *pts, double *radii, int ncircles, int color)
{
    int i;
    XArc *arcs;
    arcs = (XArc *)malloc(sizeof(XArc)*ncircles);
    for (i = 0; i < ncircles; i++) {
        arcs[i].x = (int)(X_DISPLAY_TRANSFORM(*pts++ - *radii));
        arcs[i].y = (int)(Y_DISPLAY_TRANSFORM(*pts++ + *radii));
        arcs[i].width = arcs[i].height = (int)(2*(*radii++));
        arcs[i].angle1 = 0;
        arcs[i].angle2 = 23040; /* degrees * 64 = 360 degrees*/ 
    }

    ezXWinSetColor(color);
    XDrawArcs(display, win, gc, arcs, ncircles);
    free(arcs);
}



void ezXWinFillCircles(double *ptsX, double *ptsY, double radius, int ncircles, int color)
{
    int i;
    XArc *arcs;
    arcs = (XArc *)malloc(sizeof(XArc)*ncircles);
    for (i = 0; i < ncircles; i++) {
        arcs[i].x = (int)(X_DISPLAY_TRANSFORM(*ptsX++ - radius));
        arcs[i].y = (int)(Y_DISPLAY_TRANSFORM(*ptsY++ + radius));
        arcs[i].width = arcs[i].height = (int)(radius * 2.0);
        arcs[i].angle1 = 0;
        arcs[i].angle2 = 23040; /* degrees * 64 = 360 degrees*/ 
    }

    ezXWinSetColor(color);
    XFillArcs(display, win, gc, arcs, ncircles);
    free(arcs);
}

/*Same as above but allows specification of individual radii */
void ezXWinFillCircles2(double *ptsX, double *ptsY, double *radii, int ncircles, int color)
{
    int i;
    XArc *arcs;
    arcs = (XArc *)malloc(sizeof(XArc)*ncircles);
    for (i = 0; i < ncircles; i++) {
        arcs[i].x = (int)(X_DISPLAY_TRANSFORM(*ptsX++ - *radii));
        arcs[i].y = (int)(Y_DISPLAY_TRANSFORM(*ptsY++ + *radii));
        arcs[i].width = arcs[i].height = (int)((*radii++) * 2.0);
        arcs[i].angle1 = 0;
        arcs[i].angle2 = 23040; /* degrees * 64 = 360 degrees*/ 
    }

    ezXWinSetColor(color);
    XFillArcs(display, win, gc, arcs, ncircles);
    free(arcs);
}

void ezXWinFillRectangle(double x, double y, double width, double height, int color)
{
    if (color == EZ_XWIN_BLACK) 
        XSetForeground(display, gc, BlackPixel(display,screen_num));
    else 
        XSetForeground(display, gc, WhitePixel(display,screen_num));
    XFillRectangle(display, win, gc, (int)x, (int)y, (int)width, (int)height);
}

void ezXWinDrawRectangle(double x, double y, double width, double height, int color)
{
    if (color == EZ_XWIN_BLACK) 
        XSetForeground(display, gc, BlackPixel(display,screen_num));
    else 
        XSetForeground(display, gc, WhitePixel(display,screen_num));
    XDrawRectangle(display, win, gc, (int)x, (int)y, (int)width,  (int)height);
}

/*  determines the difference in microseconds between cur and last */
unsigned long elaspedIdleTime(struct timeval cur, struct timeval last)
{
  unsigned long ans;
  ans = 1000000 * (cur.tv_sec - last.tv_sec);
  ans += (cur.tv_usec - last.tv_usec);
  return ans;
}

void load_font(XFontStruct **font_info)
{
  char *fontname = "9x15";
  
  /* Load font and get font information structure. */
  if ((*font_info = XLoadQueryFont(display,fontname)) == NULL)
    {
      (void) fprintf( stderr, "%s: Cannot open 9x15 font\n", 
		      progname);
      exit( -1 );
    }
}

void getGC(
    Window win,
    GC *gc,
    XFontStruct *font_info)
{
  unsigned long valuemask = 0; /* ignore XGCvalues and use defaults */
  XGCValues values;
  unsigned int line_width = 1;
  int line_style = LineSolid;
  int cap_style = CapRound;
  int join_style = JoinRound;
  
  /* Create default Graphics Context */
  *gc = XCreateGC(display, win, valuemask, &values);
  
  /* specify font */
  XSetFont(display, *gc, font_info->fid);
  
  /* specify black foreground since default window background is 
   * white and default foreground is undefined. */
  XSetForeground(display, *gc, BlackPixel(display,screen_num));
  
  /* set line attributes */
  XSetLineAttributes(display, *gc, line_width, line_style, 
		     cap_style, join_style);
  
  /* set dashes */
  /* XSetDashes(display, *gc, dash_offset, dash_list, list_length);*/
}

int ezXMainLoopG(void) {
    int wx, wy; 	/* window position */
    unsigned int border_width = 4;	/* four pixels */
    unsigned int display_width, display_height;
    char *window_name = "EZ - X Windows";
    char *icon_name = "EZ - X Windows";
  
    XSizeHints *size_hints;
    XIconSize *size_list;
    XWMHints *wm_hints;
    XClassHint *class_hints;
    XTextProperty windowName, iconName;
    int count;
    XEvent event;

    char *display_name = NULL;
    struct timeval tv, last_tv;
    struct timezone tz;
    char buffer[20];
    int bufsize = 20;
    KeySym key;
    XComposeStatus compose;
    int charcount;
    unsigned long event_mask;
    unsigned long diff;
    int buttonX,buttonY;
    int rootX,rootY;
    Window win1,win2;
    unsigned int maskRtn;
  

    if (!(size_hints = XAllocSizeHints())) {
        fprintf(stderr, "%s: failure allocating memory\n", progname);
        exit(0);
    }
    if (!(wm_hints = XAllocWMHints())) {
        fprintf(stderr, "%s: failure allocating memory\n", progname);
        exit(0);
    }
    if (!(class_hints = XAllocClassHint())) {
        fprintf(stderr, "%s: failure allocating memory\n", progname);
        exit(0);
    }
  
  

    /* connect to X server */
    if ( (display=XOpenDisplay(display_name)) == NULL )
    {
        (void) fprintf( stderr, "%s: cannot connect to X server %s\n", 
                        progname, XDisplayName(display_name));
        exit( -1 );
    }

    /* get screen size from display structure macro */
    screen_num = DefaultScreen(display);
    display_width = DisplayWidth(display, screen_num);
    display_height = DisplayHeight(display, screen_num);

    /* Note that in a real application, x and y would default to 0
     * but would be settable from the command line or resource database.  
     */
    wx = wy = 0;

    /* size window with enough room for text */
    width = (int)((double)display_width/1.5);
    height = (int)((double)display_height/1.5);

    /* create opaque window */
    win = XCreateSimpleWindow(display, RootWindow(display,screen_num), 
                              wx, wy, width, height, border_width, 
                              BlackPixel(display, screen_num), WhitePixel(display,screen_num));

    /* Get available icon sizes from Window manager */

    if (XGetIconSizes(display, RootWindow(display,screen_num), 
                      &size_list, &count) == 0)
        (void) fprintf( stderr, 
                        "%s: Window manager didn't set icon sizes - using default.\n", progname);
    else {
        ;
        /* A real application would search through size_list
         * here to find an acceptable icon size, and then
         * create a pixmap of that size.  This requires
         * that the application have data for several sizes
         * of icons. */
    }


   
    /* Set size hints for window manager.  The window manager may
     * override these settings.  Note that in a real
     * application if size or position were set by the user
     * the flags would be UPosition and USize, and these would
     * override the window manager's preferences for this window. */
  
    /* x, y, width, and height hints are now taken from
     * the actual settings of the window when mapped. Note
     * that PPosition and PSize must be specified anyway. */
  
    size_hints->flags = PPosition | PSize | PMinSize;
    size_hints->min_width = 300;
    size_hints->min_height = 200;
  
    /* These calls store window_name and icon_name into
     * XTextProperty structures and set their other 
     * fields properly. */
    if (XStringListToTextProperty(&window_name, 1, &windowName) == 0) {
        (void) fprintf( stderr, "%s: structure allocation for windowName failed.\n", 
                        progname);
        exit(-1);
    }
  
    if (XStringListToTextProperty(&icon_name, 1, &iconName) == 0) {
        (void) fprintf( stderr, "%s: structure allocation for iconName failed.\n", 
                        progname);
        exit(-1);
    }
  
    wm_hints->initial_state = NormalState;
    wm_hints->input = True;

    wm_hints->flags = StateHint | IconPixmapHint | InputHint;
  
    class_hints->res_name = progname;
    class_hints->res_class = "Basicwin";
  
    XSetWMProperties(display, win, &windowName, &iconName, 
                     &progname, 1, size_hints, wm_hints, 
                     class_hints);
  
    /* Select event types wanted */
    XSelectInput(display, win, ExposureMask | KeyPressMask | 
                 ButtonPressMask | StructureNotifyMask);
  

    load_font(&font_info);
  
    /* create GC for text and drawing */
    getGC(win, &gc, font_info);
  
    /* Display window */
    XMapWindow(display, win);
  
    event_mask = ExposureMask|KeyPressMask|ButtonPressMask|StructureNotifyMask;
  
    /*  plane = XCreatePixmap(display,win,width,height,SCREEN_DEPTH);
        XSetForeground(display, gc, WhitePixel(display,screen_num));
        XFillRectangle(display,plane,gc,0,0,width,height);
    */

    XSetForeground(display, gc, BlackPixel(display,screen_num));

    /*  drawing_area = XtVaCreateWidget("drawing-area", xmDrawingAreaWidgetClass, scrolled_window,
        XmNresizePolicy, XmRESIZE_NONE,
        XmNwidth,        width,
        XmNheight,       height,
        NULL);
    */
    gettimeofday(&last_tv, &tz);
  
    /* get events, use first to display text and graphics */

    colormap   = XDefaultColormap(display, screen_num );
    initColors();


    /* Call the user defined init function */

  
    ezXWinInit();

    while (1)  {
        usleep(IDLE_USECS);
        gettimeofday(&tv, &tz);
        diff = elaspedIdleTime(tv, last_tv);
        if ((int)diff > IDLE_USECS) {
            /* Call the user defined idle function */
            ezHandleIdleEvent(display, screen_num, win, gc, width, height);
            last_tv.tv_sec = tv.tv_sec;
            last_tv.tv_usec = tv.tv_usec;
        }
        if (XCheckMaskEvent(display,event_mask,&event))
            switch  (event.type) {
            case Expose:
                /* unless this is the last contiguous expose,
                 * don't draw the window */
                if (event.xexpose.count != 0)
                    break;
                ezHandleDrawRequest(display, screen_num, win, gc, width, height);
                break;
            case ConfigureNotify:
                /* window has been resized, change width and
                 * height to send to draw_text and draw_graphics
                 * in next Expose */
                width = event.xconfigure.width;
                height = event.xconfigure.height;
	
                /*XFreePixmap(display,plane);
                  plane = XCreatePixmap(display,win,width,height,SCREEN_DEPTH);
                  XSetForeground(display, gc, WhitePixel(display,screen_num));
                  XFillRectangle(display,plane,gc,0,0,width,height);
                  XSetForeground(display, gc, BlackPixel(display,screen_num));
                */
                break;
            case ButtonPress:
                XQueryPointer(
                    display,
                    win,
                    &win1,
                    &win2,
                    &rootX,
                    &rootY,
                    &buttonX,
                    &buttonY,
                    &maskRtn);
                /* button x and y are the locations */
                ezHandleButtonPress(display, screen_num, win, gc, width, height, buttonX, buttonY);
                break;
            case KeyPress:
                charcount = XLookupString((XKeyEvent *)&event,buffer,bufsize,&key,&compose);
                /* buffer[0] holds the key pressed */
                ezHandleKeyPress(display, screen_num, win, gc, width, height, buffer[0]);
                break;
            default:
                /* all events selected by StructureNotifyMask
                 * except ConfigureNotify are thrown away here,
                 * since nothing is done with them */
                break;
            } /* end switch */
    } /* end while */

    return 1;
}
int ezXMainLoopNG(void) {
  unsigned long diff;
  struct timeval tv, last_tv;
  struct timezone tz;
  
  ezXWinInit();

  while (1)  {
    usleep(IDLE_USECS);
    gettimeofday(&tv, &tz);
    diff = elaspedIdleTime(tv, last_tv);
    if ((int)diff > IDLE_USECS) {
      /* Call the user defined idle function */
      ezHandleIdleEvent(display, screen_num, win, gc, width, height);
      last_tv.tv_sec = tv.tv_sec;
      last_tv.tv_usec = tv.tv_usec;
    }
  }
  return 1;
}

int main(int argc, char **argv)
{
  int c;
  progname = argv[0];

  while ((c = getopt(argc, argv, "gcmrnd:h") ) != EOF) {
    switch (c) {
    case 'g':
      GRAPHICAL_DISPLAY = 1;
      break;
    case 'n':
      GRAPHICAL_DISPLAY = 0;
      break;
    case 'c':
      COLLECT_LASER_DATA_TO_FILE = 1;
      break;
    case 'm':
      COLLECT_LASER_DATA_TO_FILE_MANUAL = 1;
      break;
    case 'r':
      REAL_LASER_READ = 1;
      break;
    case 'd':
      REAL_LASER_READ = 0;
      COLLECT_LASER_DATA_TO_FILE = 0;
      IDLE_USECS = 100;
	  strcpy(gsInputFile,optarg);
      break;
    case 'h':
      printf("USAGE: %s [options]\n\t-h This help message.\n\t-g Turn on graphical display.\n", progname);
      printf("\t-n Turn off graphical display.\n\t-c Collect data to file.\n");
      printf("\t-m Manually collect data to file by pressing left mouse button.\n");
      printf("\t-r Read data from the laser.\n\t-d Read data from file.\n");
      exit(0);
    }
  }
  if (GRAPHICAL_DISPLAY) return ezXMainLoopG();
  else return ezXMainLoopNG();
}

