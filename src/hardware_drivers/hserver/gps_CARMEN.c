#include "gps_CARMEN.h"
#include "PoseCalculatorInterface.h"
#include "LogManager.h"
#include "ipt_handler.h"
#include "ipt/callbacks.h"

declareHandlerCallback(GpsCARMEN)
implementHandlerCallback(GpsCARMEN)

const int GpsCARMEN::WATCHDOG_CHECK_INTERVAL_SEC_ = 10;

void gpsCARMENLocalizeHandler(MSG_INSTANCE msgInstance, void *callData, void *clientData)
{
	static FORMATTER_PTR formatter = IPC_msgInstanceFormatter(msgInstance);
	((GpsCARMEN *)clientData)->callbackLocalizeScan((carmen_localize_globalpos_message *)callData);
	IPC_freeData(formatter, callData);
}

void *startGpsCARMENThread_(void *gpsCARMENInstance)
{
    ((GpsCARMEN*)gpsCARMENInstance)->reader_thread();
    return NULL;
}

GpsCARMEN::GpsCARMEN(Gps **a, const string& strPortString) :
	Gps(a)
{
	int i;

	save_packets = false;
	for (i = 0; i < EnGpsInfoType_COUNT; i++)
	{
		stats[i] = 0;
	}

	report_level = HS_REP_LEV_NORMAL;

	version = "1.0";
	rt20Status = 100;
	poseCalcID_ = -1;

	printfTextWindow("Starting GPS CARMEN.\n");

	addSensors();
	updateStatusBar_();

    server = ipc_preferred_server_name(strPortString.c_str());

    pthread_create(&gpsCARMENreaderThread, NULL, startGpsCARMENThread_,(void*)this);
    refreshScreen();
}

void GpsCARMEN::callbackLocalizeScan(carmen_localize_globalpos_message* msg)
{
	HSPose_t pose;

	//printfTextWindow("GPS LOCALIZE:\tX = %g\tY = %g\tTheta = %g   XY: %g  Converged = %d\n",
	//		msg->globalpos_std.x, msg->globalpos_std.y, msg->globalpos_std.theta, msg->globalpos_xy_cov, msg->converged);

	pthread_mutex_lock(&mutexStatsLock);

	if((msg->globalpos_std.x>0.15) || (msg->globalpos_std.y>0.15) || (msg->globalpos_std.theta>0.15))
		rt20Status = 110; //If std > 15cm or aprox 10 degrees, worst confidence
	else
		rt20Status = 100; //Else, best confidence

	stats[EnGpsInfoType_DIRECTION       ] = msg->globalpos.theta * 180.0 / M_PI;
	if(stats[EnGpsInfoType_DIRECTION       ]<0)
		stats[EnGpsInfoType_DIRECTION       ]+=360;

	stats[EnGpsInfoType_HORIZONTAL_SPEED] = 0;
	stats[EnGpsInfoType_VERTICAL_SPEED  ] = 0;
	stats[EnGpsInfoType_LAT] = 0;
	stats[EnGpsInfoType_LON] = 0;
	stats[EnGpsInfoType_HEIGHT] = 0;
	stats[EnGpsInfoType_NUM_SATS] = 1;
	xyt[0] = msg->globalpos.x;
	xyt[1] = msg->globalpos.y;
    xyt[2] = stats[EnGpsInfoType_DIRECTION];
    pthread_mutex_unlock(&mutexStatsLock);
    updateStatusBar_();

	if ((gPoseCalc != NULL) && (poseCalcID_ >= 0) && (msg->converged))
	{
		pose.rot.yaw = msg->globalpos.theta * 180.0 / M_PI;
		if(pose.rot.yaw<0)
			pose.rot.yaw+=360;

		pose.rot.extra.time = getCurrentEpochTime();
		gPoseCalc->updateModuleRotation(poseCalcID_, pose.rot);

		pose.loc.x = msg->globalpos.x;
		pose.loc.y = msg->globalpos.y;
		pose.loc.extra.time = getCurrentEpochTime();
		gPoseCalc->updateModuleLocation(poseCalcID_, pose.loc);
	}
}

void GpsCARMEN::updateStatusBar_(void)
{
    char buf[400];

    sprintf(statusStr, "Gps CARMEN: ");

    pthread_mutex_lock(&mutexStatsLock);

    sprintf(
    		buf,
    		"%c X: %.2f   Y: %.2f   Theta: %.2f",
    		statusbarSpinner_->getStatus(),
    		xyt[0],
    		xyt[1],
    		xyt[2]);

    pthread_mutex_unlock(&mutexStatsLock);


    strcat(statusStr, buf);
    statusbar->update(statusLine);
}


void GpsCARMEN::reader_thread()
{
	printTextWindow("GPS CARMEN thread started");

	switch(ipc_connect_preferred_server(server))
	{
	case IPC_CONNECTION_ERROR:
		printTextWindow("Error connecting carmenRobot");
		return;
	case IPC_ALREADY_CONNECTED:
		IPC_subscribeData(MISSIONLAB_LOCALIZE_GLOBALPOS_NAME, gpsCARMENLocalizeHandler, this);
		return;
	}

	IPC_subscribeData(MISSIONLAB_LOCALIZE_GLOBALPOS_NAME, gpsCARMENLocalizeHandler, this);

	 while (true)
	 {
		 pthread_testcancel();
	     IPC_listenWait(1000);
	 }
}
