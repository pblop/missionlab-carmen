///////////////////////////////////////////////////////////////////////////////
//
//                                 message.h
//
// This file declares the class that encapsulates the basis for communication
// in IPT: the message class.  Messages have associated with them a message 
// type, a message instance number, and a known number of bytes of data.
// Messages that come from other modules have associated with them which
// connection they came over from.  The message type of a message can be
// used to generate formatted data that can be put into a structure and
// managed reasonably.
//
// Classes defined for export:
//    IPMessage
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_message_h
#define ip_message_h

class IPMessageType;
class IPCommunicator;
class IPConnection;

/* WARNING:  If you change any of the member variables of IPMessage, you _must_
   change IPMESSAGE_SIZE in ipt_comm.h and tcxP.h to reflect the new 
   sizeof(IPMessage) (currently 32).  If you do not do this you may introduce
   nearly untraceable bugs into systems using the TCX and IPT C cover
   functions. */
class IPMessage {
  public:
    IPMessage(IPMessageType*, int, int, unsigned char* = 0);
    IPMessage(int, int, int, unsigned char* = 0);
    IPMessage(IPMessageType*, int, void*);
    IPMessage(const IPMessage&);
    ~IPMessage();

    /* Data is stored primarily unformatted, and can be gotten through these
       methods */
    void Data(int& size, unsigned char*& data) const
        { size = _size; data = _data; }
    int SizeData() const { return _size; }
    unsigned char* Data() const { return _data; }

    int Instance() const { return _instance; }
    int ID() const;

    /* Data can be gotted in formatted form here */
    void* FormattedData(int force_copy = 0);
    void FormattedData(void*, int force_copy = 0);
    void DeleteFormatted(void*);
    void DeleteContents(void*);

    IPMessageType* Type() const { return _type; }
    IPMessageType* EncodeType(IPCommunicator*);

    void Connection(IPConnection* c) { _connection = c; }
    IPConnection* Connection() const { return _connection; }

    /* Ways to set other pieces of information */
    unsigned char* SetData(int size, unsigned char* data = 0);
    void SetInstance(int instance) { _instance = instance; }
    void SetType(IPMessageType* type) { _type = type; }

    int Print(int = 0);

    void* formatted_data_store() const { return _formatted_data; }
    int formatted_contents() const { return (int) _formatted_contents; }

  private:
    /* WARNING:  If you add, delete, or change any of these, you _must_
       change IPMESSAGE_SIZE in ipt.h and tcxP.h to reflect the new 
       sizeof(IPMessage) (currently 32).  If you do not do this you may
       introduce nearly untraceable bugs into systems using the TCX and IPT C
       cover functions. */
    int _size;
    int _instance;
    int _id;
    unsigned char* _data;
    IPMessageType* _type;
    short _allocated;
    short _formatted_contents;
    void* _formatted_data;
    IPConnection* _connection;
};

#endif
