#ifndef CNP_SERVER_H
#define CNP_SERVER_H

#include "cnp_contracts.h"
#include <vector>
#include <list>
//#include "cnp_task_constraint.h"

class CNP_Server {
 public:
  CNP_Server(void);
  ~CNP_Server(void);
  bool Inject_Task(CNP_Request &request);

  void Receive_Offer(CNP_Offer &offer);
  void Cancel_Task(int taskid);
  void Complete_Task(int taskid);
  void Cancel_Task(CNP_Cancel *tcan, int tid);
  std::vector<CNP_OpenRequest> Check_ForTask(int robotid);
  CNP_Award Check_Expire(void);
  CNP_Request *Is_ReneggedTask(int tid);
  void Remove_Task(int taskid);  
  std::list<CNP_TaskConstraint> Get_TaskConstraints(int tid);

 private:
  CNP_Award Award_Task(int taskid);
  bool Is_DuplicateTask(CNP_Request &request);



  void Send_Request(CNP_OpenRequest *request);
  CNP_OfferType Match_TaskOffer(CNP_Offer &offer);

  void Check_Rebid(CNP_OpenRequest *req, CNP_Offer *offer);
  CNP_OpenRequest *Find_OpenTask(int taskid);
  CNP_Offer *Find_MaxBidder(int taskid); 
  CNP_Offer *Find_RandomBidder(int taskid); 

  CNP_ExecutingRequest *Remove_Executing(int taskid);
  void MoveTo_Execution(CNP_Award &award);
  void Send_Cancel(CNP_Remove *scanc);
 

  std::map<int, CNP_OpenRequest> Open_Contracts;
  std::map<int, CNP_ExecutingRequest> Executing_Contracts;
  std::map<int, CNP_Request> Renegged_Contracts;
   
};


#endif
