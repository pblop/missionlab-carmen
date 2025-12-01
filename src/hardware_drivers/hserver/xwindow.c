/**********************************************************************
 **                                                                  **
 **                              xwindow.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  X11 Interface for HServer                                       **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: xwindow.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <pthread.h>

#include "HServerTypes.h"
#include "hserver.h"
#include "xwindow.h"
//#include "eventnames.h"
#include "message.h"

Xwindow *xwindow = NULL;
char    *color_file = "./hserver_colors.txt";

const int Xwindow::SKIP_STATUSBAR_UPDATE_ = 10;
const int Xwindow::READER_THREAD_USLEEP_ = 1000;

int Xwindow::loadColors(){
    FILE *fp;
    int i;
    unsigned int r,g,b;
    char buf[512];
    const bool SET_COLORS_MANUALLY = true;

    if (!SET_COLORS_MANUALLY)
    {
        // color_file should contain:
        //
        // 5
        // 0 0 0 
        // 255 255 255
        // 255 0 0
        // 0 255 0
        // 0 0 255
        //
        fp = fopen(color_file,"r");
        if (fp == NULL)
        {
            sprintf(buf, "Xwindow Errror: Can't open \"%s\".", color_file);
            printTextWindow(buf);
            return false;
        }
        fscanf(fp,"%d",&i);
        num_colors = i;
        for(i=0;i<num_colors;i++){
            fscanf(fp,"%d%d%d",&r,&g,&b);
            colors[i] = b|(g<<8)|(r<<16);
        }
        fclose(fp);
    }
    else
    {
        r = 0;
        g = 0;
        b = 0;
        colors[0] = b|(g<<8)|(r<<16);

        r = 255;
        g = 255;
        b = 255;
        colors[1] = b|(g<<8)|(r<<16);

        r = 255;
        g = 0;
        b = 0;
        colors[2] = b|(g<<8)|(r<<16);

        r = 0;
        g = 255;
        b = 0;
        colors[3] = b|(g<<8)|(r<<16);

        r = 0;
        g = 0;
        b = 255;
        colors[4] = b|(g<<8)|(r<<16);
    }

    return true;
}

unsigned int Xwindow::RGBtoColor( int r, int g, int b )
{
    return ( b | (g << 8) | (r << 16) );
}
  


XImage* Xwindow::RGBToXImage( int w, int h, unsigned char* data )
{
    Status rtn;
    XImage *ximg = new XImage;
  
    ximg->width = w;
    ximg->height = h;
    ximg->xoffset = 0;
    ximg->format = ZPixmap;
    ximg->data = (char*)data;
  

    ximg->byte_order = LSBFirst;
    ximg->bitmap_unit = 8;
    ximg->bitmap_bit_order = LSBFirst;

    ximg->bitmap_pad = 8;

    ximg->depth = 24;
    ximg->bytes_per_line = w * (24/8);
    ximg->bits_per_pixel = 24;

    //ximg->red_mask   = 0xFF << 16; magic markers
    //ximg->green_mask = 0xFF << 8;
    //ximg->blue_mask  = 0xFF << 0;  

    rtn = XInitImage(ximg);
    if (rtn) {
        //printTextWindow("Xwindow Error XInitImage");
    }

    return ximg;
}


void Xwindow::show()
{
    XMapWindow(display,win);
}


void Xwindow::create(char* name, int xp, int yp, int w, int h)
{
    x = xp;
    y = yp;
    width = w;
    height = h;

    // make our window
    win = XCreateSimpleWindow(display, 
                              RootWindow(display,screen_num), 
                              x, y, width, height, 0, 
                              BlackPixel(display, screen_num), 
                              BlackPixel(display, screen_num));

    size_hints = XAllocSizeHints();
    size_hints->min_width = 300;
    size_hints->min_height = 200;

    XmbSetWMProperties(display, win,name, name, NULL, 0, size_hints, NULL, NULL);
    XSelectInput(display, win, ExposureMask | KeyPressMask | 
                 ButtonPressMask | StructureNotifyMask);

    // make the pesky, GC
    XGCValues values;
    unsigned long valuemask = 0; // ignore XGCvalues and use defaults 
    gc = XCreateGC(display, win, valuemask, &values);

    // get the screen depth
    XWindowAttributes wa;
    XGetWindowAttributes(display, win, &wa);
    x_depth = wa.depth;
}  

int Xwindow::handleKeyboard(XKeyEvent* report)
{
    char buf[100];
    KeySym keysym;
    XComposeStatus compose;
    int count;

    count = XLookupString(report,buf,100,&keysym,&compose);
    switch(keysym){
    }
    return(0);
}

static void start_thread(void *xwindow_instance)
{
    ((Xwindow*)xwindow_instance)->handler_thread();
}

void Xwindow::handler_thread(void)
{
    XEvent report;
    char buf[100];
    static int count = 0;

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        if (report_level >= HS_REP_LEV_ERR){
            printTextWindow("Xwindow error XOpenDisplay");
        }
        return;
    }
    screen_num = DefaultScreen(display);
    create("Hserver",0,0,640,480);
    printTextWindow("Xwindow created");
    show();

    if (report_level>=HS_REP_LEV_DEBUG)
    {
        sprintf(buf,"  screen_num %d  width %d  height %d  x %d  y %d  depth %d",
                screen_num,width,height,x,y,x_depth);
        printTextWindow(buf);
    }

    while(true)
    {
        pthread_testcancel();

        if (XPending(display))
        {
            XNextEvent(display, &report);
            // let the window(s) deal w/ events first
            switch(report.type){ // then, we'll deal with it
            case KeyPress:
                handleKeyboard((XKeyEvent*)&report);
                break;
            default:
                if (report_level >= HS_REP_LEV_DEBUG) printfTextWindow("event = %d", report.type);
                break;
            }
        }

        if (count > SKIP_STATUSBAR_UPDATE_)
        {
            updateStatusBar_();
            count = 0;
        }
        count++;

        usleep(READER_THREAD_USLEEP_);
    }
}

void Xwindow::drawRect(int x,int y,int w,int h,int c)
{
    XSetForeground(display,gc,colors[c]);
    XDrawRectangle(display, win, gc, x, y, w, h);  
}

void Xwindow::drawPoint(int x,int y,int c)
{
    XSetForeground(display,gc,colors[c]);
    XDrawPoint(display, win, gc, x, y);
}

void Xwindow::putImage(XImage *ximg,int x,int y,int w,int h){
    XPutImage(display, win, gc, ximg, 0,0,x,y,w,h);
}

void Xwindow::putRGB(int x,int y,int w,int h, unsigned char* data)
{
    XImage *ximg;
  
    ximg = RGBToXImage(w,h,data);
    putImage(ximg,x,y,w,h);
    delete ximg;
}


void Xwindow::control()
{
    bool bDone = 0;
    
    messageDrawWindow( EnMessageType_X_CONTROL, EnMessageErrType_NONE );
    do
    {
        int c = getch();
        switch(c){
        case 'd':
            messageHide();
            delete this;
            return;
            break;
            break;
        case 'x':
        case 'Q':
        case KEY_ESC:
            bDone = true;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            messageMovePanelKey(c);
            break;
        }
    } while ( !bDone );
    messageHide();
}

Xwindow::~Xwindow(){
    pthread_cancel(handler);
    pthread_join(handler,NULL);
    XCloseDisplay(display);    
    XFreeGC(display,gc);
    printTextWindow("Xwindow disconnected");
}

Xwindow::Xwindow(Xwindow **a) : Module((Module**)a, HS_MODULE_NAME_XWINDOW)
{
    if ( loadColors() )
    {
        pthread_create(&handler,NULL,(void*(*)(void*))&start_thread,(void*)this);
    }
    else
    {
        printTextWindow("Xwindow not created");
        a = NULL;
    }
}

void Xwindow::updateStatusBar_(void)
{
    char buf[100];

    statusStr[0] = '\0';

    strcat(statusStr, "Xwindow: ");

    sprintf(buf, "%c", statusbarSpinner_->getStatus());
    strcat(statusStr, buf);

    statusbar->update(statusLine);
}


/**********************************************************************
# $Log: xwindow.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/19 18:51:19  endo
# color_file no longer necessary.
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:41:08  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.4  2000/12/12 23:23:05  blee
# Changed #defines for the user interface to enumerated types.
#
# Revision 1.3  2000/09/19 03:51:29  endo
# RCS log added.
#
#
#**********************************************************************/
