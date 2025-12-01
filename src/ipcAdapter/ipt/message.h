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

typedef const char IPMessageType;

class IPCommunicator;
class IPConnection;

#include <ipc.h>

class IPMessage {
  public:
    ~IPMessage()
    {
    	IPC_freeByteArray(byteArray);
    	IPC_freeData(formatter, callData);
    	callData = NULL;
    	formatter = NULL;
    	msgInstance = NULL;
    }

	IPMessage(BYTE_ARRAY rawData, void *data, FORMATTER_PTR fmt, MSG_INSTANCE instance)
	{
		msgInstance = instance;
		formatter = fmt;
		callData = data;
		byteArray = rawData;
	}

	IPMessage(BYTE_ARRAY rawData, void *data, FORMATTER_PTR fmt)
	{
		msgInstance = NULL;
		formatter = fmt;
		callData = data;
		byteArray = rawData;
	}

    void* FormattedData(int force_copy = 0)
    {
		void *dataCopy;

    	if(!force_copy)
    		return callData;

		IPC_unmarshall(formatter, byteArray,(void **)&dataCopy);
    	return dataCopy;
    }

    MSG_INSTANCE getMsgInstance()
    {
    	return msgInstance;
    }

    int SizeData() const
    {
    	if(msgInstance)
    		return IPC_dataLength(msgInstance);
    	return 0;
    }

  private:
    MSG_INSTANCE msgInstance;
    BYTE_ARRAY byteArray;
    void *callData;
    FORMATTER_PTR formatter;
};

#endif
