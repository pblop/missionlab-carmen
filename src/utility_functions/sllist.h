/**********************************************************************
 **                                                                  **
 **                              sllist.h                            **
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

/* $Id: sllist.h,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#ifndef SLLIST_H
#define SLLIST_H

struct SLLNode {
	void *Data;
	SLLNode *NextNode;
};


class SLList {
public:
  SLList(void);
  SLList(void *newdata);
  ~SLList(void);
  void Reset_List(void);
  void Insert_Head(void *newdata);
  void Insert_Rear(void *newdata);
  void Insert_CurrentR(void *newdata);
  void Insert_CurrentF(void *newdata);
  void Insert_List(SLList *mlist);
  void *Delete_Head(void);
  void *Delete_Rear(void);
  void *Delete_Current(void);
  void *Get_Data(void);
  bool Next_Node(void);
  int Get_ListSize(void);
  bool Goto_Node(int seekpos);
  void Append_ListIntact(SLList *newlist);

protected:
  void Insert_First(void *newdata);
  SLLNode *Head;
  SLLNode *Rear;
  SLLNode *NextNode;
  SLLNode *PrevNode;
  SLLNode *CurNode;
  int ListSize;
  int CurListPos;
};


#endif

/**********************************************************************
 * $Log: sllist.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2005/06/21 18:15:00  endo
 * sllist moved to utility_function.
 *
 *
 **********************************************************************/
