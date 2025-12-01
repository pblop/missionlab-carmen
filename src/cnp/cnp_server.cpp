/**********************************************************************
 **                                                                  **
 **                           cnp_server.cpp                         **
 **                                                                  **
 **  Written by: Patrick Ulam                                        **
 **                                                                  **
 **  Copyright 2005 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <list>
#include <map>
#include <stdlib.h>
#include <time.h>

#include "../mlab/gt_sim.h"
#include "cnp_server.h"
#include "cnp_test_define.h"


using namespace std;


CNP_Server::CNP_Server(void) {

  srand(time(NULL));
}

CNP_Server::~CNP_Server(void) {
  // empty
}


// adds a task for bidding
bool CNP_Server::Inject_Task(CNP_Request &request) {
  CNP_OpenRequest oreq;
  CNP_Request *req;

  // make sure it isn't a duplicate task
  if (Is_DuplicateTask(request)) {
     return false;
  }

  // now check to see it isnt a reneged task
  req = Is_ReneggedTask(request.TaskID);
  if (req != NULL) {
    request = *req;
  }
  
  // put it in the open contracts list and send the offer out to the robots
  oreq.TaskID = request.TaskID;
  oreq.Iteration = request.Iteration;
  oreq.TaskType = request.TaskType;
  oreq.Requirements = request.Requirements;
  oreq.Exclusions = request.Exclusions;
  
  // PU: TODO fix expiration time
  oreq.Expire = 20;
  Open_Contracts[oreq.TaskID] = oreq;

  return true;
}


// checks to see if a given task id has been renegged. if it has it returns a pointer to the CNP_Request
CNP_Request *CNP_Server::Is_ReneggedTask(int tid) {

  // first check the open auctions
  std::map<int, CNP_Request>::iterator oreqit = Renegged_Contracts.find(tid);

  if (oreqit != Renegged_Contracts.end()) {
    return &(oreqit->second);
  }
  
  return NULL;
}

list<CNP_TaskConstraint> CNP_Server::Get_TaskConstraints(int tid) {
  std::list<CNP_TaskConstraint> cnpTaskConstraints;
  CNP_TaskConstraint cnpTaskConstraint; 
  string bufString, subBufString;
  string name, value, typeString, key;
  string::size_type colonIndex1, colonIndex2;
  string::size_type spaceIndex;
  string::size_type commaIndex;
  string::size_type newlineIndex;
  string::size_type separatorIndex;
  int subBufSize;
  int constraintType;
  int errorNum = 0;
  bool hadError = false;


  CNP_Request *req = Is_ReneggedTask(tid);
  if (req == NULL) {
    //fprintf(stderr, "Trying to get task constraints for non renegged task\n");
    return cnpTaskConstraints;
  }
  
  cnpTaskConstraint.taskInfo.id = req->TaskID;
  cnpTaskConstraint.taskInfo.name = req->TaskType;
  
  bufString = req->Requirements;

  // Delete white spaces
  while (true) {
    spaceIndex = bufString.find_first_of(' ');
    
    if (spaceIndex == string::npos) {
      // Done searching.
      break;
    }
    
    // Get the string after the space
    subBufString = bufString.substr(spaceIndex+1, string::npos);
    
    // Get the string before the string
    bufString = bufString.substr(0, spaceIndex);
    
    // Combine the two
    bufString += subBufString; 
  }
  while(true)     {
    // Find ','.
    commaIndex = bufString.find(',');
    
    // Find '\n'.
    newlineIndex = bufString.find('\n');
    
    // Pick which ever comes first.
    if (commaIndex <= newlineIndex)
      {
	separatorIndex = commaIndex;
      }
    else
      {
	separatorIndex = newlineIndex;
      }
    
    if (separatorIndex == string::npos)
      {
	// No more ',' or '\n' (i.e., the last set).
	subBufString = bufString;
      }
    else
      {
	// Get the string before ','.
	subBufString = bufString.substr(0, separatorIndex);
      }
    
    // Check the size.
    subBufSize = subBufString.size();
    
    if (subBufSize == 0)
      {
	if (separatorIndex == string::npos)
	  {
	    // It was perhaps already at the end. Finish it.
	    break;
	  }
	
	// Move on to the next set.
	bufString = bufString.substr(separatorIndex+1, string::npos);
	continue;
      }
    
    // Find ':'.
    colonIndex1 = subBufString.find_first_of(':');
    colonIndex2 = subBufString.find_last_of(':');
    
    if ((colonIndex1 == string::npos) || (colonIndex2 == string::npos))
      {
	errorNum = 1;
	hadError = true;
	break;
      }
    
    // Get the constraint name and value.
    name = subBufString.substr(0, colonIndex1);
    typeString = subBufString.substr(colonIndex1+1, colonIndex2-colonIndex1-1);
    constraintType = atoi(typeString.c_str());
    value = subBufString.substr(colonIndex2+1, string::npos);
    
    if (((int)(name.size()) == 0) ||
	((int)(typeString.size()) == 0) ||
	((int)(value.size()) == 0))
      {
	errorNum = 2;
	hadError = true;
	break;
      }
    if (value != "")
      {
	// Save them in the list.
	cnpTaskConstraint.strConstraintName = name;
	cnpTaskConstraint.constraintType = (CNP_ConstraintType)constraintType;
	cnpTaskConstraint.strConstraintValue = value;
	cnpTaskConstraints.push_back(cnpTaskConstraint);
      }
    
    if (bufString == subBufString)
      {
	// Done
	break;
      }
    else if ((int)(bufString.size()) >= (subBufSize+1))
      {
	// Move on to the next set.
	bufString = bufString.substr(subBufSize+1, string::npos);
      }
    else
      {
	// Had error
	errorNum = 3;
	hadError = true;
	break;
      }
  }
  
  if (hadError)
    {
      fprintf(stderr, "Warning: Error(%d) in CNP_Server:: problem parsing constraints\n", errorNum);
    }
  
  
  return cnpTaskConstraints;
}

// this makes sure that a task can't be requested twice
bool CNP_Server::Is_DuplicateTask(CNP_Request &request) {

  // first check the open auctions
  std::map<int, CNP_OpenRequest>::iterator oreqit = Open_Contracts.find(request.TaskID);
  if (oreqit != Open_Contracts.end()) {
    return true;
  }

  // next check the currently executing auctions
  std::map<int, CNP_ExecutingRequest>::iterator ereqit = Executing_Contracts.find(request.TaskID);
  if (ereqit != Executing_Contracts.end()) {
    return true;
  }

  return false;
}


// this handles reception of the offer(bid) for the contract
void CNP_Server::Receive_Offer(CNP_Offer &offer) {


  CNP_OfferType ofty = Match_TaskOffer(offer);
  

  // this is for the iterative bidding Im gonna comment this for now
  // if the offer provides new information
  if (ofty == NEW_INFO) {
    //    oreq = Find_OpenTask(offer->TaskID);
    
    //    oreq->Iteration = offer->Iteration;
    //    oreq->Info = offer->Info;
    //    Clear_Bidders(oreq);
    //    oreq->Bids->Insert_Rear((void *)offer);
    //    oreq->Expire = 20;
  }

}






// checks if an offer actually matches with open auctions
CNP_OfferType CNP_Server::Match_TaskOffer(CNP_Offer &offer) {
  
  std::map<int, CNP_OpenRequest>::iterator oreqit = Open_Contracts.find(offer.TaskID);
  if (oreqit == Open_Contracts.end()) {
    return NO_MATCH;
  }

  if (offer.Iteration == oreqit->second.Iteration) {

    if (!offer.TaskAccept) {
      return NO_MATCH;
    }
    
    oreqit->second.Bids[offer.RobotID] = offer;
    return CURRENT_MATCH;
  }

  return NEW_INFO;

  // PU LOOK AT TO DOUBLE CHECK
  //  else if (oreqit->Iteration < offer.Iteration) {
  //    return NEW_INFO;
  //  }
}



CNP_Award CNP_Server::Award_Task(int taskid) {
    CNP_Award award;
    CNP_Offer *offer;
    const bool DEBUG_AWARD_TASK = false;

    if (CNP_RANDOM) {
        offer = Find_RandomBidder(taskid);
    }
    else {
        offer = Find_MaxBidder(taskid);
    }
    if (offer == NULL) {
        if (DEBUG_AWARD_TASK)
        {
            fprintf(stderr, "CNP_Server::Award_Task(). Offer is NULL!\n");
        }
        // Assign -2 to indicate that no bidder could won
        // the task.
        award.TaskID = -2;
        award.Iteration = -2;
        award.RobotID = -2;

        Remove_Task(taskid);
        return award;
    }

    award.TaskID = taskid;
    award.Iteration = offer->Iteration;
    award.RobotID = offer->RobotID;

    if (DEBUG_AWARD_TASK)
    {
        fprintf(
            stderr,
            "CNP_Server::Award_Task(). Task %d awarded to Robot %d (Iteration %d)\n",
            award.TaskID,
            award.RobotID,
            award.Iteration);
    }

    MoveTo_Execution(award);
    return award;
}

CNP_Offer *CNP_Server::Find_RandomBidder(int taskid) {
  CNP_Offer *maxoffer = NULL;

  std::map<int, CNP_OpenRequest>::iterator oreqit;
  oreqit = Open_Contracts.find(taskid);
  if (oreqit == Open_Contracts.end()) {
      //printf("cant find task!\n");
    return NULL;
  }


  CNP_OpenRequest oreq = oreqit->second;
  std::map<int, CNP_Offer>::iterator ofit = oreq.Bids.begin();
  int bidsize = oreq.Bids.size();
  int rn = rand() % bidsize;
  for (int cnt = 0; cnt < rn; cnt++) {
      ofit++;
  }
   maxoffer = &(ofit->second);

  
  return maxoffer;
}



void CNP_Server::MoveTo_Execution(CNP_Award &award) {
 
  std::map<int, CNP_OpenRequest>::iterator oreqit = Open_Contracts.find(award.TaskID);

  CNP_ExecutingRequest exreq;
  exreq.TaskID = oreqit->second.TaskID;
  exreq.Iteration = oreqit->second.Iteration;
  exreq.TaskType = oreqit->second.TaskType;

  exreq.Requirements = oreqit->second.Requirements;
  exreq.Exclusions = oreqit->second.Exclusions;
  exreq.RobotID = award.RobotID;

  Executing_Contracts[award.TaskID] = exreq;
  Open_Contracts.erase(award.TaskID);
}

  

// return the offer of the maximum bidder for a task
CNP_Offer *CNP_Server::Find_MaxBidder(int taskid) {
  CNP_Offer *maxoffer = NULL;
  float maxbid = -1;

  std::map<int, CNP_OpenRequest>::iterator oreqit;
  oreqit = Open_Contracts.find(taskid);
  if (oreqit == Open_Contracts.end()) {
    return NULL;
  }
  
  CNP_OpenRequest oreq = oreqit->second;
  std::map<int, CNP_Offer>::iterator ofit = oreq.Bids.begin();
  while (ofit != oreq.Bids.end()) {
    if (ofit->second.Bid > maxbid) {
      maxbid = ofit->second.Bid;
      maxoffer = &(ofit->second);
    }

    ofit++;
  }
  return maxoffer;
}
  


// this cancel just removes the task from the execution list
void CNP_Server::Cancel_Task(int taskid) {
  Remove_Task(taskid);
}


// this cancel is equivilant to a reneg, it places the task in the reneg list for possible future injecttion
//   with updated constraints
void CNP_Server::Cancel_Task(CNP_Cancel *tcan, int tid) {
  CNP_ExecutingRequest exreq;
  CNP_Request oreq;
  
  std::map<int, CNP_ExecutingRequest>::iterator erit;
  
  erit = Executing_Contracts.find(tid);
  if (erit == Executing_Contracts.end()) {
    return;
  }

  exreq = erit->second;
  oreq.TaskID = tid;
  oreq.Iteration = exreq.Iteration;
  oreq.TaskType = exreq.TaskType;
  if (strcmp(tcan->Info, exreq.Requirements.c_str())) {
    oreq.Requirements = tcan->Info;
  }
  else {
    oreq.Requirements = exreq.Requirements;
  }

  oreq.Exclusions.push_back(tcan->RobotID); //exreq.Exclusions;

  Renegged_Contracts[tid] = oreq;

  Executing_Contracts.erase(tid);
}  
  



// handles a task completion message
void CNP_Server::Complete_Task(int taskid) {

  Remove_Task(taskid);
}



void CNP_Server::Remove_Task(int taskid) {
  
  Open_Contracts.erase(taskid);
  Executing_Contracts.erase(taskid);
  Renegged_Contracts.erase(taskid);
}



// return a list of task that that a robotid is eligable for bidding on
std::vector<CNP_OpenRequest> CNP_Server::Check_ForTask(int robotid) {
    std::vector<CNP_OpenRequest> oreqs;
    std::list<int>::iterator lit;

    std::map<int, CNP_OpenRequest>::iterator oreqit = Open_Contracts.begin();
    while (oreqit != Open_Contracts.end()) {
        lit = find(oreqit->second.Exclusions.begin(), oreqit->second.Exclusions.end(), robotid);
        if (lit == oreqit->second.Exclusions.end()) {
            oreqs.push_back(oreqit->second);
        }
        else {
            //oreqs.push_back(oreqit->second);  
            // exluding this robot
        }
        oreqit++;
    }

    //fprintf(stderr, "oreg size: %d\n", oreqs.size());
    return oreqs;
}



CNP_Award CNP_Server::Check_Expire(void) {
    static int update = 0;
    CNP_Award aw;
    aw.RobotID = -1;

    update++;
    if (update != 10) { 
        update = 0;
        std::map<int,CNP_OpenRequest>::iterator orit = Open_Contracts.begin();
        while (orit != Open_Contracts.end()) {
            orit->second.Expire--;
            if (orit->second.Expire <= 0) {

                return Award_Task(orit->first);
            }
            orit++;
        }
    }
    return aw;
}

/**********************************************************************
 * $Log: cnp_server.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.15  2006/05/06 04:26:29  endo
 * Bugs of TrackTask fixed.
 *
 * Revision 1.14  2006/05/02 04:12:58  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.13  2006/05/01 19:49:12  pulam
 * *** empty log message ***
 *
 * Revision 1.12  2006/04/28 22:50:42  pulam
 * Constraint checking update, Terrainmap disable for large maps, renegging overhaul
 *
 * Revision 1.11  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.10  2006/02/20 22:19:29  pulam
 * Bug fix involving broadcast of task completion
 *
 * Revision 1.9  2006/02/19 23:51:53  pulam
 * Changes made for experiments
 *
 * Revision 1.8  2006/01/30 02:37:03  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 **********************************************************************/
