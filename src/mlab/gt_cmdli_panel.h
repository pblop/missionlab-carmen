/**********************************************************************
 **                                                                  **
 **                        gt_cmdli_panel.h                          **
 **                                                                  **
 **  Written by: Douglas C. MacKenzie                                **
 **                                                                  **
 **  Copyright 2004 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_cmdli_panel.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $  */

#ifndef GT_cmdli_panel_H
#define GT_cmdli_panel_H

#include <X11/Intrinsic.h>
#include <string>
#include <map>


#include "gt_command.h"
//#include "ComAddr.h"
#include "objcomms.h"
#include "TimeOfDay.h"
#include "semaphore.h"
#include "commMsg.h"
#include "CMDLstatus.h"
#include "CMDLcommand.h"
#include "cmdl.h"
#include "resources.h"

using std::string;

// ******************************************************************
class gt_cmdli_panel
{
   typedef struct confirm_kill_robot_data_t {
       gt_cmdli_panel *cmdliPanelInstance;
       Widget w;
   };

   public:
      // constructors and destructors
      gt_cmdli_panel(Widget parent);
      ~gt_cmdli_panel();

      // display the panel created in the open_panel command
      void popup_panel();

      // hide the panel
      void popdown_panel();

/*
      void gt_display_step( gt_Step* this_cmd_list,
                            gt_Step_status step_status,
                            gt_Step* next_cmd_list );


      void set_cmdli_panel_filename( char* filename );

      void update_pause_button(void);
      void toggle_pause_execution_button(void);

      void EnablePauseButton( bool bEnable );
      void update_feedback_button(void); // ENDO

      const string FEEDBACK_BUTTON_LABEL_STOP = "Cut-Off Feedback";
      const string FEEDBACK_BUTTON_LABEL_NONSTOP = "Receive Feedback";
*/

      // called on a periodic basis to update the status display
      // returns true if done (panel is not being displayed)
      bool update_display();

      /// Load the cmdl from the specified file.
      /// No changes are made unless it returns true.
      bool load_from_file(const string &filename);

      void set_cmdli_panel_filename( const string &filename );

   private:
      // create the panel and display it
      // returns true on success, false if was error
      bool create_panel();

      /// callback when pause button is toggled
      static void toggle_pause_execution_button_cb();

      static void execute_user_command_cb();

      static void popup_panel_cb(Widget w, XtPointer client_data);
      static void popdown_panel_cb(Widget w, XtPointer client_data);

      /// download a new mission
      static void download_cmdli_file_cb(Widget w, XtPointer client_data);

      /// callback function to update our status cache as messages arrive
      static void updateStatus_cb(sara::commMsg *msg, void *ptr);

      // Tell the robot to stop, and tell its teammembers to start ignoring it.
      static void kill_robot_cb(Widget w, XtPointer client_data);

      // Make sure that the kill button was accidentally pressed.
      static void confirm_kill_robot_cb(Widget w, XtPointer client_data);

      /// abort the mission
      static void abort_command_execution_cb(Widget w, XtPointer client_data);

      /// callback when pause button is toggled
      static void toggle_pause_execution_button_cb(XtPointer client_data);

      // send a message to our team
      // return true if all receipients acknowledged the message
      //        false otherwise. 
      // Updates the screen status flags to show who responded
      bool sendCommand(const sara::CMDLcommand &cmd);

      /// callback to pause the robots
      static void pause_button_cb(Widget w, XtPointer client_data);

      /// callback to resume the robots
      static void resume_button_cb(Widget w, XtPointer client_data);

      /// start the new mission
      static void start_mission_cb(Widget w, XtPointer client_data);

      static void check_cmdli_filename_field_cb(Widget w, XtPointer client_data);
      static void load_cmdli_file_cb(Widget w, XtPointer client_data);

      /// our parent widget
      Widget parent;

      /// handle to the popup
      Widget cmdli_panel_shell;

      /// child for cmdli_panel_shell
      Widget cmdli_panel;

      /// holds the rows of robot records
      Widget robotrows;

      /// the bottom widget in the robotrows form
      Widget bottom_widget;

      /// Used to force recomputing the layout
      Widget name_title;

      /// Where they enter a filename to download
      Widget cmdli_filename_field;

      /// have we created the popup yet?
      bool created;

      /// the current info for each robot
      class status
      {
	 public:
            status() : statusWidget(NULL), record(NULL) {};
            ~status() {if( record ) delete record; record = NULL;}

	    // statusWidget is null if we haven't created a line for them yet on the display
	    Widget        killButton;

	    // the time since the last packet
	    Widget        statusWidget;
	    Pixel         currentColor;
	    uint          currentValue;  // show as -- if > 99.

	    Widget        cmdStatusWidget;

	    // shows the cmdli interpreter state
	    Widget        cmdiStateWidget;
	    int           currentCmdiState;

	    Widget        statusString;

	    Widget	  waitListWidget;

	    string        currentWaitValue;
	    string        currentCmdValue;

	    // their IP address
	    sara::ComAddr addr;
	    
	    // the last status record we got from them, and when we got it.
	    sara::CMDLstatus *record;

	    // the wall clock time when this packet arrived.
	    sara::TimeOfDay   arrivalTime;

	    // the time between this packet and the one before it. 0 if first one.
	    sara::TimeOfDay   lastPeriod;

	    // The status of the last command. True: Acked, False, no show
	    bool gotCmdAck;
      };
      typedef std::map< string, status > addrList;
      addrList teamList;
      sara::semaphore teamListLock;

      /// our comms handle
      sara::objcomms *comobj;

      /// Add a new row into the robot display
      /// NOTE: The caller must hold a lock on TeamList before calling this function!
      void addRow(addrList::iterator it);

      /// is the status window being displayed?
      bool isUp;

      /// a message number counter
      unsigned short nextID;

      /// used to track changes to the text entry box
      bool cmdli_filename_field_touched;
      /// the filename
      string cmdli_filename;

      /// Copy of the mission the robots are now running
      sara::cmdl *thecmdl;

      /// copy of raw text from file loaded into thecmdl
      string theCmdlFileData;

      /// Try to prevent overlapping commands.
      bool sendCommandActive;
      
      sara::resources cmdli_rc_table;
};

#endif

/**********************************************************************
 * $Log: gt_cmdli_panel.h,v $
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
 * Revision 1.4  2004/11/22 00:20:38  endo
 * Checked in for Doug to support CMDLi v5.
 *
 * Revision 1.3  2004/11/17 04:24:43  endo
 * confirm_kill_robot_cb added.
 *
 * Revision 1.2  2004/11/15 21:49:54  endo
 * Checked in for Doug. The CMDLi dialog improved.
 *
 * Revision 1.1  2004/11/12 21:00:44  endo
 * Support for the CMDLi dialog added.
 *
 **********************************************************************/
