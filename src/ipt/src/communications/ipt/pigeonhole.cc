///////////////////////////////////////////////////////////////////////////////
//
//                                 pigeonhole.cc
//
// This file implements classes that implement the pigeonholes for messages
// Pigeon holes are "boxes" in which exactly one message can fit.  By 
// pigeon holing a message type, you can be assured of getting the most recent
// message without any additional list manipulation because whenever a new
// message comes in it "knocks out" the old one.
//
// Classes defined for export:
//    IPPigeonHole
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include <ipt/message.h>
#include <ipt/pigeonhole.h>

/* Create a pigeon hole for a message type */
IPPigeonHole::IPPigeonHole(IPMessageType* t)
{
    _message = 0;
    _type = t;
    _handler_active = 0;
    _enqueued = 0;
    _filled = 0;
}

/* Delete a pigeon hole */
IPPigeonHole::~IPPigeonHole()
{
    if (_message)
        delete _message;
}

/* Fill a pigeon hole with a message.  If "msg" is actually the same things
   as the contents message of the pigeon hole, just mark the hole as filled,
   otherwise delete the contents message and replace it with "msg" and mark
   the hole as filled */
void IPPigeonHole::Fill(IPMessage* msg)
{
    if (msg != _message) {
        if (_message)
            delete _message;

        _message = msg;
    }

    _filled = 1;
}

/* Return the contents message and mark the hole as filled */
IPMessage* IPPigeonHole::Empty()
{
    if (!_filled)
        return 0;

    _filled = 0;

    return new IPMessage(*_message);
}

/* Set the pigeon hole message to "msg" without affecting the filled state */
void IPPigeonHole::Set(IPMessage* msg)
{
    if (msg != _message) {
        if (_message)
            delete _message;

        _message = msg;
    }
}
