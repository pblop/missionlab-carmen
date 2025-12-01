//////////////////////////////////////////////////////////////////////////////
//
//                                 unixcommunicator.h
//
// This file declares the classe needed for UNIX based IPT communicators
//
// Classes defined for export:
//    UnixCommunicator
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef unixcommunicator_h
#define unixcommunicator_h

#include <ipt/tcpcommunicator.h>

/* This is the class to use with a standard Unix module */
class UnixCommunicator : public TCPCommunicator {
  public:
    UnixCommunicator(const char* mod_name, const char* host_name = 0);
    UnixCommunicator(int);
    virtual ~UnixCommunicator();

  private:
    void initialize_unix();

  private:
    const char* _user_id;
};

#endif
