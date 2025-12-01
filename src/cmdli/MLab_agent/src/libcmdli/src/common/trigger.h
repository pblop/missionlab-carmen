#ifndef trigger_H
#define trigger_H
/**********************************************************************
 **                                                                  **
 **  trigger.h                                                       **
 **                                                                  **
 **  interface which users implement to create a trigger             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2004.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: trigger.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: trigger.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:14:58  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.2  2004/05/11 19:34:50  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.1  2004/01/15 00:25:05  doug
// working on cmdl
//
///////////////////////////////////////////////////////////////////////

// *********************************************************************
namespace sara
{
// Interface definition for triggers
class trigger 
{
public:
   /// Called once when an instance of the trigger is created.
   virtual void init() 		{};

   /// Called when the trigger looses scope after having been in scope
   /// (i.e., it ran last cycle, but didn't run this cycle.)
   virtual void lostScope()	{};

   /// Called when the trigger gains scope after having been out of scope
   /// (i.e., it did not run last cycle, but is going to run this cycle)
   virtual void gainingScope()	{};

   /// Called each cycle when the trigger's output is required.
   virtual bool execute(const ParmList &parms) = 0;

   /// Called once when the instance is going to be destroyed.
   virtual void cleanup() 	{};
};

/*********************************************************************/
}
#endif
