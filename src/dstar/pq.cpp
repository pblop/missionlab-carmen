/**********************************************************************
 **                                                                  **
 **                        pq.cpp                                    **
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


/* $Id: pq.cpp,v 1.1.1.1 2008/07/14 16:44:18 endo Exp $ */


#include"pq.hpp"


Pqueue::Pqueue(unsigned vsize)
{
  size=0;
  storage=new pqnode[vsize]; 
  if(!storage) 
    {
      cout<<"Memory Panic\n"<<flush;exit(0);
    } 
  storage[0].prio= MIN_KEY;
}


bool equals(node* a, node* b) 
{
  return (a->x==b->x && a->y==b->y);
}


void Pqueue::insert(Element item, Key key) 
{
  pqnode p(item,key);
  unsigned i=++size;
  while(storage[i/2].prio>key) 
    {
      storage[i]=storage[i/2];
      i/=2;
    }
  storage[i]=p;
}

Element Pqueue::peek() 
{
  return size>0?storage[1].elem:0;
}

Element Pqueue::pop() 
{
    if(size<1)
    {
        return 0;
    }
    unsigned child;
    pqnode last=storage[size--],min=storage[1];
    int i;
    for(i = 1; (i*2) <= (int)size; i = child) 
    {
        child=i*2;
        if(child!=size)
        {
            if(storage[child+1].prio < storage[child].prio) 
            {
                child++;
            }
        }
        if(last.prio > storage[child].prio)
        {
            storage[i]=storage[child];
        }
        else 
        {
            break;
        }
    }
    storage[i]=last;
    return min.elem;
}

void Pqueue::percolate_all(Key (*keyfunc)(Element,Graph*),Graph* g) 
{
    int i;

    for(i = 1; i < (int)size; i++) 
    {
        percolate(storage[i].elem,keyfunc(storage[i].elem,g));
    }
}

void Pqueue::percolate(Element item, Key key) 
{
  int pos=-1;
  pqnode tmp(item,key);
  for(unsigned int i=1;i<=size;i++)
    {
      if(equals(item,storage[i].elem))
	{
	  pos=i;
          break;
	}
    }
  if(pos==-1) 
    {
      return;
    }
  if(key==storage[pos].prio) 
    {
      return;
    }
  else if(key<storage[pos].prio) 
    {
      storage[pos].prio=key;
      percolate_up(pos);
    }
  else
    {
      storage[pos].prio=key;
      percolate_down(pos);
    }
}

void Pqueue::percolate_up(unsigned i) 
{
  pqnode tmp=storage[i];
  while(storage[i/2].prio>tmp.prio) 
    {
      storage[i]=storage[i/2];
      i/=2;
    }
  storage[i]=tmp;
}

void Pqueue::percolate_down(unsigned i) 
{
  pqnode tmp=storage[i];
  unsigned child;
  for(;i*2<=size;i=child) 
    {
      child=i*2;
      if(child!=size)
	{
	  if(storage[child+1].prio<storage[child].prio)
	    {
	      child++;
	    }
	}
      if(storage[child].prio>=tmp.prio)
	{ 
	  break;
	}
      storage[i]=storage[child];
    }
  storage[i]=tmp;
} 

bool Pqueue::exists(const Element item) 
{
    int i;

    for(i = 1; i <= (int)size; i++) 
    {
        if(storage[i].elem==item)
        {
            return true;
        }
    }
    return false;
}

void Pqueue::remove(Element item) 
{
  int pos=-1;
  if(size==0)
    {
      return;
    }
  Key k=make_pair(MIN_KEY.first+1,MIN_KEY.second);
  for(unsigned int i=1;i<=size;i++)
    {
      if(equals(item,storage[i].elem))
	{
	  pos=i;
	}
    }
  if(pos==-1)
    {
      return;
    }
  storage[pos].prio=k;
  percolate_up(pos);
  pop();
}



/*#include<iostream>
int main() {
   Pqueue p;
   for(int i=0;i<10;i++) p.insert(i,Key(i,i));
   for(int i=0;i<10;i++) cout<<p.pop()<<" ";
   cout<<endl;
   for(int i=0;i<10;i++) p.insert(i,Key(i,i));
   //cout<<"percolates "<<flush;
//   for(int i=0;i<=4;i++)
   p.percolate(0,Key(10,0));
   for(int i=0;i<10;i++) cout<<p.pop()<<" ";
   for(int i=0;i<10;i++) p.insert(i,Key(i,i));
   for(int i=0;i<4;i++) p.remove(i);
   cout<<endl<<"removed";
   for(int i=0;i<6;i++) cout<<p.pop()<<" ";
}*/

/**********************************************************************
 * $Log: pq.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:18  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/15 02:43:33  endo
 * gcc-3.4 upgrade
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

