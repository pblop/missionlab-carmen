/**********************************************************************
 **                                                                  **
 **                        pq.hpp                                    **
 **                                                                  **
 **                                                                  **
 **  Written by:  Ananth Ranganathan                                 **
 **                                                                  ** 
 **  The D* Lite algorithm was developed by Sven Koenig and Maxim    ** 
 **  Likhachev.                                                      **
 **                                                                  **
 **  This code adheres closely to the D*Lite pseudo-code in the      **
 **  paper:                                                          **
 **                                                                  **
 **  S. Koenig and M. Likhachev                                      **
 **  "Improved Fast Replanning for Robot Navigation in Unknown       **
 **  Terrain", ICRA-2002.                                            **
 **                                                                  **
 **  Copyright 2003, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: pq.hpp,v 1.1.1.1 2008/07/14 16:44:18 endo Exp $ */


#ifndef PQUEUE_H
#define PQUEUE_H

#include"graph.hpp"
#include<climits>

//using namespace std;

const Key MIN_KEY=Key(INT_MIN,INT_MIN);

class Dstar;

struct pqnode
{

  Element elem;
  Key prio;
  pqnode() : elem(0),prio(MIN_KEY) {}
  pqnode(Element e, Key k) : elem(e),prio(k) {}
  bool operator==(const pqnode& p) 
  { return elem==p.elem && prio==p.prio; }

};  

class Pqueue 
{

  const static unsigned DEF_SIZE=10099;
  unsigned size;
  pqnode* storage;

  void percolate_up(unsigned i);
  void percolate_down(unsigned i);

public:

  Pqueue(unsigned vsize=DEF_SIZE);
  ~Pqueue() { delete [] storage; }
  void insert(Element item, Key key);
  Element peek();
  Element pop();
  void percolate(Element item, Key key);
  void percolate_all(Key (*keyfunc)(Element,Graph*),Graph*);
  bool exists(const Element item);
  Key minkey() { return storage[1].prio; }
  void remove(Element item);
  bool empty() { return size<1; }
  int getSize() { return size; }

};
  
#endif

/**********************************************************************
 * $Log: pq.hpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:18  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:25  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2003/04/06 15:00:50  endo
 * gcc 3.1.1
 *
 * Revision 1.1  2003/04/06 07:23:34  endo
 * Initial revision
 *
 **********************************************************************/
