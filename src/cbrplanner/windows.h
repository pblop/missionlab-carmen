/**********************************************************************
 **                                                                  **
 **                              windows.h                           **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef WINDOWS_H
#define WINDOWS_H

/* $Id: windows.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <curses.h>
#include <panel.h>
#include <string>

using std::string;

//-----------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------
typedef struct WindowSize_t {
    int x;
    int y;
};

typedef enum CursorVisibility_t {
    CURSOR_HIDE,
    CURSOR_NORMAL,
    CURSOR_VERYVISIBLE
};

//-----------------------------------------------------------------------
// Subwindow class
//-----------------------------------------------------------------------
class Subwindow {

protected:
    Subwindow **self_;
    WINDOW *titleWindow_;
    WINDOW *window_;
    PANEL *titlePanel_;
    PANEL *panel_;
    string title_;
    WindowSize_t mainWindowSize_;
    int startPosY_;
    int height_;

    static const int LINE_OFFSET_;
    static const int MOVE_OFFSET_;
    static const int TITLE_HEIGHT_;
    static const int MAX_BUFSIZE_;
    static const int SCAN_USLEEPTIME_;
    static const int KEYNUM_BACKSPACE_;
    static const int KEYNUM_ENTER_;
    static const string EMPTY_STRING_;

    void printWindow_(char *str);
    void redrawWindow_(void);

public:
    Subwindow(void);
    Subwindow(Subwindow **self);
    ~Subwindow(void);
    void createWindow(
	WindowSize_t mainWindowSize,
	int startPosY,
	int height,
	string title);
    void printfWindow(const char *format, ...);
    void printWindow(char *str);
    void clearWindow(void);
    void redrawWindow(void);
    void setTitle(string title);
    string scanfWindow(void);
};

inline void Subwindow::printWindow(char *str)
{
    printWindow_(str);
}

inline void Subwindow::redrawWindow(void)
{
    redrawWindow_();
}

//-----------------------------------------------------------------------
// Windows class
//-----------------------------------------------------------------------
class Windows {

protected:
    Windows **self_;
    WINDOW *mainWindow_;
    WINDOW *backWindow_;
    PANEL *backPanel_;
    WindowSize_t mainWindowSize_;
    Subwindow *plannerWindow_;
    Subwindow *commWindow_;
    Subwindow *keyboardWindow_;
    bool disableDisplay_;

    static const int COMM_WINDOW_START_POS_Y_;
    static const int PLANNER_WINDOW_START_POS_Y_;
    static const int KEYBOARD_WINDOW_HEIGHT_;
    static const int LINE_OFFSET_;
    static const int MOVE_OFFSET_;
    static const int MAX_BUFSIZE_;
    static const string EMPTY_STRING_;
    static const string MAIN_TITLE_;
    static const string COMM_WINDOW_TITLE_;
    static const string PLANNER_WINDOW_TITLE_;

    void initWindows_(void);
    void createMainWindow_(void);
    void createBackWindow_(void);
    void redrawWindows_(void);

public:
    Windows(void);
    Windows(Windows **self);
    ~Windows(void);
    string scanfKeyboardWindow(void);
    void startWindows(void);
    void printfPlannerWindow(const char *format, ...);
    void printPlannerWindow(char *str);
    void clearPlannerWindow(void);
    void printfCommWindow(const char *format, ...);
    void printCommWindow(char *str);
    void clearCommWindow(void);
    void printfKeyboardWindow(const char *format, ...);
    void printKeyboardWindow(char *str);
    void setKeyboardWindowTitle(string title);
    void clearKeyboardWindow(void);
    bool displayDisabled(void);
};

inline void Windows::printPlannerWindow(char *str)
{
    if (!disableDisplay_)
    {
        plannerWindow_->printWindow(str);
    }
}

inline void Windows::clearPlannerWindow(void)
{
    if (!disableDisplay_)
    {
        plannerWindow_->clearWindow();
    }
}

inline void Windows::printCommWindow(char *str)
{
    if (!disableDisplay_)
    {
        commWindow_->printWindow(str);
    }
}

inline void Windows::clearCommWindow(void)
{
    if (!disableDisplay_)
    {
        commWindow_->clearWindow();
    }
}

inline void Windows::printKeyboardWindow(char *str)
{
    if (!disableDisplay_)
    {
        keyboardWindow_->printWindow(str);
    }
}

inline void Windows::setKeyboardWindowTitle(string title)
{
    if (!disableDisplay_)
    {
        keyboardWindow_->setTitle(title);
    }
}

inline void Windows::clearKeyboardWindow(void)
{
    if (!disableDisplay_)
    {
        keyboardWindow_->clearWindow();
    }
}

inline string Windows::scanfKeyboardWindow(void)
{
    if (disableDisplay_)
    {
        return EMPTY_STRING_;
    }

    return (keyboardWindow_->scanfWindow());
}

inline bool Windows::displayDisabled(void)
{
    return disableDisplay_;
}

extern Windows *gWindows;

#endif
/**********************************************************************
 * $Log: windows.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.2  2006/01/30 02:50:35  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2003/04/06 15:29:47  endo
 * gcc 3.1.1
 *
 * Revision 1.1  2003/04/06 08:43:44  endo
 * Initial revision
 *
 *
 **********************************************************************/
