/**********************************************************************
 **                                                                  **
 **                            mlab_cnp.cc                           **
 **                                                                  **
 **  Written by:       Yoichiro Endo                                 **
 **  Based on code by: Patrick Ulam                                  **
 **                                                                  **
 **  This class deals with CNP related functions within mlab.        **
 **                                                                  **
 **  Copyright 2005 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: mlab_cnp.cc,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "mlab_cnp.h"
#include "gt_sim.h"

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string MlabCNP::EMPTY_STRING_ = "";
const string MlabCNP::STRING_CNP_TRACK_RED_ = "TRACKRED";
const string MlabCNP::STRING_CNP_TRACK_YELLOW_ = "TRACKYELLOW";
const string MlabCNP::STRING_CNP_TRACK_DEFAULT_REQUIREMENTS_ = "Land";
const int MlabCNP::CHECK_EXPIRE_LOOP_TIMER_MSEC_ = 100;

//-----------------------------------------------------------------------
// Constructor for MlabCNP class.
//-----------------------------------------------------------------------
MlabCNP::MlabCNP(XtAppContext xtApp) :
    xtApp_(xtApp),
    cnpServerManager_(NULL),
    cnpTaskID_(-1)
{
    // Instantiate classes
  cnpServerManager_ = new CNP_Server_Manager();

    // Add a work process for checking CNP expiration
    startCheckExpireLoop_();
}

//-----------------------------------------------------------------------
// Distructor for MlabCBRClient class.
//-----------------------------------------------------------------------
MlabCNP::~MlabCNP(void)
{
    delete cnpServerManager_;
}


//-----------------------------------------------------------------------
// This function checks to see if the robot got a task.
//-----------------------------------------------------------------------
bool MlabCNP::checkForTask(int robotID, MlabCNP_RequestMess_t *mess)
{
  std::vector<CNP_OpenRequest> req;

  req = cnpServerManager_->Check_ForTask(robotID);
  
  if (req.size() == 0)
    {
      return false;
    }
  
  // PU: Note this is arbitrary due to the possibility of being qualified for additional tasks
  mess->TaskID = req[0].TaskID;
  mess->Iteration = req[0].Iteration;
  mess->TaskDesc = req[0].TaskType;
  mess->Restriction = req[0].Requirements;

  return true;
}


//-----------------------------------------------------------------------
// This function cancels the task
//-----------------------------------------------------------------------
void MlabCNP::cancelTask(CNP_Cancel canc)
{
    const bool DEBUG_CANCEL_TASK = false;

    // remove the award from the list
    list<CNP_Award>::iterator awit = cnpAwardList_.begin();
    bool doneflag = false;
    while (!doneflag) {
        if (awit == cnpAwardList_.end()) {
            doneflag = true;
        }
        else {
            if ((awit->TaskID == canc.TaskID) && 
                (awit->Iteration == canc.Iteration)) {
                cnpAwardList_.erase(awit);
                doneflag = true;
            }
            else {
                awit++;
            }
        }
    }
    
    cnpServerManager_->Cancel_Task(&canc, cnpTaskID_);
    
    if (DEBUG_CANCEL_TASK)
    {
        fprintf(stderr, "MlabCNP::cancelTask(): Task canceled.\n");
    }
    if (canc.Info != NULL) {
        ReinjectTask(canc.TaskID);
    }
}


//-----------------------------------------------------------------------
// This function indicates that a task has been completed successfully
//-----------------------------------------------------------------------
void MlabCNP::completeTask(int taskid)
{
    list<CNP_Award>::iterator awit = cnpAwardList_.begin();
    while (awit != cnpAwardList_.end()) {
      if (awit->TaskID == taskid) {
	cnpAwardList_.erase(awit);
	break;
      }
      awit++;
    }
    
    cnpServerManager_->Complete_Task(taskid);
}


//-----------------------------------------------------------------------
// This function bids the task
//-----------------------------------------------------------------------
void MlabCNP::bidTask(CNP_Offer &offer)
{
    CNP_Offer cnpOffer;
    cnpOffer.TaskID = offer.TaskID;
    cnpOffer.Iteration = offer.Iteration;
    cnpOffer.RobotID = offer.RobotID;
    cnpOffer.TaskAccept = offer.TaskAccept;
    cnpOffer.Bid = offer.Bid;

    cnpServerManager_->Receive_Offer(cnpOffer);
}

//-----------------------------------------------------------------------
// This function gets the award
//-----------------------------------------------------------------------
bool MlabCNP::getAward(CNP_Award *currentAward)
{
    char buf[1024];
    bool foundAward = false;
    const bool DEBUG_GET_AWARD = false;

    list<CNP_Award>::iterator awit = cnpAwardList_.begin();

    if (DEBUG_GET_AWARD)
    {
        fprintf(stderr, "MlabCNP::getAward():\n");
    }

    while (awit != cnpAwardList_.end())
    {
        if (DEBUG_GET_AWARD)
        {
            fprintf(stderr, " awit->RobotID = %d | TaskID = %d | Iteration = %d\n", awit->RobotID, awit->TaskID, awit->Iteration);
            fprintf(stderr, " currentAward->RobotID = %d | TaskID = %d | Iteration = %d\n", currentAward->RobotID, currentAward->TaskID, currentAward->Iteration);
        }

        if (((awit->TaskID) == (currentAward->TaskID)) &&
            ((awit->Iteration) == (currentAward->Iteration)))
        {
            foundAward = true;

            if (currentAward->RobotID == awit->RobotID)
            {
                if (awit->RobotID != -2)
                {
                    sprintf(
                        buf,
                        "Task(%d) awarded to Robot(%d)\n",
                        awit->TaskID,
                        awit->RobotID);
	  
                    sim_alert(buf);
                }
                else
                {
                    // Nobody won the auction.
                    cnpAwardList_.erase(awit);
                }
            }
            else
            {
                currentAward->RobotID = awit->RobotID;
            }

            break;
        }

        awit++;
    } 
    
    if (!foundAward)
    {
        currentAward->RobotID = -1;
    }

    return foundAward;
}


//-----------------------------------------------------------------------
// This function checks the expiration status by running the function
// as XtWorkProc.
//-----------------------------------------------------------------------
void MlabCNP::startCheckExpireLoop_(void)
{
    XtAppAddTimeOut(
        xtApp_,
        CHECK_EXPIRE_LOOP_TIMER_MSEC_,
        (XtTimerCallbackProc)cbCheckExpire_,
        this);
}

//-----------------------------------------------------------------------
// This function is an XtWorkProc. It checks the expiration status.
//-----------------------------------------------------------------------
XtTimerCallbackProc MlabCNP::cbCheckExpire_(XtPointer clientData)
{
    MlabCNP *mlabCNPInstance_ = NULL;
    CNP_Award award;
    const bool DEBUG_CB_CHECK_EXPIRE = false;

    if (paused)
    {
        return false;
    }

    mlabCNPInstance_ = (MlabCNP *)clientData;
    award = mlabCNPInstance_->cnpServerManager_->Check_Expire();

    if (DEBUG_CB_CHECK_EXPIRE)
    {
        fprintf(stderr, "MlabCNP::cbCheckExpire_(): award.RobotID = %d\n", award.RobotID);
    }

    if (award.RobotID != -1)
    {
        mlabCNPInstance_->cnpAwardList_.push_back(award);
    }

    mlabCNPInstance_->startCheckExpireLoop_();

    return false;                                                                                               
}

//-----------------------------------------------------------------------
// This function save initial CNP robot-constraints in the data.
//-----------------------------------------------------------------------
void MlabCNP::saveInitCNPRobotConstraints(
    int robotID,
    vector<CNP_RobotConstraint> initCNPRobotConstraints)
{
    InitCNPRobotConstraintsEntry_t entry;
    int i;
    bool entryFound = false;

    for (i = 0; i < (int)(initCNPRobotConstraintsList_.size()); i++)
    {
        if (initCNPRobotConstraintsList_[i].robotID == robotID)
        {
            initCNPRobotConstraintsList_[i].initCNPRobotConstraints = 
                initCNPRobotConstraints;
            entryFound = true;
            break;
        }
    }

    if (!entryFound)
    {
        entry.robotID = robotID;
        entry.initCNPRobotConstraints = initCNPRobotConstraints;

        initCNPRobotConstraintsList_.push_back(entry);
    }
}

//-----------------------------------------------------------------------
// This function retrieves initial CNP robot-constraints in the data.
//-----------------------------------------------------------------------
vector<CNP_RobotConstraint> MlabCNP::getInitCNPRobotConstraints(int robotID)
{
    vector<CNP_RobotConstraint> initCNPRobotConstraints;
    int i;

    for (i = 0; i < (int)(initCNPRobotConstraintsList_.size()); i++)
    {
        if (initCNPRobotConstraintsList_[i].robotID == robotID)
        {
            initCNPRobotConstraints = 
                initCNPRobotConstraintsList_[i].initCNPRobotConstraints;
            break;
        }
    }

    return initCNPRobotConstraints;
}






// -------------------------------------------------------------------------
// PU: Modifications to 1) handle multiple tasks executing at once   2) retrieve/parse task constraints from the cnp
//            server
//-----------------------------------------------------------------------
// This function saves the current task constraints
//-----------------------------------------------------------------------
void MlabCNP::saveCurrentCNPTaskConstraints(
    list<CNP_TaskConstraint> cnpTaskConstraints)
{
    CNP_Request req;
    list<CNP_TaskConstraint>::const_iterator iterTC;
    string taskName;
    char buf[1024];
    int taskID = -1;
    int errorNum = 0;
    bool entryFound = false, hadError = false;
    const string DEFAULT_TASK_NAME = "None";
    const string DEFAULT_RESTRICTION = "None";
    const int DEFAULT_ITERATION = 1;

    taskName = DEFAULT_TASK_NAME;

    for (iterTC = cnpTaskConstraints.begin();
         iterTC != cnpTaskConstraints.end();
         iterTC++)
    {
        if (!entryFound)
        {
            taskName = iterTC->taskInfo.name;
            taskID = iterTC->taskInfo.id;
            entryFound = true;
            continue;
        }

        if (((iterTC->taskInfo.id) != taskID) &&
            ((iterTC->taskInfo.name) != taskName))
        {
            // The ID and name should be consistent through out the list.
            hadError = true;
            errorNum = 1;
            break;
        }
    }

    if ((!entryFound) || hadError)
    {
        fprintf(
            stderr,
            "Error(%d): MlabCNP::saveCurrentCNPTaskConstraints(). Error in the input.\n",
            errorNum);
        return;
    }


    // first we check to see if the task requested is a reinjection of a renegged task
    //    CNP_Request *renegreq = cnpServerManager_->Is_TaskReinjection(taskID);
    //    if (renegreq != NULL) {
    if (false) {
      //      req = *renegreq;
      //      cnpServerManager_->Remove_Task(taskID);

      // convert text to constraints

      //      currentCNPTaskConstraints_ = cnpServerManager_->Get_TaskConstraints(taskID);
    }

    
    // if it is not a renegged task then we handle it normally
    else {
      // Save the task ID.
      currentCNPTaskConstraints_ = cnpTaskConstraints;
      cnpTaskID_ = taskID;
      
      // Marshall the input.
      req.TaskID = cnpTaskID_;
      req.Iteration = DEFAULT_ITERATION;
      req.TaskType = taskName.c_str(); 
      req.Requirements = DEFAULT_RESTRICTION; 
    }

    if (cnpServerManager_->Inject_Task(req))
    {
        sprintf(
            buf,
            "Task(%d) injected: [%s]\n",
            req.TaskID,
            req.TaskType.c_str());

        sim_alert(buf);
    }
    else
    {
        errorNum = 3;
        fprintf(
            stderr,
            "Error(%d): MlabCNP::saveCurrentCNPTaskConstraints(). Task Injection Failed.\n",
            errorNum);
        return;
    }
 }

//-----------------------------------------------------------------------
// This function saves the current task constraints
//-----------------------------------------------------------------------
vector<CNP_TaskConstraint> MlabCNP::getCurrentCNPTaskConstraints(void)
{
   
   vector<CNP_TaskConstraint> vec;
   list<CNP_TaskConstraint>::const_iterator iterTC;

    for (iterTC = currentCNPTaskConstraints_.begin();
         iterTC != currentCNPTaskConstraints_.end();
         iterTC++)
    {
        vec.push_back(*iterTC);
    }

    return vec;
}

//-----------------------------------------------------------------------
// This function prints out the debugging message that is called inside
// CNP library.
//-----------------------------------------------------------------------
void CNPPrintf(const char *format, ...)
{
    va_list args;
    char buf[4096];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    fprintf(stderr, "CNP: %s", buf);
}




//-----------------------------------------------------------------------
// This function saves the current task constraints and reinjects a task
//-----------------------------------------------------------------------
void MlabCNP::ReinjectTask(int tid) {
    CNP_Request req;
    list<CNP_TaskConstraint>::const_iterator iterTC;
    string taskName;
    char buf[1024];
    int errorNum = 0;
    const string DEFAULT_TASK_NAME = "None";
    const string DEFAULT_RESTRICTION = "None";
 
    // first we check to see if the task requested is a reinjection of a renegged task
    CNP_Request *renegreq = cnpServerManager_->Is_TaskReinjection(tid);
    if (renegreq != NULL) {
        req = *renegreq;
  
    
        // convert text to constraints
        currentCNPTaskConstraints_ = cnpServerManager_->Get_TaskConstraints(tid);
    }
    else {
        errorNum = 1;
        fprintf(
            stderr,
            "Error(%d): MlabCNP::Reinject Task(). Task Injection Failed.\n",
            errorNum);
        return;
    }    
    if (cnpServerManager_->Inject_Task(req)) {
        sprintf(
            buf,
            "Task(%d) reinjected: [%s]\n",
            req.TaskID,
            req.TaskType.c_str());
    
        sim_alert(buf);
    }
    else {
        errorNum = 2;
        fprintf(
            stderr,
            "Error(%d): MlabCNP::Reinject Task(). Task Injection Failed.\n",
            errorNum);
        return;
    }
    // cnpServerManager_->Remove_Task(tid); 
}





/**********************************************************************
 * $Log: mlab_cnp.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.9  2006/05/15 02:07:35  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.8  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.7  2006/05/06 04:25:14  endo
 * Bugs of TrackTask fixed.
 *
 * Revision 1.6  2006/05/02 04:19:59  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.5  2006/05/01 19:49:12  pulam
 * *** empty log message ***
 *
 * Revision 1.4  2006/04/28 22:50:42  pulam
 * Constraint checking update, Terrainmap disable for large maps, renegging overhaul
 *
 * Revision 1.3  2006/01/19 01:45:41  pulam
 * Cleaned up CNP code
 *
 * Revision 1.2  2006/01/12 20:32:44  pulam
 * cnp cleanup
 *
 * Revision 1.1  2006/01/10 06:10:31  endo
 * AO-FNC Type-I check-in.
 *
 **********************************************************************/
