/**********************************************************************
 **                                                                  **
 **                         sgig_management.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: sgig_management.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#ifndef SGIG_POPUP_H
#define SGIG_POPUP_H

#include <string>

using std::string;

void sgig_run(void);
void sgig_send_message( string strXMLMessage );
bool sgig_connected(void);
string sgig_get_environment_message(void);

#endif

/**********************************************************************
 * $Log: sgig_management.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1  2007/05/14 20:40:49  pulam
 * Added interface for SGIG
 *
 * Revision 1.4  2006/10/23 22:14:53  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.3  2006/09/21 14:47:54  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.2  2006/09/18 18:35:11  alanwags
 * cim_send_message() implemented.
 *
 * Revision 1.1  2006/08/19 16:50:45  alanwags
 * This is the MPPC management dialog for cfgedit
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * 
 *
 **********************************************************************/
