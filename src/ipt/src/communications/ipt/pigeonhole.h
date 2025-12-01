///////////////////////////////////////////////////////////////////////////////
//
//                                 pigeonhole.h
//
// This header file defines classes that implement the pigeonholes for messages
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

#ifndef pigeonhole_h
#define pigeonhole_h

class IPMessage;
class IPMessageType;

/* Pigeonhole class for messages */
class IPPigeonHole {
  public:
    IPPigeonHole(IPMessageType*);
    ~IPPigeonHole();

    int Filled() const { return _filled != 0; }
    IPMessageType* Type() const { return _type; }
    int HandlerActive() const { return _handler_active; }
    void HandlerActive(int h) { _handler_active = h; }

    /* status for if message is enqueued on a message list */
    int Enqueued() const { return _enqueued; }
    void Enqueue() { _enqueued = 1; }
    void Dequeue() { _enqueued = 0; }

    void Fill(IPMessage* msg);
    IPMessage* Empty();
    IPMessage* Contents() const {  return _message; }
    void Set(IPMessage* msg);

  private:
    int _enqueued;
    IPMessage* _message;
    IPMessageType* _type;
    int _handler_active;
    int _filled;
};

#endif
