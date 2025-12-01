#ifndef vx_connection_h
#define vx_connection_h


#include <ipt/connection.h>

class VxConnection : public IPConnection {
  public:
    VxConnection(const char*, const char*, IPCommunicator*,
                 VxConnection* = NULL);
    virtual ~VxConnection();

    virtual int Active();
    virtual int Viable();
    virtual int Send(IPMessage*);
    virtual IPMessage* Receive();
    virtual void Deactivate();
    virtual int DataAvailable() { return 0; }

    void RemoteDeactivate();
    int RemoteReceive(IPMessage*);
    virtual void ActivateVx(VxConnection*);
    void SetOther(VxConnection* other) { _other = other; }
    VxConnection* Other() const { return _other; }

  private:
    VxConnection* _other;
    IPConnection* _holder;
    IPMessage* _msg_holder;
    int _waiting_for_message;
};
  
#endif
