/******************************************************************************
*
*                                 tcx_cover.h
*
* This header file fakes some things for covering TCX that need to be included
* both by tcx.h (i.e., by the client's C program) and by the C++ cover 
* functions
*
* Structures faked by typedef'ed void
*    TCX_MODULE_PTR, TCX_REF_PTR, TCX_FMT_PTR, MSG_DEF_PTR
*
* Structures actually defined
*    TCX_REG_MSG_TYPE, TCX_REG_HND_TYPE
*
*  "1995, Carnegie Mellon University. All Rights Reserved." This
*  software is made available for academic and research purposes only. No
*  commercial license is hereby granted.  Copying and other reproduction is
*  authorized only for research, education, and other non-commercial
*  purposes.  No warranties, either expressed or implied, are made
*  regarding the operation, use, or results of the software.
*
*****************************************************************************/

#ifndef ip_tcx_cover_h
#define ip_tcx_cover_h

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define TCX_RECV_ALL  1
#define TCX_RECV_NONE 2
#define TCX_RECV_ONCE 3

#define PRINT_MODULE  0x0001
#define PRINT_MESSAGE 0x0002
#define PRINT_INSTANCE 0x0004
#define PRINT_LINEFEED  0x0008

#define NO_HND       0x0000
#define TCX_HND      0x0001
#define USER_HND     0x0002
#define ALL_HND      (USER_HND | TCX_HND)

#define TCX_STORE_ALL 0
#define TCX_STORE_LATEST 1
#define TCX_STORE_LATEST_EACH_MODULE 2

#ifdef P_
#undef P_
#endif

#if defined(__STDC__) || defined(__cplusplus)
# define P_(s) s
#else
# define P_(s) ()
#endif

typedef void *TCX_MODULE_PTR;
typedef void *TCX_REF_PTR;
typedef void *TCX_FMT_PTR;
typedef void *MSG_DEF_PTR;

typedef struct {
  char *msgName;
  char *msgFormat;
} TCX_REG_MSG_TYPE;

typedef struct {
  char *msgName;
  char *hndName;
  void (*hndProc) P_((TCX_REF_PTR, void*, void*));
  int hndControl;
  void *hndData;
} TCX_REG_HND_TYPE;

#endif
