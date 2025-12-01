/**********************************************************************
 **                                                                  **
 **                  mission_spec_wizard_types.h                     **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2007 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef MISSION_SPEC_WIZARD_TYPES_H
#define MISSION_SPEC_WIZARD_TYPES_H

/* $Id: mission_spec_wizard_types.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <string>

using std::string;

//----------------------------------------------------------------
// If you add a new type, make sure to add the corresponding string
// for RCTABLE_DISABLE_MISSION_SPEC_WIZARD_STRINGS_ in
// mission_expert.h and MISSION_SPEC_WIZARD_NAMES below.
//----------------------------------------------------------------
typedef enum MissionSpecWizardTypes_t {
    MISSION_SPEC_WIZARD_DISABLED = -1,
    MISSION_SPEC_WIZARD_BAMS,
    MISSION_SPEC_WIZARD_ICARUS,
    NUM_MISSION_SPEC_WIZARD_TYPES
};

static const string MISSION_SPEC_WIZARD_NAMES[NUM_MISSION_SPEC_WIZARD_TYPES] =
{
    "UAV Wizard", // MISSION_SPEC_WIZARD_BAMS
    "ICARUS Wizard" // MISSION_SPEC_WIZARD_ICARUS
};

#endif
/**********************************************************************
 * $Log: mission_spec_wizard_types.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/10/17 17:10:37  pulam
 * s/BAMS Wizard/UAV Wizard/
 *
 * Revision 1.1  2007/05/15 18:50:01  endo
 * BAMS Wizard implemented.
 *
 **********************************************************************/
