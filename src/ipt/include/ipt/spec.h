/******************************************************************************
 *
 *                                 spec.h
 *
 * This file declares the convenience structures used for specifying message
 * types and routine handlers.  It's ugly because it works for both C and C++
 *
 * Structures defined for export:
 *    IPMessageSpec, IPMsgHandlerSpec, IPHandlerSpec
 *
 *  "1995, Carnegie Mellon University. All Rights Reserved." This
 *  software is made available for academic and research purposes only. No
 *  commercial license is hereby granted.  Copying and other reproduction is
 *  authorized only for research, education, and other non-commercial
 *  purposes.  No warranties, either expressed or implied, are made
 *  regarding the operation, use, or results of the software.
 *
 *****************************************************************************/

#ifndef ip_spec_h
#define ip_spec_h

#ifndef P_

#if defined(__STDC__) || defined(__cplusplus)
# define P_(s) s
#else
# define P_(s) ()
#endif

#endif

/* message specifier structure.  Used with IPCommunicator::RegisterMessages */
#if defined(__cplusplus)
struct IPMessageSpec 
#else
typedef struct
#endif
{
    char* name;
    char* format;
#if defined(__cplusplus)
};
#else
} IPMessageSpec;
#endif

/* message specifier structure.  Used with IPCommunicator::RegisterMessages */
#if defined(__cplusplus)
struct IPFormatSpec 
#else
typedef struct
#endif
{
    char* name;
    char* format;
#if defined(__cplusplus)
};
#else
} IPFormatSpec;
#endif

/* handler specifier structure.  Provided for users convenience */
#if defined(__cplusplus)
struct IPMsgHandlerSpec 
#else
typedef struct 
#endif
{
    char* msg_name;
    void (*callback) P_((IPCommunicator*, IPMessage*, void*));
    int context;
    void* data;
#if defined(__cplusplus)
};
#else
} IPMsgHandlerSpec;
#endif

#if defined(__cplusplus)
struct IPHandlerSpec 
#else
typedef struct 
#endif
{
    char* msg_name;
    void (*callback) P_((IPMessage*, void*));
    int context;
    void* data;
#if defined(__cplusplus)
};
#else
} IPHandlerSpec;
#endif

#endif
