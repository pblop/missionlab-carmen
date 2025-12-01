/**********************************************************************
 **                                                                  **
 **                           bams_wizard.h                          **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  This is a header file for bams_wizard.cc.                       **
 **                                                                  **
 **  Copyright 2007 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef BAMS_WIZARD_H
#define BAMS_WIZARD_H

/* $Id: bams_wizard.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <time.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <vector>
#include <string>

#include "mission_spec_wizard.h"
#include "symbol.hpp"
#include "load_rc.h"
#include "mission_expert_types.h"
#include "mission_expert.h"

using std::vector;
using std::string;

class MissionExpert;

class BamsWizard : public MissionSpecWizard {

public:
    typedef enum StepNumber_t {
        STEP_GET_GIG_SURROGATE_DATA,
        STEP_CHECK_GIG_SURROGATE_DATA,
        STEP_SPECIFY_MISSION,
        STEP_CHOOSE_MISSION,
        STEP_REHEARSE_MISSION,
        STEP_SEND_MISSION,
        STEP_COMPLETION,
        NUM_STEPS
    };

protected:
    static const StepData_t STEP_DATA_[NUM_STEPS];
    static const string MAIN_WINDOW_TITLE_;
    static const string MINI_WINDOW_TITLE_;
    static const string INSTRUCTION_STARTUP_MESSAGE_;
    static const string INSTRUCTION_GET_GIG_SURROGATE_DATA_MESSAGE_;
    static const string INSTRUCTION_CHECK_GIG_SURROGATE_DATA_MESSAGE_;
    static const string INSTRUCTION_LOADING_AREA_MAP_MESSAGE_;
    static const string INSTRUCTION_SPECIFY_MISSION_MESSAGE_;
    static const string INSTRUCTION_SPECIFY_MISSION_NO_DATA_MESSAGE_;
    static const string INSTRUCTION_CHOOSE_MISSION_MESSAGE_;
    static const string INSTRUCTION_REHEARSE_MISSION_MESSAGE_;
    static const string INSTRUCTION_SEND_MISSION_MESSAGE_;
    static const string INSTRUCTION_COMPLETION_SUCCESS_MESSAGE_;
    static const string INSTRUCTION_COMPLETION_FAILURE_MESSAGE_;
    static const string DATA_INVALID_SLECTION_MESSAGE_;
    static const string DATA_SGIG_NOT_CONNECTED_MESSAGE_;
    static const string DATA_SGIG_NO_ENV_DATA_MESSAGE_;

    string instructionStartUpMessage_(void);
    string mainWindowTitle_(void);
    string miniWindowTitle_(void);
    StepData_t *stepData_(void);
    int numSteps_(void);
    int stepChooseMission_(void);
    int stepCompletion_(void);
    int stepRehearsal_(void);
    int stepSpecifyMission_(void);
    void askUserSpecifyMission_(void);
    void processBackBtn_(void);
    void processDetailBtn_(void);
    void processFirstStep_(void);
    void processNextBtn_(void);
    void processStep_(int stepNum);

public:
    BamsWizard(void);
    BamsWizard(
        Display *display,
        Widget parent,
        XtAppContext app,
        MissionExpert *mexp,
        const symbol_table<rc_chain> &rcTable);
    ~BamsWizard(void);
};

inline string BamsWizard::instructionStartUpMessage_(void)
{
    return INSTRUCTION_STARTUP_MESSAGE_;
}

inline string BamsWizard::mainWindowTitle_(void)
{
    return MAIN_WINDOW_TITLE_;
}

inline string BamsWizard::miniWindowTitle_(void)
{
    return MINI_WINDOW_TITLE_;
}

inline MissionSpecWizard::StepData_t *BamsWizard::stepData_(void)
{
    return (StepData_t *)STEP_DATA_;
}

inline int BamsWizard::numSteps_(void)
{
    return NUM_STEPS;
}

inline int BamsWizard::stepChooseMission_(void)
{
    return STEP_CHOOSE_MISSION;
}

inline int BamsWizard::stepCompletion_(void)
{
    return STEP_COMPLETION;
}

inline int BamsWizard::stepRehearsal_(void)
{
    return STEP_REHEARSE_MISSION;
}

inline int BamsWizard::stepSpecifyMission_(void)
{
    return STEP_SPECIFY_MISSION;
}

#endif
/**********************************************************************
 * $Log: bams_wizard.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/09/18 22:36:10  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.1  2007/05/15 18:50:00  endo
 * BAMS Wizard implemented.
 *
 * Revision 1.8  2007/03/05 19:35:37  endo
 * The bug on Back button fixed.
 *
 * Revision 1.7  2007/02/13 11:01:12  endo
 * IcarusWizard can now display meta data.
 *
 * Revision 1.6  2006/10/23 22:14:53  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.5  2006/09/22 18:43:04  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.4  2006/09/22 17:57:17  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.3  2006/09/21 14:47:54  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.2  2006/09/15 22:37:40  endo
 * ICARUS Wizard compilation meter bars added.
 *
 * Revision 1.1  2006/09/13 19:03:48  endo
 * ICARUS Wizard implemented.
 *
 **********************************************************************/
