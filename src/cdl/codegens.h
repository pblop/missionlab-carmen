/**********************************************************************
 **                                                                  **
 **                       codegens.h                                 **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: codegens.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include "load_rc.h"

int   mrpl_codegen(Symbol * top, char *filename);
int   cnl_codegen(Symbol * top, char *filename);
int   SAUSAGES_codegen(Symbol * top, char *filename);

extern char *command_line;
extern bool debug_save;
extern bool cdl_had_error;
extern symbol_table<rc_chain> rc_table;
extern int generated_cnl_files;

#define fsa_start_state_name "Start"


extern SymbolList AgentStack;

Symbol *find_pu_data(char *parm_name, int loops);
Symbol *real_source(const Symbol *src);
void    error(const char *s);
void    error(const char *s, const char *s2);
void    error(const char *s, const int i);
void    indent(ofstream & s, const int level);




/**********************************************************************
 * $Log: codegens.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:28  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.15  2000/03/20 00:26:02  endo
 * The bug of CfgEdit failing to compile multiple robots
 * was fixed.
 *
 * Revision 1.14  1996/05/14 23:16:56  doug
 * *** empty log message ***
 *
 * Revision 1.13  1996/03/13  01:52:36  doug
 * fixed error reporting
 *
 * Revision 1.12  1996/03/08  20:43:36  doug
 * *** empty log message ***
 *
 * Revision 1.11  1996/03/08  00:46:41  doug
 * *** empty log message ***
 *
 * Revision 1.10  1996/02/18  22:44:13  doug
 * handle pushup parms in cnl_codegen
 *
 * Revision 1.9  1996/02/18  21:01:25  doug
 * *** empty log message ***
 *
 * Revision 1.8  1996/02/01  02:29:28  doug
 * *** empty log message ***
 *
 * Revision 1.7  1996/01/17  18:46:26  doug
 * *** empty log message ***
 *
 * Revision 1.6  1995/10/09  21:57:34  doug
 * *** empty log message ***
 *
 * Revision 1.5  1995/06/29  15:04:53  jmc
 * Added RCS log string.
 **********************************************************************/
