#ifndef GPS_CARMEN_H
#define GPS_CARMEN_H

#include "gps.h"
#include "ipt_handler.h"
#include "hserver_ipt.h"
#include "ipc_client.h"

class GpsCARMEN : public Gps
{

public:
	GpsCARMEN(Gps **a, const string& strPortString);
	void callbackLocalizeScan(carmen_localize_globalpos_message* msg);
	void reader_thread();

protected:
	void updateStatusBar_(void);
    IPConnection *m_carmenConnection;
    IPHandlerCallback* m_localizeIptCallback;
    pthread_t gpsCARMENreaderThread;
    const char *server;

	IPCommunicator* m_communicator;
    static const int WATCHDOG_CHECK_INTERVAL_SEC_;
};

#endif //GPS_CAN_H
