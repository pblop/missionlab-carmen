#ifndef ip_vxcommunicator_h
#define ip_vxcommunicator_h

#include <ipt/tcpcommunicator.h>
#include <semLib.h>

class VxConnection;

class VxCommunicator : public TCPCommunicator {
  public:
    VxCommunicator(const char* mod_name, const char* host_name = 0);

    virtual int WaitForInput(double);

    VxConnection* AddVxConnection(VxConnection*);
    void FlagActivity() { _activity = 1; }

    virtual void AddEvent(IPEvent*);
    virtual void QueueMessage(IPMessage*);
    virtual void QueuePigeonHole(IPPigeonHole*);

    virtual IPEvent* pop_event();
    virtual IPMessage* pop_message();
    virtual IPPigeonHole* pop_hole();

    virtual IPMessage* remove_message(int (*)(const char*, const char*),
                                      const char*, int = 0);
    virtual IPPigeonHole* remove_hole(int (*)(const char*, const char*),
                                      const char*, int = 0);

  private:
    int _activity;
    SEM_ID _message_sem;
    SEM_ID _hole_sem;
    SEM_ID _event_sem;
};

#endif
