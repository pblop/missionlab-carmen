/**********************************************************************
 **                                                                  **
 **                        gt_cmdli_panel.c                          **
 **                                                                  **
 **  Written by: Douglas C. MacKenzie                                **
 **                                                                  **
 **  Copyright 2004 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_cmdli_panel.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $  */

#include <stdio.h>   
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>

#include <Xm/Xm.h>
#include <Xm/ArrowBG.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/ScrolledW.h>
#include <Xm/Frame.h>

#include "gt_load_command.h"
#include "gt_command.h"
#include "gt_sim.h"
#include "mission_design.h"
#include "console.h"
#include "console_side_com.h"
#include "cfgedit_common.h"

#include "gt_cmdli_panel.h"
#include "CMDLcommand.h"
#include "CMDLcommandAck.h"
#include "MsgTypes.h"
#include "Command.h"

#define XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)

/// Define a set of strings (copied from mic.h)
typedef std::set<std::string> stringSet;

/**********************************************************************
 **                                                                  **
 **                            constants                             **
 **                                                                  **
 **********************************************************************/

#define DISPLAY_MAX_LEN 100

static uint broadcast_port = 12345;
static string broadcast_IP = "224.0.1.150:12345";
static uint comms_debug_flags = 0;
static bool useCompression = true;
static string our_name("mlab");

static const string RC_MULTICAST_ADDR("MulticastAddr");
static const string RC_UNICAST_PORT("UnicastPort");
static const string RC_UNICASTADDRS("RobotAddresses");
static const string RC_COMMSDEBUG("CommsDebug");


/**********************************************************************
 **                                                                  **
 **                              macros                              **
 **                                                                  **
 **********************************************************************/

#define	RES_CONVERT( res_name, res_value) \
      XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1


// ******************************************************************
// constructors 
gt_cmdli_panel::gt_cmdli_panel( Widget parentWidget ) :
   parent(parentWidget),
   cmdli_filename_field(NULL),
   created(false),
   isUp(false),
   nextID(0),
   thecmdl(NULL),
   sendCommandActive(false)
{
   std::string logfilename = "cmdli-" + our_name + ".log";
   sara::logging_file = fopen(logfilename.c_str(), "w");

   const string rc_filename("cmdli.rc");

   // Load the resource file, if there is one
   int rtn = cmdli_rc_table.load(rc_filename.c_str(), true, false, false);
   if( rtn == 0 )
   {
      printf("gt_cmdli_panel - Loaded resource file %s", rc_filename.c_str());
   }
   else if( rtn == 1 )
   {
      printf("gt_cmdli_panel - Syntax error in resource file %s", rc_filename.c_str());
   }
   else
   {
      char *dirname = getcwd(NULL, 0);
      printf("gt_cmdli_panel - Did not find a response file.  Looked for %s in %s", rc_filename.c_str(), dirname);
      free(dirname);
   }

   // Overwrite our defaults, if they specified something
   if( cmdli_rc_table.get_string(RC_MULTICAST_ADDR, broadcast_IP) )
   {
      printf("gt_cmdli_panel - Using %s as multicast address", broadcast_IP.c_str());
   }

   int tmp;
   if( cmdli_rc_table.get_int(RC_UNICAST_PORT, tmp) )
   {
      broadcast_port = tmp;
      printf("gt_cmdli_panel - Using %d as unicast port", broadcast_port);
   }

   if( cmdli_rc_table.get_int(RC_COMMSDEBUG, tmp) )
   {
      comms_debug_flags = tmp;
      cerr << "gt_cmdli_panel - Setting CommsDebug to " << comms_debug_flags << endl;
   }

}

// ******************************************************************
// destructors
gt_cmdli_panel::~gt_cmdli_panel()
{
}

/*********************************************************************/
/// Load the cmdl from the specified file.
/// No changes are made unless it returns true.
bool 
gt_cmdli_panel::load_from_file(const string &filename)
{
   // Try to open the file.
   ifstream in;
   in.open( filename.c_str() );
   stringstream buf;

   char c;
   while( in.get(c) )
   {
      buf.put(c);
   }

   // Create a temp object, in case the load fails.
   stringSet empty;
   sara::cmdl *p = new sara::cmdl(empty, empty, NULL);

   bool ok = p->load("", buf.str() );
   if( !ok )
   {
cerr << "gt_cmdli_panel::load_from_file - Error loading file" << endl;
      delete p;
      return false;
   }

   // Switch to the new cmdl
   if( thecmdl != NULL )
      delete thecmdl;
   thecmdl = p; 
   theCmdlFileData = buf.str();

   // update the display to show the current filename
   if( cmdli_filename_field )
   {
      XmTextFieldSetString( cmdli_filename_field, (char *)filename.c_str() );
      XmTextFieldSetInsertionPosition( cmdli_filename_field, filename.size() );
      cmdli_filename_field_touched = true;
   }
   cmdli_filename = filename;

   return true;
}

// ******************************************************************
// called on a periodic basis to update the status display
// returns true if done (panel is not being displayed)
bool 
gt_cmdli_panel::update_display()
{
   if( isUp )
   {
      // walk the records and update our display status
      for(addrList::iterator it = teamList.begin(); it != teamList.end(); ++it)
      {
	 sara::TimeOfDay timeSinceLastHeard = sara::TimeOfDay::now() - it->second.arrivalTime;
	 // we will show the time between the last two packets, unless
	 // the time from the last one until now is greater.
	 if( timeSinceLastHeard < it->second.lastPeriod )
	    timeSinceLastHeard = it->second.lastPeriod;

	 // build the string we will display.  
	 //   Seconds since last heard from them, or -- if over 99.
	 uint secs = (uint)(timeSinceLastHeard.secs() + 0.5);
	 char buf[3] = "--";
	 if( secs < 100 )
	    sprintf(buf,"%2d",secs);
	 else
	 {
	    // keep it from winding up so we don't redraw the string every time
	    secs = 100;
	 }
	 XmString str = XmStringCreateLocalized(buf);

	 // build the color
	 Pixel newcolor = gColorPixel.green;
	 if( secs > 60 )
	    newcolor = gColorPixel.red;
	 else if( secs > 30 )
	    newcolor = gColorPixel.yellow;

         // update the status color and timer
	 bool updateColor = newcolor != it->second.currentColor;
	 bool updateValue = secs != it->second.currentValue;
	       
	 // just make the minimum changes
	 if( updateColor && updateValue )
	 {
            XtVaSetValues( it->second.statusWidget, 
	          XmNbackground, newcolor, 
	          XmNlabelString, str, 
	          NULL );
	    it->second.currentColor = newcolor;
	    it->second.currentValue = secs;
	 }
	 else if( updateValue )
	 {
            XtVaSetValues( it->second.statusWidget, 
	          XmNlabelString, str, 
	          NULL );
	    it->second.currentValue = secs;
	 }
	 else if( updateColor )
	 {
            XtVaSetValues( it->second.statusWidget, 
	          XmNbackground, newcolor, 
	          NULL );
	    it->second.currentColor = newcolor;
	 }

	 // cleanup
	 XmStringFree(str);
      }
   }

   return !isUp;
}

// ******************************************************************
// create the panel and display it
bool 
gt_cmdli_panel::create_panel()
{
   cmdli_panel_shell = XtVaCreatePopupShell(
        "CMDLi Interface", xmDialogShellWidgetClass, parent,
        XmNallowShellResize,	True,
        NULL );

   // create a form to hold the major blocks
   cmdli_panel = XtVaCreateWidget(
        "cmdli_panel", xmFormWidgetClass, cmdli_panel_shell,
        XmNresizable,	       True,
        XmNwidth, 600,
        NULL );

   // Put the robot data in a form at the top
   robotrows = XtVaCreateWidget(
        "robotrows", xmFormWidgetClass, cmdli_panel,
         XmNtopAttachment,     XmATTACH_FORM,
         XmNleftAttachment,    XmATTACH_FORM,
         XmNrightAttachment,   XmATTACH_FORM,
         XmNresizable,	       True,
        NULL );

    // Put the control buttons in a frame at the bottom
    Widget headerFrame = XtVaCreateManagedWidget(
        "frameForButtons", xmFrameWidgetClass, robotrows,
         XmNshadowType,        XmSHADOW_ETCHED_OUT,
         XmNtopAttachment,     XmATTACH_WIDGET,
         XmNtopWidget,         robotrows,
         XmNleftAttachment,    XmATTACH_FORM,
         XmNrightAttachment,   XmATTACH_FORM,
        NULL );

   // put a header on the robot entires
   Widget header = XtVaCreateWidget(
      "header", xmFormWidgetClass, headerFrame,
      XmNmarginHeight,      4,
      XmNmarginWidth,       4,
      XmNhorizontalSpacing, 5, 
      XmNtopAttachment,     XmATTACH_FORM,
      XmNleftAttachment,    XmATTACH_FORM,
//      XmNrightAttachment,   XmATTACH_FORM,
      NULL );
   bottom_widget = header;
/*
   // left is kill button
   Widget kill_title = XtVaCreateManagedWidget(
           "Drop", xmLabelGadgetClass, header,
           XmNalignment,         XmALIGNMENT_BEGINNING,
           XmNtopAttachment,     XmATTACH_FORM,
           XmNtopOffset,         5,
           XmNbottomAttachment,  XmATTACH_FORM,
           XmNbottomOffset,      5,
           XmNleftAttachment,    XmATTACH_FORM,
	   XmNwidth,		 40,
	   XmNrecomputeSize,	 False,
           NULL );

   // next is status
   Widget status_title = XtVaCreateManagedWidget(
           "Last", xmLabelGadgetClass, header,
           XmNtopAttachment,     XmATTACH_FORM,
           XmNtopOffset,         5,
           XmNbottomAttachment,  XmATTACH_FORM,
           XmNbottomOffset,      5,
           XmNleftAttachment,    XmATTACH_WIDGET,
           XmNleftWidget,        kill_title,
           NULL );
   // next is name
   name_title = XtVaCreateManagedWidget(
           "Name", xmLabelGadgetClass, header,
           XmNalignment,         XmALIGNMENT_BEGINNING,
           XmNtopAttachment,     XmATTACH_FORM,
           XmNtopOffset,         5,
           XmNbottomAttachment,  XmATTACH_FORM,
           XmNbottomOffset,      5,
           XmNleftAttachment,    XmATTACH_WIDGET,
           XmNleftWidget,        status_title,
	   XmNwidth,		 40,
	   XmNrecomputeSize,	 False,
           NULL );

   // current status
   Widget text_title = XtVaCreateManagedWidget(
           "Current robot status", xmLabelGadgetClass, header,
           XmNalignment,         XmALIGNMENT_BEGINNING,
           XmNtopAttachment,     XmATTACH_FORM,
           XmNtopOffset,         5,
           XmNbottomAttachment,  XmATTACH_FORM,
           XmNbottomOffset,      5,
           XmNleftAttachment,    XmATTACH_WIDGET,
           XmNleftWidget,        name_title,
           XmNleftOffset,        50,
           NULL );
*/
   XtManageChild( header );

    // force the bottom form to attach to the robotrows
    XtVaSetValues( bottom_widget, XmNbottomAttachment, XmATTACH_FORM, NULL );

   // if we have a mission loaded, precreate a record for each robot.
   if( thecmdl )
   {
      stringSet robots = thecmdl->getRobotNames();
      for(stringSet::iterator it=robots.begin(); it != robots.end(); ++it)
      {
	 // create a new empty record
	 teamList[*it].record = NULL;

         addrList::iterator pos = teamList.find(*it);
	 if( pos == teamList.end() )
	 {
            warn_userf("cmdli_implementation::gt_cmdli_panel - Internal Error: Unable to add record just added to display!");
	    break;
	 }
	 else
	 {
            addRow( pos );
	 }
      }
   }

   XtManageChild( robotrows );

    // Put the control buttons in a frame at the bottom
    Widget frameForCtrlButtons = XtVaCreateManagedWidget(
        "frameForButtons", xmFrameWidgetClass, cmdli_panel,
         XmNshadowType,        XmSHADOW_ETCHED_OUT,
         XmNtopAttachment,     XmATTACH_WIDGET,
         XmNtopWidget,         robotrows,
         XmNleftAttachment,    XmATTACH_FORM,
         XmNrightAttachment,   XmATTACH_FORM,
        NULL );

    // Now a form inside the frame
    Widget controlbuttons = XtVaCreateWidget(
        "controlbuttons", xmFormWidgetClass, frameForCtrlButtons,
        NULL );

    Widget filelabel = XtVaCreateManagedWidget(
        "CMDL File:", xmLabelGadgetClass, controlbuttons,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         12,
        XmNleftAttachment,    XmATTACH_FORM,
        NULL );

    cmdli_filename_field = XtVaCreateManagedWidget(
        "command-filename", xmTextFieldWidgetClass, controlbuttons, 
        XmNcolumns,           63,
        XmNmaxLength,         256,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         5,
        XmNleftAttachment,    XmATTACH_WIDGET,
        XmNleftWidget,        filelabel,
        XmNwidth,             250,
        NULL );
    XtAddCallback( cmdli_filename_field, XmNactivateCallback, 
                   (XtCallbackProc) load_cmdli_file_cb, this );
    XtAddEventHandler( cmdli_filename_field, KeyPressMask | ButtonPressMask, false,
                       (XtEventHandler) check_cmdli_filename_field_cb, this );
    cmdli_filename_field_touched = false;

    if( cmdli_filename.empty() )
       XmTextFieldSetString( cmdli_filename_field, " <Type the name of a new cmdli file here>");
    else
       XmTextFieldSetString( cmdli_filename_field, (char *)cmdli_filename.c_str() );


    Widget downloadfileButton = XtVaCreateManagedWidget(
        " Download CMDL File ", xmPushButtonGadgetClass, controlbuttons,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         8,
//        XmNleftAttachment,    XmATTACH_WIDGET,
//        XmNleftWidget,        cmdli_filename_field,
        XmNrightAttachment,   XmATTACH_FORM,
        NULL );
    XtAddCallback( downloadfileButton, XmNactivateCallback, 
                   (XtCallbackProc) download_cmdli_file_cb, this );



    Widget execute_button = XtVaCreateManagedWidget(
        "execute-commands", xmPushButtonGadgetClass, controlbuttons,
        RES_CONVERT( XmNlabelString, " Start Mission " ),
        XmNbackground,        gColorPixel.green,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         cmdli_filename_field,
        XmNtopOffset,         8,
        XmNleftAttachment,    XmATTACH_FORM,
        XmNleftOffset,        15,
        NULL );
    XtAddCallback( execute_button, XmNactivateCallback,
                   (XtCallbackProc) start_mission_cb, this );

    Widget pause_button = XtVaCreateManagedWidget(
        "pause", xmPushButtonGadgetClass, controlbuttons,
        RES_CONVERT( XmNlabelString, " Pause  " ),
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         cmdli_filename_field,
        XmNtopOffset,         8,
        XmNrightAttachment,   XmATTACH_POSITION,
        XmNrightPosition,     50,
        XmNrightOffset,       8,
        NULL );
    XtAddCallback( pause_button, XmNactivateCallback,
                   (XtCallbackProc) pause_button_cb, this);


    Widget resume_button = XtVaCreateManagedWidget(
        "resume", xmPushButtonGadgetClass, controlbuttons,
        RES_CONVERT( XmNlabelString, " Resume " ),
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         cmdli_filename_field,
        XmNtopOffset,         8,
        XmNleftAttachment,    XmATTACH_WIDGET,
        XmNleftWidget,        pause_button,
        XmNleftOffset,        16,
        NULL );
    XtAddCallback( resume_button, XmNactivateCallback,
                   (XtCallbackProc) resume_button_cb, this);


    Widget abort_button = XtVaCreateManagedWidget(
        " ABORT ", xmPushButtonGadgetClass, controlbuttons,
	XmNbackground,        gColorPixel.red,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         cmdli_filename_field,
        XmNtopOffset,         8,
        XmNrightAttachment,   XmATTACH_FORM,
        XmNrightOffset,       15,
        NULL );
    XtAddCallback( abort_button, XmNactivateCallback,
                   (XtCallbackProc) abort_command_execution_cb, this );

    // manage the form
    XtManageChild( controlbuttons );




    // Put the control buttons in a frame at the bottom
    Widget frameForCloseButton = XtVaCreateManagedWidget(
        "frameForButtons", xmFrameWidgetClass, cmdli_panel,
         XmNshadowType,        XmSHADOW_ETCHED_OUT,
         XmNtopAttachment,     XmATTACH_WIDGET,
         XmNtopWidget,         frameForCtrlButtons,
         XmNleftAttachment,    XmATTACH_FORM,
         XmNrightAttachment,   XmATTACH_FORM,
//         XmNbottomAttachment,  XmATTACH_FORM,
        NULL );

    // Now a form inside the frame
    Widget closebutton = XtVaCreateWidget(
        "controlbuttons", xmFormWidgetClass, frameForCloseButton,
        NULL );


    // put in the close button
    Widget closeButton = XtVaCreateManagedWidget(
        "Close", xmPushButtonGadgetClass, closebutton,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         5,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNleftAttachment,    XmATTACH_POSITION,
        XmNrightAttachment,   XmATTACH_POSITION,
        XmNleftPosition,      40,
        XmNrightPosition,     60,
        NULL );
    XtAddCallback( closeButton, XmNactivateCallback,
                   (XtCallbackProc) popdown_panel_cb, this );

    XtManageChild( closebutton );


   // pop it up
   XtManageChild( cmdli_panel );
   isUp = true;


   // popup the window
    while ( XPending( XtDisplay( main_window ) ) > 0 )
    {
        XEvent event;
        XNextEvent( XtDisplay( main_window ), &event );
        XtDispatchEvent( &event );
    }


   // Start the communications subsystem so we can talk to the robots.
   comobj = new sara::objcomms();

   // set the debug level
   if( comms_debug_flags )
   {
      comobj->setDebug(comms_debug_flags);
   }

   // if specified, set multicast as unicast
   if( cmdli_rc_table.check_exists(RC_UNICASTADDRS) )
   {
      if( !comobj->loadMemberAddresses( cmdli_rc_table.get(RC_UNICASTADDRS) ) )
      {
         printf("gt_cmdli_panel - Unable to load the robot addresses from the rc file!");
      }
printf("gt_cmdli_panel - setting multicast as unicast team member addresses to:");
sara::strings addrs = cmdli_rc_table.get(RC_UNICASTADDRS);
for(uint i=0; i<addrs.size(); i++)
{
   stringstream str;
   str << addrs[i] << endl;
   printf( "   %s", str.str().c_str());
}
   }

   if( !comobj->open(broadcast_IP, broadcast_port, our_name) )
   {
      warn_userf("Unable to inititialize communications system");
   }

   // Set compression on or off
   if( useCompression )
   {
      comobj->enableCompression();
   }
   else
   {
      comobj->disableCompression();
   }

      // Register the packets we will use
   comobj->attachHandler(sara::MsgTypes::CMDLCOMMAND, sara::CMDLcommand::from_stream);
   comobj->attachHandler(sara::MsgTypes::CMDLCOMMANDACK, sara::CMDLcommandAck::from_stream);
   comobj->attachHandler(sara::MsgTypes::CMDLSTATUS, sara::CMDLstatus::from_stream);

   // Register our callback function to sniff status packets
   comobj->subscribeTo(sara::MsgTypes::CMDLSTATUS, updateStatus_cb, this);

   return true;
}

// ********************************************************************
/// Add a new row into the robot display
/// NOTE: The caller must hold a lock on TeamList before calling this function!
void 
gt_cmdli_panel::addRow(addrList::iterator it)
{
    // put it on the bottom.  Should insert it alphabetically.

    // Remove the old bottom widget's attachment
    XtVaSetValues( bottom_widget, XmNbottomAttachment, XmATTACH_NONE, NULL );

    // frame each robot.
    Widget frame = XtVaCreateManagedWidget(
        "", xmFrameWidgetClass, robotrows,
        //         XmNshadowType,        XmSHADOW_ETCHED_OUT,
        XmNleftAttachment,    XmATTACH_FORM,
        XmNrightAttachment,   XmATTACH_FORM,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         bottom_widget,
        XmNbottomAttachment,  XmATTACH_FORM,
        //           XmNheight,            40,
        //           XmNwidth,             100,
        NULL );
    bottom_widget = frame;


    // put each row in a form
    Widget form = XtVaCreateWidget(
        "command-form1", xmFormWidgetClass, frame,
        NULL );

    // top left is name
    /*
    Widget name = XtVaCreateManagedWidget(
        it->first.c_str(), xmLabelGadgetClass, form,
        XmNalignment,         XmALIGNMENT_BEGINNING,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         5,
        XmNleftAttachment,    XmATTACH_FORM,
        //	   XmNwidth,		 100,
        //	   XmNrecomputeSize,	 False,
        NULL );
    */
    // bottom left is kill button
    it->second.killButton = XtVaCreateManagedWidget(
        "Drop", xmPushButtonGadgetClass, form,
        XmNbackground,	 gColorPixel.yellow,
        XmNforeground,	 gColorPixel.black,
        //           XmNtopAttachment,     XmATTACH_POSITION,
        //           XmNtopPosition,       50,
        //           XmNrightOffset,       8,
        //           XmNtopAttachment,     XmATTACH_FORM,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNleftAttachment,    XmATTACH_FORM,
        NULL );
    XtAddCallback(it->second.killButton, XmNactivateCallback,
                  (XtCallbackProc)confirm_kill_robot_cb, this );

    // bottom, next is status flag.  Default to not heard from yet.
    it->second.statusWidget = XtVaCreateManagedWidget(
        "--", xmLabelGadgetClass, form,
        XmNbackground,	 gColorPixel.red,
        //           XmNtopAttachment,     XmATTACH_POSITION,
        //           XmNtopPosition,       50,
        //           XmNtopAttachment,     XmATTACH_FORM,
        //           XmNtopOffset,         8,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      8,
        XmNleftAttachment,    XmATTACH_WIDGET,
        XmNleftWidget,        it->second.killButton,
        XmNleftOffset,        5,
        NULL );
    it->second.currentColor = gColorPixel.red;
    it->second.currentValue = 100;

    // bottom next is command status flag
    it->second.cmdStatusWidget = XtVaCreateManagedWidget(
        " ", xmLabelGadgetClass, form,
        XmNbackground,	 gColorPixel.white,
        //           XmNtopAttachment,     XmATTACH_POSITION,
        //           XmNtopPosition,       50,
        //           XmNtopAttachment,     XmATTACH_FORM,
        //           XmNtopOffset,         8,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      8,
        XmNleftAttachment,    XmATTACH_WIDGET,
        XmNleftWidget,        it->second.statusWidget,
        XmNleftOffset,        5,
        NULL );

    // next is cmdli state
    it->second.cmdiStateWidget = XtVaCreateManagedWidget(
        "         ", xmLabelGadgetClass, form,
        XmNbackground,	 gColorPixel.white,
        XmNalignment,         XmALIGNMENT_BEGINNING,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         8,
        //           XmNbottomAttachment,  XmATTACH_FORM,
        //           XmNbottomOffset,      5,
        XmNleftAttachment,    XmATTACH_FORM,
        XmNleftOffset,        120,
        NULL );
    it->second.currentCmdiState = -1;
    /*
    // next on bottom line: "Wait list:"
    Widget waitlist = XtVaCreateManagedWidget(
        "Waiting For:", xmLabelGadgetClass, form,
        XmNalignment,         XmALIGNMENT_BEGINNING,
        //           XmNtopAttachment,     XmATTACH_WIDGET,
        //           XmNtopWidget,         it->second.cmdiStateWidget,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNleftAttachment,    XmATTACH_OPPOSITE_WIDGET,
        XmNleftWidget,        it->second.cmdiStateWidget,
        XmNrecomputeSize,	 False,
        NULL );
    */
    {
        Arg args[32];
        int n = 0;
        XtSetArg( args[ n ], XmNcolumns,                 100); n++;
        XtSetArg( args[ n ], XmNrows,                    1); n++;
        XtSetArg( args[ n ], XmNeditable,                False); n++;
        XtSetArg( args[ n ], XmNautoShowCursorPosition,  False); n++;
        XtSetArg( args[ n ], XmNcursorPositionVisible,   False); n++;
        XtSetArg( args[ n ], XmNtopAttachment,           XmATTACH_FORM); n++;
        XtSetArg( args[ n ], XmNrightAttachment,         XmATTACH_FORM); n++;
        XtSetArg( args[ n ], XmNleftAttachment,          XmATTACH_WIDGET); n++;
        XtSetArg( args[ n ], XmNleftWidget,              it->second.cmdiStateWidget); n++;
        XtSetArg( args[ n ], XmNleftOffset,              5); n++;
        XtSetArg( args[ n ], XmNvalue,                   "<status will be displayed as received>"); n++;
        XtSetArg( args[ n ], XmNscrollHorizontal,        False); n++;
        XtSetArg( args[ n ], XmNscrollVertical,          True); n++;
        XtSetArg( args[ n ], XmNeditMode,                XmMULTI_LINE_EDIT); n++;
        XtSetArg( args[ n ], XmNwordWrap,                True); n++;

        it->second.statusString = XmCreateScrolledText(form, "", args, n);
        XtManageChild( it->second.statusString );
    }


    {
        Arg args[32];
        int n = 0;
        XtSetArg( args[ n ], XmNcolumns,                 100); n++;
        XtSetArg( args[ n ], XmNrows,                    1); n++;
        XtSetArg( args[ n ], XmNbackground,              gColorPixel.gray80); n++;
        XtSetArg( args[ n ], XmNeditable,                False); n++;
        XtSetArg( args[ n ], XmNautoShowCursorPosition,  False); n++;
        XtSetArg( args[ n ], XmNcursorPositionVisible,   False); n++;
        XtSetArg( args[ n ], XmNtopAttachment,           XmATTACH_WIDGET); n++;
        XtSetArg( args[ n ], XmNtopWidget,               it->second.statusString); n++;
        XtSetArg( args[ n ], XmNrightAttachment,         XmATTACH_FORM); n++;
        XtSetArg( args[ n ], XmNleftAttachment,          XmATTACH_OPPOSITE_WIDGET); n++;
        XtSetArg( args[ n ], XmNleftWidget,              it->second.cmdiStateWidget); n++;
        XtSetArg( args[ n ], XmNbottomAttachment,        XmATTACH_FORM); n++;
        XtSetArg( args[ n ], XmNvalue,                   ""); n++;
        XtSetArg( args[ n ], XmNscrollHorizontal,        False); n++;
        XtSetArg( args[ n ], XmNscrollVertical,          True); n++;
        XtSetArg( args[ n ], XmNeditMode,                XmMULTI_LINE_EDIT); n++;
        XtSetArg( args[ n ], XmNwordWrap,                True); n++;

        it->second.waitListWidget = XmCreateScrolledText(form, "", args, n);
        XtManageChild( it->second.waitListWidget );
    }

    // so will overwrite the default the first time.
    it->second.currentCmdValue = "XX";
    it->second.currentWaitValue = "XX";

    // finish up
    XtManageChild( form );
}

// ********************************************************************
/// callback function to update our status cache as messages arrive
void 
gt_cmdli_panel::updateStatus_cb(sara::commMsg *msg, void *ptr)
{
   if( msg == NULL )
   {
      warn_userf("cmdli_implementation::gt_cmdli_panel - Internal Error: Null msg!");
      return;
   }

   gt_cmdli_panel *thisrec = (gt_cmdli_panel *)ptr;
   if( thisrec == NULL )
   {
      warn_userf("cmdli_implementation::gt_cmdli_panel - Internal Error: Null this!");
      delete msg;
      return;
   }

   // Check if it is a status message
   sara::CMDLstatus *rec = dynamic_cast<sara::CMDLstatus *>(msg);
   if( rec == NULL )
   {
      warn_userf("cmdli_implementation::gt_cmdli_panel - Internal Error: Unable to upcast msg to CMDLstatus!");
      delete msg;
      return;
   }

   if( rec->status != sara::CMDLstatus::MSGINVALID )
   {
       //bool needRebuild = false;

      // get access
      //      thisrec->teamListLock.p();

      // carefully free the last record, if one exists.
      addrList::iterator it = thisrec->teamList.find(rec->name);
      if( it != thisrec->teamList.end() )
      {
	 // we know about them

	 // delete any stale status
	 if( it->second.record )
	 {
	    delete it->second.record;

	    // update the arrivaltime and lastperiod
	    sara::TimeOfDay lasttime = it->second.arrivalTime;
	    it->second.arrivalTime = sara::TimeOfDay::now();
	    it->second.lastPeriod = it->second.arrivalTime - lasttime;
	 }
	 else
	 {
	    // was null, so add their address
	    it->second.addr = rec->senderAddr;
	 }

	 // add the new status record
	 it->second.record = rec;
      }
      else
      {
	 // first time we have heard from them.

	 // This creates a new record, and sets the address
	 thisrec->teamList[rec->name].addr = rec->senderAddr;

	 // add the new status
	 thisrec->teamList[rec->name].record = rec;
	 thisrec->teamList[rec->name].arrivalTime = sara::TimeOfDay::now();

         it = thisrec->teamList.find(rec->name);
	 if( it == thisrec->teamList.end() )
	 {
            warn_userf("cmdli_implementation::gt_cmdli_panel - Internal Error: Unable to add record just added to display!");
	    return;
	 }
	 else
	 {
            thisrec->addRow( it );
	 }
      }

      // update the status message
      if( rec->status != it->second.currentCmdiState )
      {
         string stat;
         switch( rec->status )
         {
	    case sara::CMDLstatus::MSGINVALID:
               stat = "         ";
               break;
      
	    case sara::CMDLstatus::CMDEXECUTING:
               stat = "Normal   ";
               break;
      
	    case sara::CMDLstatus::CMDFAILED:
               stat = "Failed   ";
               break;
      
	    case sara::CMDLstatus::CMDDONE:
               stat = "DONE     ";
               break;
   
	    case sara::CMDLstatus::ATBARRIER:
               stat = "AtBarrier";
               break;

	    case sara::CMDLstatus::PASTBARRIER:
               stat = "InBarrier";
               break;

	    case sara::CMDLstatus::MISSIONDONE:
               stat = "Done     ";
               break;

	    case sara::CMDLstatus::PAUSED:
               stat = "Paused   ";
               break;

	    case sara::CMDLstatus::STOPPED:
               stat = "Stopped  ";
               break;
         }
         XmString str = XmStringCreateLocalized((char *)stat.c_str());
         XtVaSetValues(it->second.cmdiStateWidget, 
            XmNlabelString, str, 
            NULL );
         XmStringFree(str);

	 // remember our displayed value
	 it->second.currentCmdiState = rec->status;
      }

      // get the current command as a string
      stringstream statOut;
      stringstream waitOut;
      if( rec->status != sara::CMDLstatus::STOPPED )
      {
	 // current command
         sara::Block *block = thisrec->thecmdl->lookupBlockName( rec->activeBlock );
         if( block )
         {
            sara::Command *command = block->getCommand( rec->activeCommand );
	    if( command )
	    {
               if( command->is_labeled() )
               {
                  statOut << command->get_label() << ". ";
               }
               statOut << command->print(); 
	    }
         }

         // current wait list
	 bool needcomma = false;
         if( !rec->blocklist.empty() )
         {
            waitOut << "Waiting for: ";
            for(uint i=0; i<rec->blocklist.size(); i++)
            {
	       if( needcomma )
                  waitOut << ", ";
               waitOut << rec->blocklist[i];
	       needcomma = true;
            }
         }
         if( !rec->ignorelist.empty() )
         {
	    if( needcomma )
	    {
               waitOut << "  ";
	       needcomma = false;
	    }
            waitOut << "Ignoring: ";
            for(uint i=0; i<rec->ignorelist.size(); i++)
            {
	       if( needcomma )
                  waitOut << ", ";
               waitOut << rec->ignorelist[i];
	       needcomma = true;
            }
         }
      }

      // set them, if they have changed
      if( statOut.str() != it->second.currentCmdValue )
      {
	 it->second.currentCmdValue = statOut.str();
         XmTextSetString( it->second.statusString, (char *)it->second.currentCmdValue.c_str() );
      }
      if( waitOut.str() != it->second.currentWaitValue )
      {
	 it->second.currentWaitValue = waitOut.str();
         XmTextSetString( it->second.waitListWidget, (char *)it->second.currentWaitValue.c_str() );
      }
   }
   else
   {
      warn_userf("cmdli_implementation::gt_cmdli_panel - Discarding corrupt status message");
   }
}

// ******************************************************************
// display the panel created in the open_panel command
void 
gt_cmdli_panel::popup_panel()
{
   if( !created )
   {
      created = create_panel();
   }
   else
   {
      XtManageChild( cmdli_panel );
   }
   isUp = true;
}

// ******************************************************************
// display the panel created in the open_panel command
void 
gt_cmdli_panel::popup_panel_cb(Widget w, XtPointer client_data)
{
   gt_cmdli_panel *ptr = (gt_cmdli_panel *)client_data;
   ptr->popup_panel();
}

// ******************************************************************
// hide the panel
void 
gt_cmdli_panel::popdown_panel()
{
   if( created )
   {
      XtUnmanageChild( cmdli_panel );
   }
   isUp = false;
}

// ******************************************************************
// hide the panel
void 
gt_cmdli_panel::popdown_panel_cb(Widget w, XtPointer client_data)
{
   gt_cmdli_panel *ptr = (gt_cmdli_panel *)client_data;
   ptr->popdown_panel();
}

// ******************************************************************
/// pause the robots
void 
gt_cmdli_panel::pause_button_cb(Widget w, XtPointer client_data)
{
   gt_cmdli_panel *ptr = (gt_cmdli_panel *)client_data;

   // create a pause command
   sara::CMDLcommand cmd;
   cmd.opcon = ptr->comobj->getName();
   cmd.opconID = ptr->nextID++;
   cmd.action = sara::CMDLcommand::CMD_PAUSE;

   // send it
   ptr->sendCommand(cmd);
}

// ******************************************************************
/// resume the robots
void 
gt_cmdli_panel::resume_button_cb(Widget w, XtPointer client_data)
{
   gt_cmdli_panel *ptr = (gt_cmdli_panel *)client_data;

   // create a pause command
   sara::CMDLcommand cmd;
   cmd.opcon = ptr->comobj->getName();
   cmd.opconID = ptr->nextID++;
   cmd.action = sara::CMDLcommand::CMD_RESUME;

   // send it
   ptr->sendCommand(cmd);
}

// ******************************************************************
/// abort the mission
void 
gt_cmdli_panel::abort_command_execution_cb(Widget w, XtPointer client_data)
{
   gt_cmdli_panel *ptr = (gt_cmdli_panel *)client_data;

   // create a stop command
   sara::CMDLcommand cmd;
   cmd.opcon = ptr->comobj->getName();
   cmd.opconID = ptr->nextID++;
   cmd.action = sara::CMDLcommand::CMD_STOP;

   // send it
   ptr->sendCommand(cmd);

   // this clears the robots' ignore lists, so clear ours too.
   addrList::iterator it;
   for(it=ptr->teamList.begin(); it!=ptr->teamList.end(); ++it)
   {
      XmString str = XmStringCreateLocalized("Drop");
         XtVaSetValues( it->second.killButton, 
	    XmNbackground, gColorPixel.yellow,
	    XmNlabelString, str, 
	    NULL );
         XmStringFree(str);
   }
}

// ******************************************************************
/// start the new mission
void 
gt_cmdli_panel::start_mission_cb(Widget w, XtPointer client_data)
{
   gt_cmdli_panel *ptr = (gt_cmdli_panel *)client_data;

   // create a pause command
   sara::CMDLcommand cmd;
   cmd.opcon = ptr->comobj->getName();
   cmd.opconID = ptr->nextID++;
   cmd.action = sara::CMDLcommand::CMD_START;

   // send it
   ptr->sendCommand(cmd);

   // this clears the robots' ignore lists, so clear ours too.
   addrList::iterator it;
   for(it=ptr->teamList.begin(); it!=ptr->teamList.end(); ++it)
   {
      XmString str = XmStringCreateLocalized("Drop");
         XtVaSetValues( it->second.killButton, 
	    XmNbackground, gColorPixel.yellow,
	    XmNlabelString, str, 
	    NULL );
         XmStringFree(str);
   }
}

// ******************************************************************
/// download a new mission
void 
gt_cmdli_panel::download_cmdli_file_cb(Widget w, XtPointer client_data)
{
   gt_cmdli_panel *ptr = (gt_cmdli_panel *)client_data;

   // can't send it if we don't have a file loaded
   if( ptr->thecmdl )
   {
      // create the command
      sara::CMDLcommand cmd;
      cmd.opcon = ptr->comobj->getName();
      cmd.opconID = ptr->nextID++;
      cmd.action = sara::CMDLcommand::CMD_DOWNLOAD;
      stringstream mission;
//      mission << *ptr->thecmdl;
      mission << ptr->theCmdlFileData;
      cmd.data = mission.str();

      // send it
      ptr->sendCommand(cmd);
   }

   // this clears the robots' ignore lists, so clear ours too.
   addrList::iterator it;
   for(it=ptr->teamList.begin(); it!=ptr->teamList.end(); ++it)
   {
      XmString str = XmStringCreateLocalized("Drop");
         XtVaSetValues( it->second.killButton, 
	    XmNbackground, gColorPixel.yellow,
	    XmNlabelString, str, 
	    NULL );
         XmStringFree(str);
   }
}

// ******************************************************************
// Make sure that the kill button was not accidentally pressed.
void gt_cmdli_panel::confirm_kill_robot_cb(Widget w, XtPointer client_data)
{
    XmString dropString, cancelString, msgString;
    gt_cmdli_panel *ptr;
    confirm_kill_robot_data_t *callbackData = NULL;

    ptr = (gt_cmdli_panel *)client_data;

    Widget confirmKillDialog = XmCreateQuestionDialog(
        ptr->parent,
        "confirmKillDialog",
        NULL,
        0);

    dropString = XSTRING("   Drop   ");
    cancelString = XSTRING("   Cancel   ");   
    msgString = XSTRING("Are you sure you want to drop the robot out of the team?");

    XtVaSetValues (
        confirmKillDialog,
        XmNmessageString, msgString,
        XmNokLabelString, dropString,
        XmNcancelLabelString, cancelString,
        NULL);

    callbackData = new confirm_kill_robot_data_t;
    callbackData->cmdliPanelInstance = ptr;
    callbackData->w = w;

    XtAddCallback(
        confirmKillDialog,
        XmNokCallback,
        (XtCallbackProc)kill_robot_cb,
        callbackData);

    XmStringFree(dropString);
    XmStringFree(cancelString);
    XmStringFree(msgString);

    XtUnmanageChild(XmMessageBoxGetChild(confirmKillDialog, XmDIALOG_HELP_BUTTON));

    XtManageChild(confirmKillDialog);
}

// ******************************************************************
// Tell the robot to stop, and tell its teammembers to start ignoring it.
void gt_cmdli_panel::kill_robot_cb(Widget w, XtPointer client_data)
{
    confirm_kill_robot_data_t *callbackData = NULL;
    gt_cmdli_panel *ptr = NULL;

    callbackData = (confirm_kill_robot_data_t *)client_data;
    ptr = callbackData->cmdliPanelInstance;

    addrList::iterator it;

    for(it=ptr->teamList.begin(); it!=ptr->teamList.end(); ++it)
    {
        if( callbackData->w == it->second.killButton )
        {
            break;
        }
    }

    if( it!=ptr->teamList.end() )
    {
        // create an ignore command
        sara::CMDLcommand cmd;
        cmd.opcon = ptr->comobj->getName();
        cmd.opconID = ptr->nextID++;
        cmd.action = sara::CMDLcommand::CMD_IGNOREROBOT;

        // the name of the robot to ignore
        cmd.data = it->first;

        // update the button to show it has been pushed
        XmString str = XmStringCreateLocalized("Out!");
        XtVaSetValues( it->second.killButton, 
                       XmNbackground, gColorPixel.red,
                       XmNlabelString, str, 
                       NULL );
        XmStringFree(str);

        // send it
        ptr->sendCommand(cmd);
    }
    else
    {
        warn_userf("gt_cmdli_panel::kill_robot_cb - Internal error: Didn't find the entry in teamList");
        return;
    }
}

// ****************************************************************************
// send a message to our team
// return true if all receipients acknowledged the message
//        false otherwise. 
// Updates the screen status flags to show who responded
bool 
gt_cmdli_panel::sendCommand(const sara::CMDLcommand &theirCmd)
{
   if( sendCommandActive )
      return false;

   sendCommandActive = true;
   sara::CMDLcommand *cmd = (sara::CMDLcommand *)theirCmd.clone();

   // timeouts
   static const sara::TimeOfDay AckMsg_Timeout(2.0);
   static const int Max_MsgSendRetries = 5;

   // init
   addrList::iterator it;
   uint numAcks = 0;
   for(it=teamList.begin(); it!=teamList.end(); ++it)
   {
      // have we ever heard from them?
      if( it->second.record )
      {
         it->second.gotCmdAck = false;

         // update the status color and timer
         char buf[2] = "_";
         XmString str = XmStringCreateLocalized(buf);
         XtVaSetValues( it->second.cmdStatusWidget, 
            XmNbackground, gColorPixel.yellow, 
            XmNlabelString, str, 
            NULL );
         XmStringFree(str);
      }
      else
      {
	 // no. just ignore them.
         it->second.gotCmdAck = true;
	 numAcks++;
      }
   }

   // Register for acks 
   sara::objcomms::QueueingHandle AckCommandHandle = comobj->beginQueueing(sara::MsgTypes::CMDLCOMMANDACK);
   if( AckCommandHandle == NULL )
   {
      warn_userf("Internal Error: beginQueueing failed - Unable to register for CMDLcommandAck messages!");
      sendCommandActive = false;
      return false;
   }

   // set up our timers
   sara::TimeOfDay endTime;
   sara::TimeOfDay remainingTime;
   sara::TimeOfDay timeOfTaskStatus;

   // The current retry
   int retry = 0;
   enum {S_SEND=0, S_WAITFORACKS} curstate = S_SEND;

   bool giveup = false;
   while( numAcks < teamList.size() && !giveup)
   {
      switch(curstate)
      {
         case S_SEND:
	 {
            // send/re-send the command 
            if( !comobj->broadcastMsg(*cmd) )
            {
               warn_userf("Internal Error: broadcast message failed! Unable to command robots.");
	       giveup = true;
	       continue;
            }

            // init the timer
            endTime = sara::TimeOfDay::now() + AckMsg_Timeout;
            curstate = S_WAITFORACKS;
            break;
         }

         case S_WAITFORACKS:
         {
	    remainingTime = endTime - sara::TimeOfDay::now();
	    sara::commMsg *msg = comobj->waitFor(AckCommandHandle, remainingTime);
	    if( msg )
	    {
	       // Got one
	       sara::CMDLcommandAck *ack = dynamic_cast<sara::CMDLcommandAck *>(msg);
               if( ack == NULL )
               {
                  warn_userf("Unable to upcast commMsg to CMDLcommandAck!");
		  delete msg;
	       }
	       else if( ack->opcon != cmd->opcon || ack->opconID != cmd->opconID )
	       {
		  // ignoring extraneous ack
	          delete msg;
	       }
	       else
	       {
	          if( ack->response != sara::CMDLcommandAck::OK )
	          {
		     stringstream str;
                     str << "robot " << ack->senderAddr.str() << " returned a response of ";
                     switch( ack->response )
                     {
			case sara::CMDLcommandAck::INVALID:
                        str << "FAILED";
                        break;

			case sara::CMDLcommandAck::FAILED:
                        str << "FAILED";
                        break;

			case sara::CMDLcommandAck::OK:
                        str << "OK";
                        break;
                     }
                     warn_userf(str.str().c_str());
	          }

                  // see if we can find them
                  addrList::iterator it = teamList.find(ack->name);
                  if( it == teamList.end() )
	          {
		     stringstream str;
                     str << "ignoring ack from " << ack->name << " at " << ack->senderAddr.str() << " - not from one of our team members!";
                     warn_userf(str.str().c_str());
	          }
                  else if( it->second.gotCmdAck == false )
                  {
                     it->second.gotCmdAck = true;
                     numAcks ++;

		     // add their name to the outbound packet to shut them up on retries.
		     cmd->gotAcks.insert( ack->name );

		     // update the display
	             char buf[2] = "A";
	             Pixel newcolor = gColorPixel.green;
	             if( ack->response != sara::CMDLcommandAck::OK )
		     {
	                buf[0] = 'F';
	                newcolor = gColorPixel.red;
		     }

                     // update the status color and timer
	             XmString str = XmStringCreateLocalized(buf);
                     XtVaSetValues( it->second.cmdStatusWidget, 
	                   XmNbackground, newcolor, 
	                   XmNlabelString, str, 
	                   NULL );
	             XmStringFree(str);
                  }
	       }
	    }

            // If time has expired, and we don't have them all, then check if we have retries left.
	    remainingTime = endTime - sara::TimeOfDay::now();
	    if( !remainingTime.isTimeLeft() )
	    {
               if( ++retry < Max_MsgSendRetries )
               {
//                  fprintf(stderr,".");
                  curstate = S_SEND;
               }
               else
               {
                  // nope, out of luck
                  giveup = true;
               }
	    }
         }
         break;
      }

      // crank X to update the display
      int cnt=0;
      while ( XPending( XtDisplay( main_window ) ) > 0 && cnt++ < 1000 )
      {
         XEvent event;
         XNextEvent( XtDisplay( main_window ), &event );
         XtDispatchEvent( &event );
      }
   }

   // Cleanup
   if( AckCommandHandle )
   {
      comobj->endQueueing(AckCommandHandle);
      AckCommandHandle = NULL;
   }

   // finish up by marking any that failed
   for(it=teamList.begin(); it!=teamList.end(); ++it)
   {
      if( it->second.gotCmdAck == false )
      {
         char buf[2] = "F";
         XmString str = XmStringCreateLocalized(buf);
         XtVaSetValues( it->second.cmdStatusWidget, 
            XmNbackground, gColorPixel.red, 
            XmNlabelString, str, 
            NULL );
         XmStringFree(str);
      }
   }

   // return true if we got all the acks
   sendCommandActive = false;
   return numAcks == teamList.size();
}

/**********************************************************************/
void 
gt_cmdli_panel::check_cmdli_filename_field_cb(Widget w, XtPointer client_data)
{
   gt_cmdli_panel *ptr = (gt_cmdli_panel *)client_data;

   if ( !ptr->cmdli_filename_field_touched )
   {
      XmTextFieldSetString( ptr->cmdli_filename_field, "" );
      ptr->cmdli_filename_field_touched = true;
   }
}

/**********************************************************************/
void 
gt_cmdli_panel::load_cmdli_file_cb(Widget w, XtPointer client_data)
{
    cerr << "gt_cmdli_panel::load_cmdli_file_cb - starting" << endl;
    gt_cmdli_panel *ptr = (gt_cmdli_panel *)client_data;

    //char msg[ 256 ];
    if ( ptr->cmdli_filename_field_touched )
    {
        // make sure we have a filename to read from
        char *filename = XmTextFieldGetString( ptr->cmdli_filename_field );
        if ( ( filename == NULL ) || ( strlen( filename ) == 0 ) )
        {
            warn_user( "Commands NOT loaded successfully: No filename entered yet!" );
            if ( filename ) 
            {
                XtFree( filename );
            }
            return;
        }

        if( !ptr->load_from_file(filename) )
        {
            warn_userf("Unable to load the cmdl file '%s'!", filename);
        }
        else
        {
            warn_userf("File '%s' successfully loaded", filename);
        }

        // cleanup
        XtFree( filename );
    }
    else
    {
        warn_user( "Unable to load file: No filename entered yet!\n" );
    }
}


/**********************************************************************
 * $Log: gt_cmdli_panel.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/07/11 10:07:49  endo
 * CMDLi Panel merged from MARS 2020.
 *
 * Revision 1.6  2004/11/22 00:20:38  endo
 * Checked in for Doug to support CMDLi v5.
 *
 * Revision 1.5  2004/11/20 23:48:33  endo
 * Checked in for Doug.
 *
 * Revision 1.4  2004/11/18 05:34:37  endo
 * The width adjusted.
 *
 * Revision 1.3  2004/11/17 04:24:43  endo
 * confirm_kill_robot_cb added.
 *
 * Revision 1.2  2004/11/15 21:49:53  endo
 * Checked in for Doug. The CMDLi dialog improved.
 *
 * Revision 1.1  2004/11/12 21:00:44  endo
 * Support for the CMDLi dialog added.
 *
 **********************************************************************/
