/**********************************************************************
 **                                                                  **
 **                              windows.cc                          **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This module deals with ncursor-based window functions.          **
 **                                                                  **
 **  Copyright 2003 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: windows.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <curses.h>
#include <panel.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "windows.h"
#include "version.h"

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const int Windows::COMM_WINDOW_START_POS_Y_ = 2;
const int Windows::PLANNER_WINDOW_START_POS_Y_ = 10;
const int Windows::KEYBOARD_WINDOW_HEIGHT_ = 2;
const int Windows::LINE_OFFSET_ = 2;
const int Windows::MOVE_OFFSET_ = 1;
const int Windows::MAX_BUFSIZE_ = 4096;
const string Windows::EMPTY_STRING_ = "";
const string Windows::MAIN_TITLE_ = "CBRServer";
const string Windows::COMM_WINDOW_TITLE_ = "Communication Status:";
const string Windows::PLANNER_WINDOW_TITLE_ = "Planner Status:";

const int Subwindow::LINE_OFFSET_ = 2;
const int Subwindow::MOVE_OFFSET_ = 1;
const int Subwindow::TITLE_HEIGHT_ = 1;
const int Subwindow::MAX_BUFSIZE_ = 4096;
const int Subwindow::SCAN_USLEEPTIME_ = 10000;
const int Subwindow::KEYNUM_BACKSPACE_ = 7;
const int Subwindow::KEYNUM_ENTER_ = 13;
const string Subwindow::EMPTY_STRING_ = "";
//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for Subwindow class.
//-----------------------------------------------------------------------
Subwindow::Subwindow(Subwindow **subwindowInstance) : self_(subwindowInstance)
{
    if (subwindowInstance != NULL)
    {
        *subwindowInstance = this;
    }

    window_ = NULL;
    panel_ = NULL;
    startPosY_ = 0;
    height_ = 0;
}

//-----------------------------------------------------------------------
// Distructor for Subwindow class.
//-----------------------------------------------------------------------
Subwindow::~Subwindow(void)
{
    if (self_ != NULL)
    {
        *self_ = NULL;
    }
}

//-----------------------------------------------------------------------
//  This function creates the window.
//-----------------------------------------------------------------------
void Subwindow::createWindow(
    WindowSize_t mainWindowSize,
    int startPosY,
    int height,
    string title)
{
    // Set up the variables.
    mainWindowSize_ = mainWindowSize;
    startPosY_ = startPosY + TITLE_HEIGHT_;
    height_ = height - TITLE_HEIGHT_;

    // Create the title window.
    titleWindow_ = new WINDOW;
    titleWindow_ = newwin(
        TITLE_HEIGHT_,
        mainWindowSize_.x - LINE_OFFSET_,
        startPosY + MOVE_OFFSET_,
        MOVE_OFFSET_);
    titlePanel_ = new_panel(titleWindow_);
    scrollok(titleWindow_, false);
    show_panel(titlePanel_);

    // Write a title.
    wattron(titleWindow_, A_BOLD);
    waddstr(titleWindow_, title.c_str());  
    wattroff(titleWindow_, A_BOLD);

    // Create the regular window.
    window_ = new WINDOW;
    window_ = newwin(
        height_,
        mainWindowSize_.x - LINE_OFFSET_,
        startPosY_ + MOVE_OFFSET_,
        MOVE_OFFSET_);
    panel_ = new_panel(window_);
    idlok(window_, true);
    scrollok(window_, true);
    show_panel(panel_);
}

//-----------------------------------------------------------------------
// This function will clear the window.
//-----------------------------------------------------------------------
void Subwindow::clearWindow(void)
{
    werase(window_);
    redrawWindow();
    wmove(window_, 0, 0);
}

//-----------------------------------------------------------------------
// This function prints string to window. nl will scroll window. Filter
// will remove all non-alphanumerics. clear will clear window and return.
//-----------------------------------------------------------------------
void Subwindow::printWindow_(char *str)
{
    waddstr(window_, str);  
    redrawWindow_();
}

//-----------------------------------------------------------------------
// This function prints a formatted text string to the window.
//-----------------------------------------------------------------------
void Subwindow::printfWindow(const char *format, ...)
{
    va_list args;
    char buf[MAX_BUFSIZE_];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    printWindow(buf);
}

//-----------------------------------------------------------------------
// This function redraws windows
//-----------------------------------------------------------------------
void Subwindow::redrawWindow_(void)
{
    update_panels();
    doupdate();
}

//-----------------------------------------------------------------------
// This function redraws windows
//-----------------------------------------------------------------------
void Subwindow::setTitle(string title)
{
    werase(titleWindow_);
    wattron(titleWindow_, A_BOLD);
    waddstr(titleWindow_, title.c_str());  
    wattron(titleWindow_, A_BOLD);
    redrawWindow_();
}

//-----------------------------------------------------------------------
// This function reads the user input in the windows.
//-----------------------------------------------------------------------
string Subwindow::scanfWindow(void)
{
    string buf = EMPTY_STRING_;
    string cString;
    int x, y, index, startX, startY;
    char c;

    // Make the cursor visible.
    curs_set(CURSOR_VERYVISIBLE);
    
    // Get the current cursor position.
    getyx(window_, startY, startX);

    // Read the string into the buffer until a newline is entered.
    while (true)
    {
        c = getch();

        if (c == KEYNUM_ENTER_)
        {
            // New line has entered. Quit reading.
            break;
        }
        else if (c == KEYNUM_BACKSPACE_)
        {
            // Backspace has pressed. Delete one letter.
            getyx(window_, y, x);
            x--;

            if (x >= startX)
            {
                mvwaddstr(window_, y, x, " ");
                wmove(window_, y, x);
                redrawWindow_();
                index = buf.size() - 1;
                buf.erase(index, 1);
            }
            continue;
        }

        cString = c;
        buf += cString;

        waddstr(window_, cString.c_str());
        getyx(window_, y, x);
        redrawWindow_();

        usleep(SCAN_USLEEPTIME_);
    }

    // Hide the cursor again. 
    curs_set(CURSOR_HIDE);

    return buf;
}

//-----------------------------------------------------------------------
// Constructor for Windows class.
//-----------------------------------------------------------------------
Windows::Windows(Windows **windowsInstance) : 
    self_(windowsInstance),
    disableDisplay_(false)
{
    if (windowsInstance != NULL)
    {
        *windowsInstance = this;
    }

    mainWindow_ = NULL;
    backWindow_ = NULL;
    backPanel_ = NULL;

    plannerWindow_ = NULL;
}

//-----------------------------------------------------------------------
// Distructor for Windows class.
//-----------------------------------------------------------------------
Windows::~Windows(void)
{
    if (!disableDisplay_)
    {
        // Show cursor.
        curs_set(CURSOR_NORMAL);
    }

    if (self_ != NULL)
    {
        *self_ = NULL;
    }
}

//-----------------------------------------------------------------------
//  This function starts the ncurser windows.
//-----------------------------------------------------------------------
void Windows::startWindows(void)
{
    int height, startPosY;

    if (disableDisplay_)
    {
        return;
    }

    createMainWindow_();
    createBackWindow_();

    height = PLANNER_WINDOW_START_POS_Y_ - 
        COMM_WINDOW_START_POS_Y_ - 
        MOVE_OFFSET_;

    new Subwindow(&commWindow_);
    commWindow_->createWindow(
        mainWindowSize_,
        COMM_WINDOW_START_POS_Y_,
        height,
        COMM_WINDOW_TITLE_);

    height = mainWindowSize_.y - 
        PLANNER_WINDOW_START_POS_Y_ -
        (3*MOVE_OFFSET_) -
        KEYBOARD_WINDOW_HEIGHT_;

    new Subwindow(&plannerWindow_);
    plannerWindow_->createWindow(
        mainWindowSize_,
        PLANNER_WINDOW_START_POS_Y_,
        height,
        PLANNER_WINDOW_TITLE_);

    startPosY = mainWindowSize_.y - KEYBOARD_WINDOW_HEIGHT_ - (2*MOVE_OFFSET_);
    
    new Subwindow(&keyboardWindow_);
    keyboardWindow_->createWindow(
        mainWindowSize_,
        startPosY,
        KEYBOARD_WINDOW_HEIGHT_,
        EMPTY_STRING_);

    redrawWindows_();
}

//-----------------------------------------------------------------------
//  This function initializes the ncurser window.
//-----------------------------------------------------------------------
void Windows::createMainWindow_(void)
{
    if (disableDisplay_)
    {
        return;
    }

    // Instantiate the window.
    mainWindow_ = new WINDOW;

    // initialize the curses library
    mainWindow_ = initscr();

    // tell curses not to do NL->CR/NL on output
    nonl();			        

    // take input chars one at a time, no wait for \n
    cbreak();			    

    // don't echo input
    noecho();			    
    keypad(stdscr, true);

    // Start colors.
    if (has_colors())
    {
        start_color();
        init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
        init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
        init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
        init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    }

    // Get the size of the main window.
    getmaxyx(mainWindow_, mainWindowSize_.y, mainWindowSize_.x);

    // Hide curser.
    curs_set(CURSOR_HIDE);

    // Disable line buffering.
    cbreak();
}

//-----------------------------------------------------------------------
//  This function creates back window.
//-----------------------------------------------------------------------
void Windows::createBackWindow_(void)
{
    char title[1024];

    if (disableDisplay_)
    {
        return;
    }

    // Create the window.
    backWindow_ = new WINDOW;
    backWindow_ = newwin(mainWindowSize_.y, mainWindowSize_.x, 0, 0);
    backPanel_ = new_panel(backWindow_);

    // Clear the window first.
    wclear(backWindow_);

    // Draw top horizontal line
    wmove(backWindow_, 0, 0);
    waddch(backWindow_, ACS_ULCORNER);
    whline(backWindow_, ACS_HLINE, mainWindowSize_.x - LINE_OFFSET_);

    // Draw left vertical line
    wmove(backWindow_, 0, mainWindowSize_.x - MOVE_OFFSET_);
    waddch(backWindow_, ACS_URCORNER);
    wmove(backWindow_, MOVE_OFFSET_, 0);
    wvline(backWindow_, ACS_VLINE, mainWindowSize_.y - LINE_OFFSET_);

    // Draw right vertical line
    wmove(backWindow_, MOVE_OFFSET_, mainWindowSize_.x - MOVE_OFFSET_);
    wvline(backWindow_, ACS_VLINE, mainWindowSize_.y - LINE_OFFSET_);

    // Draw the bottom horizontal line.
    wmove(backWindow_, mainWindowSize_.y - MOVE_OFFSET_, 0);
    waddch(backWindow_, ACS_LLCORNER);
    whline(backWindow_, ACS_HLINE, mainWindowSize_.x - LINE_OFFSET_);

    // Draw the divider between the title and communication windows.
    wmove(backWindow_, mainWindowSize_.y - MOVE_OFFSET_, mainWindowSize_.x - MOVE_OFFSET_);
    waddch(backWindow_, ACS_LRCORNER);
    wmove(backWindow_, COMM_WINDOW_START_POS_Y_, 0);
    waddch(backWindow_, ACS_LTEE);
    whline(backWindow_, ACS_HLINE, mainWindowSize_.x - LINE_OFFSET_);
    wmove(backWindow_, COMM_WINDOW_START_POS_Y_, mainWindowSize_.x - MOVE_OFFSET_);
    waddch(backWindow_, ACS_RTEE);
    wmove(backWindow_, MOVE_OFFSET_, MOVE_OFFSET_);

    // Draw the divider between the communication and planner windows.
    wmove(backWindow_, mainWindowSize_.y - MOVE_OFFSET_, mainWindowSize_.x - MOVE_OFFSET_);
    waddch(backWindow_, ACS_LRCORNER);
    wmove(backWindow_, PLANNER_WINDOW_START_POS_Y_, 0);
    waddch(backWindow_, ACS_LTEE);
    whline(backWindow_, ACS_HLINE, mainWindowSize_.x - LINE_OFFSET_);
    wmove(backWindow_, PLANNER_WINDOW_START_POS_Y_, mainWindowSize_.x - MOVE_OFFSET_);
    waddch(backWindow_, ACS_RTEE);
    wmove(backWindow_, MOVE_OFFSET_, MOVE_OFFSET_);

    // Draw the divider between the planner and keyboard windows.
    wmove(backWindow_, mainWindowSize_.y - MOVE_OFFSET_, mainWindowSize_.x - MOVE_OFFSET_);
    waddch(backWindow_, ACS_LRCORNER);
    wmove(backWindow_, mainWindowSize_.y - KEYBOARD_WINDOW_HEIGHT_ - (2*MOVE_OFFSET_), 0);
    waddch(backWindow_, ACS_LTEE);
    whline(backWindow_, ACS_HLINE, mainWindowSize_.x - LINE_OFFSET_);
    wmove(backWindow_,
	  mainWindowSize_.y - KEYBOARD_WINDOW_HEIGHT_ - (2*MOVE_OFFSET_),
	  mainWindowSize_.x - MOVE_OFFSET_);
    waddch(backWindow_, ACS_RTEE);
    wmove(backWindow_, MOVE_OFFSET_, MOVE_OFFSET_);

    // Write a title.
    wattron(backWindow_, A_BOLD);
    sprintf(title, "%s [version %s]", MAIN_TITLE_.c_str(), CBRPLANNER_VERSION.c_str());
    waddstr(backWindow_, title);  
    wattroff(backWindow_, A_BOLD);

    show_panel(backPanel_);
}

//-----------------------------------------------------------------------
// This function prints a formatted text string to the planner window.
//-----------------------------------------------------------------------
void Windows::printfPlannerWindow(const char *format, ...)
{
    va_list args;
    char buf[MAX_BUFSIZE_];

    if (disableDisplay_)
    {
        return;
    }

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    plannerWindow_->printWindow(buf);
}

//-----------------------------------------------------------------------
// This function prints a formatted text string to the comm window.
//-----------------------------------------------------------------------
void Windows::printfCommWindow(const char *format, ...)
{
    va_list args;
    char buf[MAX_BUFSIZE_];

    if (disableDisplay_)
    {
        return;
    }

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    commWindow_->printWindow(buf);
}

//-----------------------------------------------------------------------
// This function prints a formatted text string to the comm window.
//-----------------------------------------------------------------------
void Windows::printfKeyboardWindow(const char *format, ...)
{
    va_list args;
    char buf[MAX_BUFSIZE_];

    if (disableDisplay_)
    {
        return;
    }

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    keyboardWindow_->printWindow(buf);
}

//-----------------------------------------------------------------------
// This function redraws windows
//-----------------------------------------------------------------------
void Windows::redrawWindows_(void)
{
    if (disableDisplay_)
    {
        return;
    }

    update_panels();
    doupdate();
    commWindow_->redrawWindow();
    plannerWindow_->redrawWindow();
}

/**********************************************************************
 * $Log: windows.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/01/30 02:50:35  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/06 08:43:44  endo
 * Initial revision
 *
 *
 **********************************************************************/
