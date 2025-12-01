#ifndef EZ_XWIN_HEADER
#define EZ_XWIN_HEADER

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>

#include "data_structures.h"

extern int GRAPHICAL_DISPLAY;

#define EZ_XWIN_WHITE  0
#define EZ_XWIN_BLACK  1
#define EZ_XWIN_RED    2
#define EZ_XWIN_BLUE   3
#define EZ_XWIN_GREEN  4
#define EZ_XWIN_BROWN  5
#define EZ_XWIN_AZURE  6
#define EZ_XWIN_PURPLE 7
#define EZ_XWIN_ORANGE 8
#define EZ_XWIN_PLUM   9
#define EZ_XWIN_NUM_COLORS 10


extern Display *display;
extern int screen_num;
extern Window win;
extern GC gc;
extern unsigned int width;
extern unsigned int height;    

/* These functions should be declared to provide the interface into this wrapper */
extern void ezXWinInit(void);
extern void ezHandleIdleEvent(Display *display, int screen_num, Window win,
                 GC gx, unsigned int width, unsigned int height);
extern void ezHandleDrawRequest(Display *display, int screen_num, Window win, 
				GC gc, unsigned int width, unsigned int height);
extern void ezHandleButtonPress(Display *display, int screen_num, Window win, 
				GC gc, unsigned int width, unsigned int height,
				int buttonX, int buttonY);
extern void ezHandleKeyPress(Display *display, int screen_num, Window win, 
			     GC gc, unsigned int width, unsigned int height, char c);

/* These functions are available */
unsigned long elaspedIdleTime(struct timeval cur, struct timeval last);
void ezXWinDrawText(unsigned int win_width, unsigned int win_height, char *txt);

void ezXWinDrawLineSegments(lineseg2d_t *linesegs, int nlines, int color);
void ezXWinDrawLineSegments2(double *pts, int nlines, int color);

void ezXWinDrawCircles(double *pts, double radius, int ncircles, int color);
void ezXWinDrawCircles2(double *pts, double *radii, int ncircles, int color);

void ezXWinFillCircles(double *ptsX, double *ptsY, double radius, int ncircles, int color);
void ezXWinFillCircles2(double *ptsX, double *ptsY, double *radii, int ncircles, int color);

void ezXWinFillRectangle(double x, double y, double width, double height, int color);
void ezXWinDrawRectangle(double x, double y, double width, double height, int color);

#endif
