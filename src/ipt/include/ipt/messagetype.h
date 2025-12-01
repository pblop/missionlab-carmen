///////////////////////////////////////////////////////////////////////////////
//
//                                 messagetype.h
//
// This header file defines the class that holds a message's type information, 
// i.e., whether (and how) it is handled, whether (and how) it is formatted,
// what its destination is, what its type name is, etc.
//
// Classes defined for export:
//    IPMessageType
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_messagetype_h
#define ip_messagetype_h

class IPFormat;
class IPHandlerCallback;
class IPDestination;
class IPList;
class IPMessage;
class IPConnection;

/* This class contains the information and methods for specifying and
   formatting message data */
class IPMessageType {
  public:
    IPMessageType(const char* name, IPFormat*, int id);
    ~IPMessageType();

    /* return the name */
    const char* Name() const { return _name; }

    /* return the ID */
    int ID() const { return _id; }

    IPFormat* Formatter() const { return _format; }
    void SetFormat(IPFormat*);

    /* status methods for if a message is pigeon holed on incoming */
    IPDestination* Destination() const { return _destination; }
    void Destination(IPDestination* d) { _destination = d; }

    /* deal with formatted data */
    void* FormatData(unsigned char* input, int size_input = 0,
                     IPConnection* conn = NULL);
    void FormatData(unsigned char* input, int size_input, void* output,
                    IPConnection* conn = NULL);
    int UnformatData(void*, int& output_size, unsigned char*& output);
    int CachedUnformatData(void*, int& output_size, unsigned char*& output);
    void RawUnformatData(void*, unsigned char* output);
    void DeleteFormattedData(void* data, unsigned char*, int);
    void DeleteContents(void* data, unsigned char*, int);

    void Handler(IPHandlerCallback* callback, int context);
    IPHandlerCallback* Handler() const { return _callback; }
    int HandlerContext() const { return _handler_context; }

    int HandlerActive() const { return _handler_active; }
    void DisableHandler() { _handler_active = 0; }
    void EnableHandler() { _handler_active = 1; }
    int HandlerInvoked() const { return _handler_invoked; }
    void HandlerInvoked(int i) { _handler_invoked = i; }

    IPMessage* Postponed();
    void Postpone(IPMessage*);

    unsigned char* Cache(int num_bytes);
    void ReleaseCache(unsigned char*);
    int GetCache(int& num_bytes, unsigned char*&);

  private:
    char* _name;                   // name of the message type
    int _id;                       // ID of message type 
    IPDestination* _destination;   // destination for message of this type
    IPFormat* _format;             // message type format, NULL means none
    IPHandlerCallback* _callback;  // callback for message type, ditto
    int _handler_context;          // context in which handler is used
                                   // (ignored for now)
    int _handler_active;           // false, handling of messages is disabled
    int _handler_invoked;          // true, callback is current being exec'ed
    IPList* _postponed_messages;   // used in postponing handling of a message
    int _size_cache;
    unsigned char* _cache;
    int _cache_in_use;
};

#endif
