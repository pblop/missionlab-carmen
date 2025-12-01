/**********************************************************************
 **                                                                  **
 **                          jbox_display.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2004 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: jbox_display.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef JBOX_DISPLAY_H
#define JBOX_DISPLAY_H

#include <Xm/Xm.h>

#include <vector>
#include <string>

using std::vector;
using std::string;

class JBoxDisplay {

    typedef struct {
        double posX;
        double posY;
        double heading;
        int robotID;
        int jboxID;
    } RobotData_t;

protected:
    XtAppContext app_;
    vector<int> allowedRobotJBoxIDs_;
    vector<RobotData_t> robotDataList_;
    vector<int> quickRobotIDList_;
    vector<int> quickJBoxIDList_;

    static const int UPDATE_TIMEOUT_MSEC_;
    static const double MIN_ROBOT_LENGTH_;
    static const double DEFAULT_ROBOT_LENGTH_;

    void loadAllowedRobotJBoxList_(void);
    void startJBoxRobot_(
        int jboxID,
        gt_Point pos,
        double heading,
        char *color);
    void saveRobotData_(RobotData_t data);
    void displayJBoxRobot_(void);


public:
    JBoxDisplay(XtAppContext app, bool overlayLoaded);
    ~JBoxDisplay(void);

    static void displayLoop(void);
};

extern JBoxDisplay *gJBoxDisplay;

#endif
/**********************************************************************
 * $Log: jbox_display.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/07/11 17:15:31  endo
 * JBoxMlab merged from MARS 2020.
 *
 * Revision 1.1  2004/04/21 04:59:27  endo
 * JBoxMlab improved.
 *
 * Revision 1.1.1.1  2004/02/27 22:30:57  endo
 * Initial MARS 2020 revision
 *
 * Revision 1.1  2003/06/19 20:35:46  endo
 * Initial revision
 *
 **********************************************************************/
