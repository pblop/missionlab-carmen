/**********************************************************************
 **                                                                  **
 **                              sllist.cc                           **
 **                                                                  **
 **  Written by:  Patrick Daniel Ulam                                **
 **                                                                  **
 **  Base implementation for a singlely linked list class            **
 **                                                                  **
 **  Copyright 2005 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: sllist.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#include <assert.h>
#include <stdio.h>
#include <fstream>
#include "sllist.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor for the empty list
//   Precondition: None
//   Postcondition: SSList with no nodes is created
//   Time = constant
////////////////////////////////////////////////////////////////////////////
SLList::SLList(void) {
  ListSize = 0;
  CurListPos = -1;
  Head = Rear = NextNode = PrevNode = CurNode = NULL; 
}

////////////////////////////////////////////////////////////////////////////
// Constructor for the list given a initial node
//	  Precondition: newdata contains the data for the initial node
//   Postconition: SLList with 1 node is created
//   Time = constant
///////////////////////////////////////////////////////////////////////////
SLList::SLList(void *newdata) {
	Insert_First(newdata);
}

//////////////////////////////////////////////////////////////////////////
// deconstructor... make sure we are deleting an empty list
//   Time = constant
//////////////////////////////////////////////////////////////////////////
SLList::~SLList(void) {
	for (int cnt = 0; cnt < ListSize; cnt++) {
		Delete_Head();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Reset_List: Resets a lists postion due to a traversal
//   Precondition: None
//   Postcondition: List position is reset to 0 if there is at least one node
//      or -1 otherwise
//   Time = constant
/////////////////////////////////////////////////////////////////////////////
void SLList::Reset_List(void) {
  CurNode = Head;

  // check the case that we are reseting the empty list
  if (ListSize == 0) {
    NextNode = NULL;
    CurListPos = -1;
  }
  else {
    NextNode = CurNode->NextNode;
    CurListPos = 0;
  }
  PrevNode = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// Insert_Head: creates a node using newdata and places it as the new head of
//    the list
//   Time = constant
////////////////////////////////////////////////////////////////////////////
void SLList::Insert_Head(void *newdata) {

	// check the case that this is the first element being inserted
  if (ListSize == 0) {
    Insert_First(newdata);
  }

  // otherwise we handle it normally 
  else {
	SLLNode *tempnode = new SLLNode;
	tempnode->Data = newdata;
	tempnode->NextNode = Head;
    Head = tempnode;
    ListSize++;
	 
	 // handle the case where curnode = head
	 if (CurListPos == 0) {
		PrevNode = Head;
		NextNode = CurNode->NextNode;
    }
	 
	 // and increment CurListPos as a node it is one position to the right of
	 //   where is used to be
	 CurListPos++;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Insert_Rear: creates a node using newdata and places it as the new rear of
//    the list
//   Time = constant
/////////////////////////////////////////////////////////////////////////////
void SLList::Insert_Rear(void *newdata) {
  
  // handle 1st element case
  if (ListSize == 0) {
    Insert_First(newdata);
  }
  else {
	  SLLNode *tempnode = new SLLNode;
	  tempnode->Data = newdata;
	  tempnode->NextNode = NULL;
	Rear->NextNode = tempnode;
    Rear = tempnode;
    ListSize++;

	 // handle the case that CurNode was at the rear when we entered method
    if (CurListPos == (ListSize - 2)) {
	   NextNode = Rear;
	}
  }
}

/////////////////////////////////////////////////////////////////////////////
// Insert_CurrentR: creates a node using newdata and places it as the node
//		immediatly following that of CurNode
//    Time = Constant
/////////////////////////////////////////////////////////////////////////////
void SLList::Insert_CurrentR(void *newdata) {
  
  // handle 1st element case
  if (ListSize == 0) {
    Insert_First(newdata);	 
  }
  
  else {
	SLLNode *tempnode = new SLLNode;
	tempnode->Data = newdata;
	tempnode->NextNode = NextNode;
	CurNode->NextNode = tempnode;
    NextNode = tempnode;
	 
	 // handle the case that CurNode is at the rear
	 if (CurNode == Rear) Rear = tempnode;
	 ListSize++;
  }
}

// Insert a node immediatly preceeding CurNode
void SLList::Insert_CurrentF(void *newdata) {

	// handle 1st element case
	if (ListSize == 0) {
	  Insert_First(newdata);
	}

	// handle the case the current node is at the head
	else if (PrevNode == NULL) {
		Insert_Head(newdata);
		CurListPos = 1;
	}

	// handle all other cases
	else {
		SLLNode *tempnode = new SLLNode;
		tempnode->Data = newdata;
		tempnode->NextNode = CurNode;
		PrevNode->NextNode = tempnode;
		CurListPos++;
	}
}

/////////////////////////////////////////////////////////////////////////
// Insert_First: inserts the first node into the empty list
//    Time = constant
////////////////////////////////////////////////////////////////////////
void SLList::Insert_First(void *newdata) {
  CurNode = new SLLNode;
  CurNode->Data = newdata; 
  CurNode->NextNode = NULL;
  Head = Rear = CurNode;
  PrevNode = NextNode = NULL;
  CurListPos = 0;
  ListSize = 1;
}

////////////////////////////////////////////////////////////////////////////
// Delete_Head: removes the head of the list and returns it's data for
//	    appropriate handling
//   PostCondition: list returned is reset
//   Time = constant
////////////////////////////////////////////////////////////////////////////
void *SLList::Delete_Head(void) {
  
  // handle  empty list
  if (ListSize == 0) return NULL;
  void *tempdata = Head->Data;

  // handle the case of more then 1 element
  if (ListSize != 1) {
	SLLNode *tempnode = Head->NextNode;
    delete Head;
    Head = tempnode;
    ListSize--;

	 // handle the case where we delete the node we are on
	 if (CurListPos == 0) {
		NextNode = Head->NextNode;
		CurNode = Head;		
    }
    else {
	   CurListPos--;
	 }
  }

  // handle the case of one element
  else {
    delete Head;
    ListSize--;
	CurListPos = -1;
	Head = Rear = NextNode = PrevNode = CurNode = NULL;
  }
  Reset_List();
  return tempdata;
}

//////////////////////////////////////////////////////////////////////////////
// Delete_Rear: removes the rear of the list and returns it's data for
//     appropriate handling   
//   PostCondition: list returns reset (ie CurListPos = 0)
//   Time = O(n) where n = size of list
/////////////////////////////////////////////////////////////////////////////
void *SLList::Delete_Rear(void) {
if (ListSize == 0) return NULL;

  void *tempdata = Rear->Data;
  Reset_List();
  
  // go to the last position in the list
  while (Next_Node());

  // do this stuff if there is more then one element in the list
  if (PrevNode != NULL) {
	 PrevNode->NextNode= NULL;
  	 delete Rear;
    Rear = PrevNode;
  }
  ListSize--;
  Reset_List();
  return tempdata;
}

////////////////////////////////////////////////////////////////////////////
// Delete_Current: deletes the node at CurNode and returns it's data for
//		appropriate handling
//   PostCondition:  List returned is reset
//   Time = n where n is the size of the list
////////////////////////////////////////////////////////////////////////////
void *SLList::Delete_Current(void) {
  
  // handle case where there is an empty list
  if (ListSize == 0) {
    return NULL;
  }

  // handle deletion from front of list
  if (CurListPos == 0) {
    return Delete_Head();
  }

  // handle deletion from rear of list
  if (CurListPos == ListSize - 1) {
    return Delete_Rear();
  }
  void *tempdata = CurNode->Data;
  PrevNode->NextNode = CurNode->NextNode;
  delete CurNode;
  ListSize--;
  Reset_List();
  return tempdata;
}

/////////////////////////////////////////////////////////////////////////////
// Get_Data: returns the data contained in CurNode
/////////////////////////////////////////////////////////////////////////////
void *SLList::Get_Data(void) {
  if (ListSize == 0) return NULL;
  return CurNode->Data;
}

/////////////////////////////////////////////////////////////////////////////
// Next_Node: Traverses the list by one node
/////////////////////////////////////////////////////////////////////////////
bool SLList::Next_Node(void) {
  if (NextNode == NULL) return false;
  PrevNode = CurNode;
  CurNode = NextNode;
  NextNode = CurNode->NextNode;
  CurListPos++;
  return true;
}

/////////////////////////////////////////////////////////////////////////////
// Goto_Node: Moves curListPos to node X
//   returns FALSE if not posible
//   time: n where n is the size of the list
////////////////////////////////////////////////////////////////////////////
bool SLList::Goto_Node(int seekpos) {

  // check for valid positions
  if ((seekpos < 0) || (seekpos > ListSize - 1)) return false;
  
  // check to see if we have to back up
  if (seekpos < CurListPos) Reset_List();

  // look through the list until we reach proper position
  while (CurListPos != seekpos) Next_Node();
  return true;
} 
 
/////////////////////////////////////////////////////////////////////////////
// Get_ListSize: returns the list size
/////////////////////////////////////////////////////////////////////////////
int SLList::Get_ListSize(void) {
  return ListSize;
}



// this adds the elements from a list into another list
void SLList::Insert_List(SLList *mlist) {
	int listsize,cnt;
	listsize = mlist->Get_ListSize();
	mlist->Reset_List();


	for (cnt = 0; cnt < listsize; cnt++) {
		Insert_Head(mlist->Get_Data());
		mlist->Next_Node();
	}
}



                                                                                
void SLList::Append_ListIntact(SLList *newlist) {
                                                                                
        newlist->Reset_List();
        for (int cnt = 0; cnt < newlist->Get_ListSize(); cnt++) {
                Insert_Rear(newlist->Get_Data());
                newlist->Next_Node();
        }
}

/**********************************************************************
 * $Log: sllist.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/12 22:49:05  endo
 * g++-3.4 upgrade.
 *
 * Revision 1.1  2005/06/21 18:15:00  endo
 * sllist moved to utility_function.
 *
 *
 **********************************************************************/
