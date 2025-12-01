#ifndef ENV_OBJECT_H
#define ENV_OBJECT_H
/**********************************************************************
 **                                                                  **
 **  env_object.h                                                    **
 **                                                                  **
 **  Base class for objects in the simulated environment.            **
 **  Used to manage the physical characteristics of the objects      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: env_object.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: env_object.h,v $
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
#include "Vector.h"
#include "shape.h"
#include "env_object_extra.h"

namespace sara
{
/**********************************************************************/
/**@name typedefs */
//@{
/// A container that can hold environment objects
typedef std::vector<class env_object *> OBJECT_CONTAINER;
//@}

/**********************************************************************/
/**@name env_object */
//@{
class env_object
{
public:
   /// Are we visible to sensors?
   bool is_visible() {return holding_us == NULL;}

   /// Can the object contain other objects?
   bool is_container();

   /// Can the object be moved?  Movable also implies can be contained.
   virtual bool is_movable() = 0;

   /// Constructors
   env_object(class shape *itsShape);
   env_object(class shape *itsShape, const strings *label);
   env_object(class shape *itsShape, const strings *label,const std::string &color);
   env_object(class shape *itsShape, const std::string &color);

   /// destructor
   virtual ~env_object();

   /// Change the color of an object
   typedef void (*colorChangedCallback_T)(env_object *);
   static colorChangedCallback_T colorChangedCallback;
   void set_color(const std::string &color);

   /// get the color of an object
   std::string get_color() const;

   /// Change a label on the object
   typedef void (*labelChangedCallback_T)(env_object *obj);
   static labelChangedCallback_T labelChangedCallback;
   void set_Label(const strings &newLabel);
   bool is_labeled() const {return label.size() > 0;}
   bool hasDisplayableLabel() const {return is_labeled() && displayLabel;}
   void set_displayLabel(bool val) {displayLabel = val;}

   /// Set the size of font the label is drawn with
   typedef enum {NORMAL_SIZE, SMALL_SIZE, LARGE_SIZE} FontSize;
   void set_fontSize(FontSize size) {fontSize = size;}
   FontSize get_fontSize() const {return fontSize;}

   /// Get the label on the object
   const strings &get_label() const {return label;}

   /// hide this object inside the container
   /// returns:  0 = Success
   ///          -1 = bad parameter
   ///          -2 = object is already in a container
   ///          -3 = object is not containable
   ///          -4 = object is not a container
   int put_us_in(env_object *container);

   /// take this object back out of the container and make it visible.
   /// returns:  0 = Success
   ///          -1 = bad parameter
   ///          -2 = object is not in a container
   ///          -3 = Internal error: object is not in the container
   int take_us_out();

   /// The shape of this object
   shape *const theShape;

private:
   /// Container to hold other objects.  NULL if we aren't a container
   OBJECT_CONTAINER    *box;		

   /// Pointer to the object holding us.  NULL if no one is holding us.
   env_object          *holding_us;	

   /// What is our color?
   std::string color;

   /// Our label
   strings label;

   /// Should the label be displayed?
   bool displayLabel;

   /// The desired font size to use writing the label
   FontSize fontSize;
};
//@}
}

/**********************************************************************/
#endif

