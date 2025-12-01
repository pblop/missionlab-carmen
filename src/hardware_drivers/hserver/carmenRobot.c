#include "carmenRobot.h"
#include "ipt_handler.h"
#include "ipt/callbacks.h"

declareHandlerCallback(CarmenRobot)
implementHandlerCallback(CarmenRobot)

const int CarmenRobot::READER_LOOP_USLEEP_ = 100000;
const int CarmenRobot::SKIP_STATUSBAR_UPDATE_ = 0;

#define deg_range_pm_180(t) {(t)=fmod((double)(t),360); if((t)>180) (t) -= 360; if((t)<=-180) (t) += 360;}

pthread_mutex_t CarmenRobot::deltaPoseMutex_ = PTHREAD_MUTEX_INITIALIZER;

void carmenRobotSonarHandler(MSG_INSTANCE msgInstance, void *callData, void *clientData)
{
	static FORMATTER_PTR formatter = IPC_msgInstanceFormatter(msgInstance);
	((CarmenRobot *)clientData)->callbackSonarScan((carmen_base_sonar_message *)callData);
	IPC_freeData(formatter, callData);
}

void carmenRobotOdometryHandler(MSG_INSTANCE msgInstance, void *callData, void *clientData)
{
	static FORMATTER_PTR formatter = IPC_msgInstanceFormatter(msgInstance);
	((CarmenRobot *)clientData)->callbackOdometryScan((carmen_base_odometry_message *)callData);
	IPC_freeData(formatter, callData);
}

void carmenRobotTruePosHandler(MSG_INSTANCE msgInstance, void *callData, void *clientData)
{
	static FORMATTER_PTR formatter = IPC_msgInstanceFormatter(msgInstance);
	((CarmenRobot *)clientData)->callbackTruePosScan((carmen_simulator_truepos_message *)callData);
	IPC_freeData(formatter, callData);
}

CarmenRobot::CarmenRobot(Robot** a, const string& strPortString) :
	Robot(a, HS_ROBOT_TYPE_CARMEN, HS_MODULE_NAME_ROBOT_CARMEN)
{
	resetPose_();
	partSonar_ = NULL;
	partXyt_ = NULL;
    isReading_ = false;
    partXyt_ = new RobotPartXyt();

    newPose.loc.x = 0;
    newPose.loc.y = 0;
    newPose.rot.yaw = 0;
    newPose.extra.time = getCurrentEpochTime();

    updateStatusBar_();

    server = ipc_preferred_server_name(strPortString.c_str());

    pthread_create(&robotThread_, NULL, &startRobotThread_,(void*)this);
    refreshScreen();
}

CarmenRobot::~CarmenRobot(void)
{
	if (isReading_)
	{
		pthread_cancel(robotThread_);
		pthread_join(robotThread_, NULL);
		isReading_ = false;
	}

	if (partXyt_ != NULL)
	{
		delete partXyt_;
		partXyt_ = NULL;
	}

	if (partSonar_ != NULL)
	{
		delete partSonar_;
		partSonar_ = NULL;
	}

	printTextWindow("Carmen robot disconnected");
	redrawWindows();
}


void *CarmenRobot::startRobotThread_(void *carmenRobotInstance)
{
    ((CarmenRobot*)carmenRobotInstance)->robotLoop_();
    return NULL;
}

//---------------------------------------------------------------------
// Esta funcion comienza el ciclo de lecturas
//---------------------------------------------------------------------
void CarmenRobot::robotLoop_(void)
{
    int count = 0;

    printTextWindow("Carmen robot thread started");

	if(!ipc_connect_preferred_server(server))
	{
		printTextWindow("Error connecting carmenRobot");
		return;
	}

    ipc_hook_message(CARMEN_BASE_ODOMETRY_NAME, CARMEN_BASE_ODOMETRY_NAME, CARMEN_BASE_ODOMETRY_FMT);
    ipc_hook_message(CARMEN_BASE_SONAR_NAME, CARMEN_BASE_SONAR_NAME, CARMEN_BASE_SONAR_FMT);

	//IPC_defineMsg(CARMEN_BASE_VELOCITY_NAME,  IPC_VARIABLE_LENGTH, CARMEN_BASE_VELOCITY_FMT);
    IPC_subscribeData(CARMEN_BASE_ODOMETRY_NAME, carmenRobotOdometryHandler, this);
    IPC_subscribeData(CARMEN_BASE_SONAR_NAME, carmenRobotSonarHandler, this);
    //IPC_subscribeData(CARMEN_SIMULATOR_TRUEPOS_NAME, carmenRobotTruePosHandler, this);

	isReading_ = true;

    while(true)
    {
        // Controlar que la terminacion no fue solicitada.
        pthread_testcancel();

        pthread_mutex_lock(&deltaPoseMutex_);

        setPose_(newPose);

        pthread_mutex_unlock(&deltaPoseMutex_);


        compControlSpeed_();
        controlRobot_();

        // Calcular la frecuencia.
        compFrequency_();

        if (count > SKIP_STATUSBAR_UPDATE_)
        {
            updateStatusBar_();
            count = 0;
        }
        count++;

        IPC_listenWait(READER_LOOP_USLEEP_/1000);
    }
}

//---------------------------------------------------------------------
// This function updates the status bar
//---------------------------------------------------------------------
void CarmenRobot::updateStatusBar_(void)
{
    HSPose_t pose;
    char buf[300];

    statusStr[0] = 0;
    strcat(statusStr, "CarmenRobot: ");
    getPose(pose);

    sprintf(buf, "%c  x: %.2f  y: %.2f  t: %.2f", statusbarSpinner_->getStatus(),
            pose.loc.x, pose.loc.y, pose.rot.yaw);

    strcat(statusStr, buf);
    statusbar->update(statusLine);
}


void CarmenRobot::controlRobot_()
{
	HSSpeed_t controlSpeed;
	getControlSpeed_(controlSpeed);

    ipc_send_base_velocity(controlSpeed.driveSpeed.value,
    		controlSpeed.steerSpeed.value * M_PI / 180.0);
}

void CarmenRobot::callbackTruePosScan(carmen_simulator_truepos_message* msg)
{
	//printfTextWindow("TRUEPOSE:\tX = %g\tY = %g\tTheta = %g\n",
	//		msg->truepose.x, msg->truepose.y, msg->truepose.theta * 180.0 / M_PI);

}

void CarmenRobot::callbackLocalizeScan(IPMessage* message)
{
	carmen_localize_globalpos_message *msg = (carmen_localize_globalpos_message *)message->FormattedData();

	printfTextWindow("LOCALIZE:\tX = %g\tY = %g\tTheta = %g\n",
			msg->globalpos.x, msg->globalpos.y, msg->globalpos.theta * 180.0 / M_PI);

	//newPose.loc.x = msg->globalpos.x;
	//newPose.loc.y = msg->globalpos.y;
	//newPose.rot.yaw = msg->globalpos.theta * 180.0 / M_PI;
	//newPose.extra.time = getCurrentEpochTime();
}

void CarmenRobot::callbackOdometryScan(carmen_base_odometry_message* msg)
{
    pthread_mutex_lock(&deltaPoseMutex_);

	double time = getCurrentEpochTime();

	newPose.loc.x = msg->x;
	newPose.loc.y = msg->y;
	newPose.rot.yaw = msg->theta * 180.0 / M_PI;
	newPose.extra.time = time;

    pthread_mutex_unlock(&deltaPoseMutex_);
}

void CarmenRobot::callbackSonarScan(carmen_base_sonar_message* msg)
{
	if(partSonar_ == NULL)
	{
		printTextWindow("Initializing robot sonars...");

		sonarAngles = new float[msg->num_sonars];
		sonarLoc = new float[msg->num_sonars][6];
		sonar = new float[msg->num_sonars];

		printfTextWindow("Num sonars: %d", msg->num_sonars);

		for(int i=0;i<msg->num_sonars;i++)
		{
			sonarAngles[i] = msg->sonar_offsets[i].theta * 180 / M_PI;
			sonarLoc[i][0] = newPose.loc.x;
			sonarLoc[i][1] = newPose.loc.y;
			sonarLoc[i][2] = msg->sonar_offsets[i].theta * 180 / M_PI;

			sonarLoc[i][3] = msg->sonar_offsets[i].x;
			sonarLoc[i][4] = msg->sonar_offsets[i].y;
			sonarLoc[i][5] = msg->cone_angle * 180 / M_PI;

			sonar[i] = msg->range[i];
		}

		partSonar_ = new RobotPartSonar(msg->num_sonars, sonarAngles);
	    sensorSonar_ = new Sensor(SENSOR_SONAR, msg->num_sonars*6*sizeof(float),
	    		(char*)sonarLoc, msg->num_sonars, sonarAngles, msg->num_sonars, sonar);

		printTextWindow("Robot sonars initialized!!");
		return;
	}

	for(int i=0;i<msg->num_sonars;i++)
	{
		sonarAngles[i] = msg->sonar_offsets[i].theta * 180 / M_PI;
		sonarLoc[i][0] = newPose.loc.x;
		sonarLoc[i][1] = newPose.loc.y;
		sonarLoc[i][2] = msg->sonar_offsets[i].theta * 180 / M_PI;
		sonar[i] = msg->range[i];
	}

}

//-------------------------------------------------------------------------------------
// Esta funcion envia comandos al robot basados en las entradas de teclado del usuario
//-------------------------------------------------------------------------------------
void CarmenRobot::control()
{
    int c;
    int done = 0;
    int msgData[3];

    msgData[0] = 0;//isRangeOn;
    msgData[1] = 0; // "speed_factor" esta obsoleto.
    msgData[2] = 0; // "angular_speed_factor" esta obsoleto.
    messageDrawWindow(EnMessageType_ROOMBA560, EnMessageErrType_NONE, msgData);

    redrawWindows();

    do
    {
        c = getch();
        switch(c) {

        case 'd':
            messageHide();
            delete this;
            return;
            break;

        case 'p':
            //isRangeOn_ = !isRangeOn_;
            //msgData[0] = isRangeOn_;
            messageDrawWindow(EnMessageType_ROOMBA560, EnMessageErrType_NONE, msgData);
            break;

        case 'r':
            refreshScreen();
            break;

        case 'x':
        case 'Q':
        case KEY_ESC:
            done = true;
            break;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            messageMovePanelKey(c);
            break;
        }
    } while (!done);
    messageHide();
}
