///////////////////////////////////////////////////////////////////////////////
//
//                                 destination.h
//
// This header file defines classes used to define destinations for messages.
// Destinations are associated the IPMessageType, i.e. each type has one.
// Whenever a IPMessage is received, it is sent to the IPDestination declared 
// by its message type.  The IPDestination then stores the message whereever
// and however it deems appropriate.  This mechanism makes things like "pigeon
// holed" messages possible, since there can be one mechanism for most
// messages, but a special mechanism for pigeon holed messages just by changing
// classes of a message type's destination
//
// Classes defined for export:
//    IPDestination, DefaultDestination, PigeonHoleDestination
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_destination_h
#define ip_destination_h

class IPConnection;
class IPMessage;
class IPMessageType;

/* Base class for message destinations */
class IPDestination {
  public:
    virtual int ProcessMessage(IPConnection*, IPMessage*) = 0;
    virtual IPMessage* MakeMessage(IPConnection*, IPMessageType*,
                                   int, int);
};

/* Default destination for non-pigeon holed messages */
class DefaultDestination : public IPDestination {
  public:
    virtual int ProcessMessage(IPConnection*, IPMessage*);
};

/* Destination for pigeon holed messages */
class PigeonHoleDestination : public IPDestination {
  public:
    virtual int ProcessMessage(IPConnection*, IPMessage*);
    virtual IPMessage* MakeMessage(IPConnection*, IPMessageType*,
                                   int, int);
};

class FixedDestination : public DefaultDestination {
  public:
    FixedDestination(int, unsigned char*);

    virtual IPMessage* MakeMessage(IPConnection*, IPMessageType*,
                                   int, int);

  private:
    int _max_size;
    unsigned char* _default_data;
};

#endif
