/**********************************************************************
 **                                                                  **
 **                       LabelToIdConversions.c                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: LabelToIdConversions.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include "LabelToIdConversions.h"

// ENDO - gcc 3.4
const SuLabelIdPair* aasuVALUE_TYPE_CONVERSIONS[NUM_LABEL_ID_PAIRS] = 
{
    asuCAMERA_LABEL_CONVERSIONS,
    asuLASER_LABEL_CONVERSIONS,
    asuCOGNACHROME_LABEL_CONVERSIONS,
    asuNOMAD_LABEL_CONVERSIONS,
    asuPIONEER_LABEL_CONVERSIONS,
    asuPIONEER2_LABEL_CONVERSIONS,
    asuAMIGOBOT_LABEL_CONVERSIONS,
    asuROBOTLASER_LABEL_CONVERSIONS,
	asuROOMBA_LABEL_CONVERSIONS,
	asuROOMBA560_LABEL_CONVERSIONS,
    asuFRAMEGRABBER_LABEL_CONVERSIONS,
    asuGPS_LABEL_CONVERSIONS,
    asuJBOX_LABEL_CONVERSIONS,
    asuCOMPASS_LABEL_CONVERSIONS,
    asuGYRO_LABEL_CONVERSIONS,
    asuPOSECALC_LABEL_CONVERSIONS,
    asuWEBCAM_LABEL_CONVERSIONS,
	asuCARMEN_LABEL_CONVERSIONS
};

/**********************************************************************
# $Log: LabelToIdConversions.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2006/05/15 06:01:03  endo
# gcc-3.4 upgrade
#
#**********************************************************************/
