/**********************************************************************
 **                                                                  **
 **  env_object.cc                                                   **
 **                                                                  **
 **  Base class for objects in the simulated environment.            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: env_object.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: env_object.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:03  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.17  2004/08/18 22:52:04  doug
* got ugv demo filmed
*
* Revision 1.16  2004/07/30 13:47:57  doug
* back from USC
*
* Revision 1.15  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.14  2004/03/01 00:44:03  doug
* correctly set displaylabel so don't display ones ending in '*'
*
* Revision 1.13  2003/11/03 16:41:56  doug
* working on libcmdl
*
* Revision 1.12  2003/03/16 03:24:04  doug
* moved libccl_code to the sara namespace
*
* Revision 1.11  2002/11/07 23:31:43  doug
* 2d circles are working
*
* Revision 1.10  2002/11/06 22:14:45  doug
* runs a bit
*
* Revision 1.9  2002/11/05 21:57:47  doug
* gaining
*
* Revision 1.8  2002/11/04 22:34:34  doug
* snapshot
*
* Revision 1.7  2002/11/01 22:44:33  doug
* snapshot
*
* Revision 1.6  2002/11/01 21:49:41  doug
* working on it
*
* Revision 1.5  2002/09/20 13:05:19  doug
* *** empty log message ***
*
* Revision 1.4  1999/03/10 00:22:28  doug
* moved extra to here from env_object
*
* Revision 1.3  1998/06/16 18:09:43  doug
* *** empty log message ***
*
* Revision 1.2  1998/06/15 21:38:09  doug
* *** empty log message ***
*
* Revision 1.1  1998/06/15 18:29:39  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "environment.h"
#include "env_object.h"

namespace sara
{
// *********************************************************************
/// Static data
env_object::colorChangedCallback_T env_object::colorChangedCallback = NULL;
env_object::labelChangedCallback_T env_object::labelChangedCallback = NULL;

/**********************************************************************/
// constructor
env_object::env_object(shape *itsShape) :
        theShape(itsShape),
	box(NULL),
	holding_us(NULL),
	color("black"),
        displayLabel(false),
        fontSize(NORMAL_SIZE)
{
   // Give the shape a pointer to us.
   itsShape->setParent(this);
}

/**********************************************************************/
// constructor
env_object::env_object(shape *itsShape, const strings *ilabel) :
        theShape(itsShape),
	box(NULL),
	holding_us(NULL),
	color("black"),
	label(*ilabel),
        displayLabel(true),
        fontSize(NORMAL_SIZE)
{
   // if the label ends in a '*', then don't display it.
   if( label.size() > 0 && label[0].at(label[0].length()-1) == '*' )
   {
      displayLabel = false;
   }

   // If null shape, error out.
   if( !itsShape )
   {
      FATAL_ERROR("NULL Shape passed to env_object constructor!");
   }

   // Give the shape a pointer to us.
   itsShape->setParent(this);
}

/**********************************************************************/
// constructor
env_object::env_object(shape *itsShape, const strings *ilabel, const string &new_color) :
        theShape(itsShape),
	box(NULL),
	holding_us(NULL),
	color(new_color),
	label(*ilabel),
        displayLabel(true),
        fontSize(NORMAL_SIZE)
{
   // if the label ends in a '*', then don't display it.
   if( label.size() > 0 && label[0].at(label[0].length()-1) == '*' )
   {
      displayLabel = false;
   }

   // Give the shape a pointer to us.
   itsShape->setParent(this);
}

/**********************************************************************/
// constructor
env_object::env_object(shape *itsShape, const string &new_color) :
        theShape(itsShape),
	box(NULL),
	holding_us(NULL),
	color(new_color),
        displayLabel(false),
        fontSize(NORMAL_SIZE)
{
   // Give the shape a pointer to us.
   itsShape->setParent(this);
}

/**********************************************************************/
// destructor
env_object::~env_object()
{
   // dump the shape
   delete theShape;
}

/**********************************************************************/
// Can the object contain other objects?
bool 
env_object::is_container()
{
   return box != NULL;
}

/**********************************************************************/
// hide this object inside the container
// returns:  0 = Success
//          -1 = bad parameter
//          -2 = object is already in a container
//          -3 = object is not containable
//          -4 = object is not a container
int 
env_object::put_us_in(env_object *container)
{
   // Check for garbage in.
   if( this == NULL || container == NULL )
      return -1;

   // Check that object is not already in a container
   if( holding_us != NULL )
      return -2;

   // Check that object can be put in containers.
   if( !is_movable() )
      return -3;

   // Check that the dest is really a container.
   if( !container->is_container() )
      return -4;

   container->box->push_back(this);
   holding_us = container;

   // Success.
   return 0;
}

/**********************************************************************/
// take this object back out of the container and make it visible.
// returns:  0 = Success
//          -1 = bad parameter
//          -2 = object is not in a container
//          -3 = Internal error: object is not in the container
int 
env_object::take_us_out()
{
   // Check for garbage in.
   if( this == NULL )
      return -1;

   // Check that object is in a container
   if( holding_us == NULL )
      return -2;

   // Find us.
   OBJECT_CONTAINER::iterator it = find(holding_us->box->begin(), holding_us->box->end(), this);

   // Shouldn't happen.
   if( it == holding_us->box->end() )
      return -3;

   // Take us out.
   holding_us->box->erase(it);
   holding_us = NULL;

   // Success.
   return 0;
}

// **********************************************************************
/// Change the color of an object
void
env_object::set_color(const string &new_color)
{
   // Change the color 
   color = new_color;

   // run the callback, if there is one.
   if( colorChangedCallback != NULL )
   {
      (*colorChangedCallback)(this);
   }
};

// **********************************************************************
/// get the color of an object
string 
env_object::get_color() const
{
   return color;
}

// **********************************************************************
/// Change a label on the object
void 
env_object::set_Label(const strings &newLabel)
{
   // Change the label
   label = newLabel;
	
   // run the callback, if there is one.
   if( labelChangedCallback != NULL )
   {
      (*labelChangedCallback)(this);
   }
}

/**********************************************************************/
}
