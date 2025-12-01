///////////////////////////////////////////////////////////////////////////////
//
//                                 callbacks.h
//
// This header file defines classes uses for callbacks for events such as
// modules connection/disconnection and incoming messages
//
// Classes defined for export:
//    IPConnectionCallback, IPRoutineConnection,
//    IPHandlerCallback, IPRoutineHandlerCallback
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_callbacks_h
#define ip_callbacks_h

class IPCommunicator;
class IPTimer;

#include "ipt/connection.h"
#include "ipt/message.h"
#include <pthread.h>
#include <ipc.h>

// Base class for calling back in response to a module connection/disconnection
class IPConnectionCallback// : public IPResource {
{
public:
    virtual void Execute(IPConnection*) = 0;
};

// Convenience routine for calling back a function rather than another subclass
// of IPConnectionCallback.  The routine should be of the form
//   void func(IPConnection* conn, void* data)
// where conn is the connection in question and data is the second parameter
// used in the creation function
class IPRoutineConnectionCallback : public IPConnectionCallback {
  public:
    IPRoutineConnectionCallback(void (*func)(IPConnection*, void*),
                                void* data = NULL)
        { _func = func; _data = data; }
    virtual void Execute(IPConnection* conn)
        { (*_func)(conn, _data); }

  private:
    void (*_func)(IPConnection*, void*);
    void* _data;
};

// Base class for calling back in response to an incoming message
class IPHandlerCallback// : public IPResource {
{
public:
    virtual void Execute(IPMessage*) = 0;
};

// Convenience routine for calling back a function rather than another subclass
// of IPHandlerCallback.  The routine should be of the form
//   void func(IPCommunicator* comm, IPMessage* msg, void* data
// where comm is the IPCommunicator, msg is the incoming message and data is
// the third parameter used in the creation function
class IPRoutineHandlerCallback : public IPHandlerCallback {
  public:
    IPRoutineHandlerCallback(IPCommunicator* comm,
                             void (*func)(IPCommunicator*, IPMessage*, void*),
                             void* param = 0)
        { _communicator = comm; _func = func; _param = param; }

    IPCommunicator* Communicator() const { return _communicator; }

    virtual void Execute(IPMessage* m) { (*_func)(_communicator, m, _param); }

  private:
    IPCommunicator* _communicator;
    void (*_func)(IPCommunicator*, IPMessage*, void*);
    void* _param;
};

// Base class for calling back in response to a timer
class IPTimerCallback// : public IPResource {
{
public:
    virtual void Execute(IPTimer*) = 0;
};

// Convenience routine for calling back a function rather than another subclass
// of IPTimerCallback.  The routine should be of the form
//   void func(IPCommunicator*, IPTimer*, void* data)
// where conn is the connection in question and data is the second parameter
// used in the creation function
class IPRoutineTimerCallback : public IPTimerCallback {
  public:
    IPRoutineTimerCallback(IPCommunicator* comm,
                           void (*func)(IPCommunicator*, IPTimer*, void*),
                                void* data = NULL)
        { _func = func; _data = data; _comm = comm; }
    virtual void Execute(IPTimer* timer)
        { (*_func)(_comm, timer, _data); }

  private:
    void (*_func)(IPCommunicator*, IPTimer*, void*);
    void* _data;
    IPCommunicator* _comm;
};

class IPHandlerCallbackAdapter {
	public:
		IPHandlerCallbackAdapter(IPHandlerCallback* callback)
		{ communicator = NULL; handlerCallback = callback; directCallback = NULL; userData = NULL;
		//pthread_mutex_init(&mutex,NULL);
		}

		IPHandlerCallbackAdapter(IPCommunicator* comm,
				void (*callback)(IPCommunicator*, IPMessage*, void*), void *data)
		{ communicator = comm; handlerCallback = NULL; directCallback = callback; userData = data;
		//pthread_mutex_init(&mutex,NULL);
		}

		void Execute(MSG_INSTANCE msgInstance, BYTE_ARRAY rawData)
		{
			FORMATTER_PTR formatter = IPC_msgInstanceFormatter(msgInstance);
			void *callData;
			IPC_unmarshall(formatter, rawData,(void **)&callData);
			IPMessage *msg = new IPMessage(rawData, callData, formatter, msgInstance);

			//pthread_mutex_lock(&mutex);
			if(handlerCallback!=NULL)
				handlerCallback->Execute(msg);
			else if(directCallback!=NULL)
				(*directCallback)(communicator, msg, userData);
			//pthread_mutex_unlock(&mutex);

			delete msg;
			msg = NULL;
		}

	private:
		IPHandlerCallback* handlerCallback;
		void (*directCallback)(IPCommunicator*, IPMessage*, void*);
		void *userData;
		IPCommunicator* communicator;
		//pthread_mutex_t mutex;
};

class IPConnectionCallbackAdapter {
	public:
		IPConnectionCallbackAdapter(IPConnectionCallback* callback)
		{ connectionCallback = callback; directCallback=NULL; userData = NULL;
		//pthread_mutex_init(&mutex,NULL);
		}

		IPConnectionCallbackAdapter(void (*callback)(IPConnection*, void*), void *data)
		{ connectionCallback = NULL; directCallback = callback; userData = data;
		//pthread_mutex_init(&mutex,NULL);
		}

		void Execute(const char *moduleName)
		{
			IPConnection * connection = new IPConnection(moduleName);
			//pthread_mutex_lock(&mutex);
			if(connectionCallback!=NULL)
				connectionCallback->Execute(connection);
			else if(directCallback!=NULL)
				(*directCallback)(connection, userData);
			//pthread_mutex_unlock(&mutex);
			delete connection;
		}

	private:
		IPConnectionCallback* connectionCallback;
		void (*directCallback)(IPConnection*, void*);
		void *userData;
		//pthread_mutex_t mutex;
};



// This hoary thing emulates templates for creating HandlerCallbacks in a
// convenient fashion.  The usages is as follows.  Say you have a class,
// AClass, and you want to call a member function in result to a message
// First you must do
//  declareHandlerCallback(AClass);
//  implementHandlerCallback(AClass);
// The declare declares it, as any class definition and implement implements it
// The usage is then,
//   IPCallback* cb =
//        new HandlerCallback(AClass)((AClass*) instance,
//                                    &AClass::member_function);
// The callback can be passes into IPCommunicator::RegisterHandler

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define __HandlerCallback(T) T##_HandlerCallback
#define HandlerCallback(T) __HandlerCallback(T)
#define __HandlerMemberFunction(T) T##_HandlerMemberFunction
#define HandlerMemberFunction(T) __HandlerMemberFunction(T)
#else
#define __HandlerCallback(T) T/**/_HandlerCallback
#define HandlerCallback(T) __HandlerCallback(T)
#define __HandlerMemberFunction(T) T/**/_HandlerMemberFunction
#define HandlerMemberFunction(T) __HandlerMemberFunction(T)
#endif

#define declareHandlerCallback(T) \
typedef void (T::*HandlerMemberFunction(T))(IPMessage*); \
class HandlerCallback(T) : public IPHandlerCallback { \
public: \
    HandlerCallback(T)(T*, HandlerMemberFunction(T)); \
\
    virtual void Execute(IPMessage*); \
private: \
    T* obj_; \
    HandlerMemberFunction(T) func_; \
};

#define implementHandlerCallback(T) \
HandlerCallback(T)::HandlerCallback(T)(T* obj, HandlerMemberFunction(T) func) { \
    obj_ = obj; \
    func_ = func; \
} \
\
void HandlerCallback(T)::Execute(IPMessage* m) { (obj_->*func_)(m); }

// This stuff emulates templates for creating ConnectionCallbacks in a
// convenient fashion.  The usages is as follows.  Say you have a class,
// AClass, and you want to call a member function in result to a message
// First you must do
//  declareConnectionCallback(AClass);
//  implementConnectionCallback(AClass);
// The declare declares it, as any class definition and implement implements it
// The usage is then,
//   IPCallback* cb =
//        new ConnectionCallback(AClass)((AClass*) instance,
//                                    &AClass::member_function);

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define __ConnectionCallback(T) T##_ConnectionCallback
#define ConnectionCallback(T) __ConnectionCallback(T)
#define __ConnectionMemberFunction(T) T##_ConnectionMemberFunction
#define ConnectionMemberFunction(T) __ConnectionMemberFunction(T)
#else
#define __ConnectionCallback(T) T/**/_ConnectionCallback
#define ConnectionCallback(T) __ConnectionCallback(T)
#define __ConnectionMemberFunction(T) T/**/_ConnectionMemberFunction
#define ConnectionMemberFunction(T) __ConnectionMemberFunction(T)
#endif

#define declareConnectionCallback(T) \
typedef void (T::*ConnectionMemberFunction(T))(IPConnection*); \
class ConnectionCallback(T) : public IPConnectionCallback { \
public: \
    ConnectionCallback(T)(T*, ConnectionMemberFunction(T)); \
\
    virtual void Execute(IPConnection*); \
private: \
    T* obj_; \
    ConnectionMemberFunction(T) func_; \
};

#define implementConnectionCallback(T) \
ConnectionCallback(T)::ConnectionCallback(T)(T* obj, ConnectionMemberFunction(T) func) { \
    obj_ = obj; \
    func_ = func; \
} \
\
void ConnectionCallback(T)::Execute(IPConnection* m) { (obj_->*func_)(m); }


// This hoary thing emulates templates for creating TimerCallbacks in a
// convenient fashion.  The usages is as follows.  Say you have a class,
// AClass, and you want to call a member function in result to a message
// First you must do
//  declareTimerCallback(AClass);
//  implementTimerCallback(AClass);
// The declare declares it, as any class definition and implement implements it
// The usage is then,
//   IPCallback* cb =
//        new TimerCallback(AClass)((AClass*) instance,
//                                    &AClass::member_function);
// The callback can be passes into IPCommunicator::RegisterTimer

#if defined(__STDC__) || defined(__ANSI_CPP__)
#define __TimerCallback(T) T##_TimerCallback
#define TimerCallback(T) __TimerCallback(T)
#define __TimerMemberFunction(T) T##_TimerMemberFunction
#define TimerMemberFunction(T) __TimerMemberFunction(T)
#else
#define __TimerCallback(T) T/**/_TimerCallback
#define TimerCallback(T) __TimerCallback(T)
#define __TimerMemberFunction(T) T/**/_TimerMemberFunction
#define TimerMemberFunction(T) __TimerMemberFunction(T)
#endif

#define declareTimerCallback(T) \
typedef void (T::*TimerMemberFunction(T))(IPTimer*); \
class TimerCallback(T) : public IPTimerCallback { \
public: \
    TimerCallback(T)(T*, TimerMemberFunction(T)); \
\
    virtual void Execute(IPTimer*); \
private: \
    T* obj_; \
    TimerMemberFunction(T) func_; \
};

#define implementTimerCallback(T) \
TimerCallback(T)::TimerCallback(T)(T* obj, TimerMemberFunction(T) func) { \
    obj_ = obj; \
    func_ = func; \
} \
\
void TimerCallback(T)::Execute(IPTimer* t) \
   { (obj_->*func_)(t); }

#endif
