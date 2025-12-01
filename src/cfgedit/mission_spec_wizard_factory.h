/**********************************************************************
 **                                                                  **
 **                 mission_spec_wizard_factory.h                    **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This file contains a factory class for MissionSpecWizard.       **
 **                                                                  **
 **  Copyright 2007 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef MISSION_SPEC_WIZARD_FACTORY_H
#define MISSION_SPEC_WIZARD_FACTORY_H

/* $Id: mission_spec_wizard_factory.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <X11/Xlib.h>
#include <Xm/Xm.h>

#include "mission_spec_wizard.h"
#include "icarus_wizard.h"
#include "bams_wizard.h"
#include "symbol.hpp"
#include "load_rc.h"
#include "mission_expert.h"

//----------------------------------------------------------------
// Sole purpose of this class to instantiate the implementation
// of MissionSpecWizard.
//----------------------------------------------------------------
class MissionSpecWizardFactory {

public:
    static MissionSpecWizard *createMissionSpecWizard(
        int missionSpecWizardType,
        Display *display,
        Widget parent,
        XtAppContext app,
        MissionExpert *mexp,
        const symbol_table<rc_chain> &rcTable);
};

inline MissionSpecWizard *MissionSpecWizardFactory::createMissionSpecWizard(
    int missionSpecWizardType,
    Display *display,
    Widget parent,
    XtAppContext app,
    MissionExpert *mexp,
    const symbol_table<rc_chain> &rcTable)
{
    MissionSpecWizard *missionSpecWizard = NULL;

    switch (missionSpecWizardType) {

    case MISSION_SPEC_WIZARD_BAMS:
        missionSpecWizard = new BamsWizard(
            display,
            parent,
            app,
            mexp,
            rcTable);
        break;

    case MISSION_SPEC_WIZARD_ICARUS:
        missionSpecWizard = new IcarusWizard(
            display,
            parent,
            app,
            mexp,
            rcTable);
        break;
    }

    if (missionSpecWizard == NULL)
    {
        fprintf(
            stderr,
            "Error: MissionSpecWizardFactory::createMissionSpecWizard(). Unknown missionSpecWizardType: %d\n",
            missionSpecWizardType);
    }

    return missionSpecWizard;
}

#endif
/**********************************************************************
 * $Log: mission_spec_wizard_factory.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1  2007/05/15 18:50:01  endo
 * BAMS Wizard implemented.
 *
 **********************************************************************/
