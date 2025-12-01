///////////////////////////////////////////////////////////////////////////////
//
//                                 g++.cc
//
// Implements functions that g++ needs for VxWorks
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#if defined(__GNUC__)

#include <stdlib.h>

#ifdef VXWORKS
#include <ioLib.h>
#else
#include <osfcn.h>
#endif

#define MEM_MESSAGE "Virtual memory exceeded in `new'\n"

extern "C" {

void
__default_handler ()
{
  /* don't use fprintf (stderr, ...) because it may need to call malloc.  */
  /* This should really print the name of the program, but that is hard to
     do.  We need a standard, clean way to get at the name.  */
  write (2, MEM_MESSAGE, sizeof (MEM_MESSAGE));
  /* don't call exit () because that may call global destructors which
     may cause a loop.  */
  exit (-1);
}

/* operator new (size_t), described in 17.3.3.5.  This function is used by
   C++ programs to allocate a block of memory to hold a single object. */

void *
__builtin_new (size_t sz)
{
  void *p;

  /* malloc (0) is unpredictable; avoid it.  */
  if (sz == 0)
    sz = 1;
  p = (void *) malloc (sz);
  if (p==0)
      __default_handler ();
  
  return p;
}

/* operator delete (void *), described in 17.3.3.3.  This function is used
   by C++ programs to return to the free store a block of memory allocated
   as a single object. */

void
__builtin_delete (void *ptr)
{
  if (ptr)
    free (ptr);
}

/* operator delete [] (void *), described in 17.3.3.4.  This function is
   used by C++ programs to return to the free store a block of memory
   allocated as an array. */

extern void __builtin_delete (void *);

void
__builtin_vec_delete (void *ptr)
{
  __builtin_delete (ptr);
}

/* void * operator new [] (size_t), described in 17.3.3.6.  This function
   is used by C++ programs to allocate a block of memory for an array.  */

void *
__builtin_vec_new (size_t sz)
{
  return __builtin_new (sz);
}

#define PURE_MESSAGE "pure virtual method called\n"
void
__pure_virtual ()
{
  write (2, PURE_MESSAGE, sizeof (PURE_MESSAGE) - 1);
  exit (-1);
}

}

#endif
