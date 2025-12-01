/**********************************************************************
 **                                                                  **
 **                          keyboard_reader.h                       **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef KEYBOARD_READER_H
#define KEYBOARD_READER_H

/* $Id: keyboard_reader.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <pthread.h>
#include <string>

using std::string;

class KeyboardReader {

protected:
    KeyboardReader **self_;
    pthread_t daemonThread_;
    bool daemonThreadIsUp_;

    static pthread_mutex_t readerMutex_;

    static const struct timespec DAEMON_SLEEPTIME_NSEC_;
    static const int DAEMON_USLEEPTIME_;
    static const string KEYBIND_TITLE_;

    void daemonMainLoop_(void);
    void catchAndProcessInput_(void);

    static void *startDaemonThread_(void *keyboardReaderInstance);

public:
    KeyboardReader(void);
    KeyboardReader(KeyboardReader **self);
    ~KeyboardReader(void);
    void startDaemon(void);
    void displayDefaultKeybind(void);
};

#endif
/**********************************************************************
 * $Log: keyboard_reader.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2005/02/07 20:09:51  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2003/04/06 15:30:45  endo
 * gcc 3.1.1
 *
 * Revision 1.1  2003/04/06 08:43:44  endo
 * Initial revision
 *
 *
 **********************************************************************/
