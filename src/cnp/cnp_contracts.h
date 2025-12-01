#ifndef CNP_CONTRACTS_H
#define CNP_CONTRACTS_H

#include "sllist.h"
#include <map>
#include <list>
#include <string>


// type of offer made for an auction
// NO_MATCH: The offer does not match a current auction
// CURRENT_MATCH: The offer matches a current auction
// NEW_INFO: The offer matches a current auction and includes additional information for iterative bidding
enum CNP_OfferType {
  NO_MATCH,
  CURRENT_MATCH,
  NEW_INFO
};


struct CNP_Request {
  int TaskID;                      // ID of the task
  int Iteration;                   // iteration used in iterative bidding
  std::string TaskType;                  // type of task (EOD, etc)
  std::string Requirements;               // Constraints on the task
  std::list<int> Exclusions;       // robots excluded from bidding
};
/*
struct CNP_Constraint {
  int RobotID;
  int Iteration;
  char *ConstraintType;
  char *Requirements;
  SLList *Exclusions;
};
*/

struct CNP_Offer {
  int TaskID;
  int Iteration;
  int RobotID;
  bool TaskAccept;
  float Bid;
};


struct CNP_OpenRequest {
  int TaskID;
  int Iteration;
  std::string TaskType;
  int Expire;          
  std::string Requirements;
  std::list<int> Exclusions;
  std::map<int, CNP_Offer> Bids;
};

struct CNP_ExecutingRequest {
  int TaskID;
  int Iteration;
  std::string TaskType;
  std::string Requirements;
  std::list<int> Exclusions;
  int RobotID;
};
  




struct CNP_Award {
  int TaskID;
  int Iteration;
  int RobotID;
};

struct CNP_Update {
  int TaskID;
  int Iteration;
  char *TaskType;
  SLList *Requirements;
  SLList *Exclusions;  
};

struct CNP_Cancel {
  int TaskID;
  int Iteration;
  int RobotID;
  bool Consensual;
  char *Info;
};

struct CNP_Remove {
  int TaskID;
  //  int Iteration;
};

struct CNP_Complete {
  int TaskID;
  int Iteration;
};


#endif
