/**********************************************************************
 **                                                                  **
 **                            roomba.c                              **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **  Roomba robot driver for HServer                                 **
 **********************************************************************/


#include <stdio.h>
#include <termio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <ctype.h>
#include <string>
#include <sys/poll.h>
#include <errno.h>
#include <math.h>
#include <signal.h>

#include "HServerTypes.h"
#include "message.h"
#include "roomba.h"
#include "hserver.h"
#include "camera.h"
#include "statusbar.h"
#include "robot_config.h"
#include "sensors.h"
#include "robot.h"
#include "PoseCalculatorInterface.h"




//---------------------------------------------------------------------
//Comandos ROOMBA
//---------------------------------------------------------------------
const int Roomba::CMD_START_ = 128;
const int Roomba::CMD_BAUD_ = 129;
const int Roomba::CMD_CONTROL_ = 130;
const int Roomba::CMD_SAFE_ = 131;
const int Roomba::CMD_FULL_ = 132;
const int Roomba::CMD_POWER_ = 133;
const int Roomba::CMD_SPOT_ = 134;
const int Roomba::CMD_CLEAN_ = 135;
const int Roomba::CMD_MAX_ = 136;
const int Roomba::CMD_DRIVE_ = 137;
const int Roomba::CMD_MOTORS_ = 138;
const int Roomba::CMD_SENSORS_ = 142;
const int Roomba::CMD_LED_ = 139;
const int Roomba::CMD_SONG_ = 140;
const int Roomba::CMD_PLAY_ = 141;

//---------------------------------------------------------------------
//Tiempos de espera
//---------------------------------------------------------------------
const int Roomba::READ_BACKOFF_USEC_ = 100;
const int Roomba::READER_LOOP_USLEEP_ = 1000;
const int Roomba::SKIP_STATUSBAR_UPDATE_ = 10;

//---------------------------------------------------------------------
//Velocidades Máximas
//---------------------------------------------------------------------
const int Roomba::MAX_VELOCIDAD = 500;
const int Roomba::MIN_VELOCIDAD = -500;
const int Roomba::MAX_ARCO = 2000;
const int Roomba::MIN_ARCO = -2000;


const int Roomba::ANCHURA_ = 258;

//---------------------------------------------------------------------
//Indices del paquete de datos
//---------------------------------------------------------------------
const int Roomba::BUMPSWHEELDROPS     = 0;
const int Roomba::WALL                = 1;
const int Roomba::CLIFFLEFT           = 2;
const int Roomba::CLIFFFRONTLEFT      = 3;
const int Roomba::CLIFFFRONTRIGHT     = 4;
const int Roomba::CLIFFRIGHT          = 5;
const int Roomba::VIRTUALWALL         = 6;
const int Roomba::MOTOROVERCURRENTS   = 7;
const int Roomba::DIRTLEFT            = 8;
const int Roomba::DIRTRIGHT           = 9;
const int Roomba::REMOTEOPCODE        = 10;
const int Roomba::BUTTONS             = 11;
const int Roomba::DISTANCE_HI         = 12;
const int Roomba::DISTANCE_LO         = 13;
const int Roomba::ANGLE_HI            = 14;
const int Roomba::ANGLE_LO            = 15;
const int Roomba::CHARGINGSTATE       = 16;
const int Roomba::VOLTAGE_HI          = 17;
const int Roomba::VOLTAGE_LO          = 18;
const int Roomba::CURRENT_HI          = 19;
const int Roomba::CURRENT_LO          = 20;
const int Roomba::TEMPERATURE         = 21;
const int Roomba::CHARGE_HI           = 22;
const int Roomba::CHARGE_LO           = 23;
const int Roomba::CAPACITY_HI         = 24;
const int Roomba::CAPACITY_LO         = 25;

//---------------------------------------------------------------------
//Mascaras para desentramar los datos
//---------------------------------------------------------------------
const int Roomba::WHEELDROP_MASK      = 0x1C;
const int Roomba::BUMP_MASK           = 0x03;
const int Roomba::BUMPRIGHT_MASK      = 0x01;
const int Roomba::BUMPLEFT_MASK       = 0x02;
const int Roomba::WHEELDROPRIGHT_MASK = 0x04;
const int Roomba::WHEELDROPLEFT_MASK  = 0x08;
const int Roomba::WHEELDROPCENT_MASK  = 0x10;

const int Roomba::MOVERDRIVELEFT_MASK = 0x10;
const int Roomba::MOVERDRIVERIGHT_MASK= 0x08;
const int Roomba::MOVERMAINBRUSH_MASK = 0x04;
const int Roomba::MOVERVACUUM_MASK    = 0x02;
const int Roomba::MOVERSIDEBRUSH_MASK = 0x01;

const int Roomba::POWERBUTTON_MASK    = 0x08;
const int Roomba::SPOTBUTTON_MASK     = 0x04;
const int Roomba::CLEANBUTTON_MASK    = 0x02;
const int Roomba::MAXBUTTON_MASK      = 0x01;



const float Roomba::SENSORES_ANGLES_[] = {45,-45,15,75,-75,-15,-20,90,0,-90};
const double Roomba::MAX_SENSOR_READABLE_DISTANCE_ = 5.0;
const double Roomba::MIN_ROBOT_VOLTAGE_ = 10.8;

//---------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------
Roomba::Roomba(Robot** a, const string& strPortString) :
    Robot(a, HS_ROBOT_TYPE_ROOMBA, HS_MODULE_NAME_ROBOT_ROOMBA)
{
    char commandBuf[6];
	byte_t dat;
    struct termios term;

    partXyt_ = NULL;
    partSensores_ = NULL;
    sensorRoomba_ = NULL;
    robotName_ = NULL;
    isReading_ = false;
    numSensores_ = NUM_SENSORES;
    int flags;

    pthread_mutex_init(&serialMutex_, NULL);

    if ((report_level > HS_REP_LEV_NONE) && (report_level != HS_REP_LEV_DEBUG))
    {
        printTextWindow("Connecting to Roomba... ", false);
    }

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Connecting to Roomba");
        printfTextWindow("robot serial port: %s", strPortString.c_str());
    }

     if(setupSerial(&commFd_, strPortString.c_str(), B57600, report_level, "Robot", 1) == -1){
		printTextWindow("Sin conexion\n");
		exit(0);
	}

    redrawWindows();
    updateStatusBar_();
    redrawWindows();

    aspirar = true;

	//Mandar el comando START
    sleep(1);
	initRobot();
	sleep(1);
	byte_t pack[9];
	pack[0] = CMD_LED_;
	pack[1] = 25;
	pack[2] = 0;
	pack[3] = 128;
	enviar_(4, pack,true);


    if ((report_level > HS_REP_LEV_NONE) && (report_level != HS_REP_LEV_DEBUG))
    {
        printTextWindow("connected", true);
    }
    else if (report_level > HS_REP_LEV_NONE)
    {
        printTextWindow("Roomba connected", true);
    }

    pthread_create(&readerThread_, NULL, &startReaderThread_,(void*)this);

    addSensors_();

}

//---------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------
Roomba::~Roomba(void)
{
    byte_t comando = CMD_POWER_;
    enviar_(1, &comando, true);
    usleep(T_MUESTREO);


    if (isReading_)
    {
        pthread_cancel(readerThread_);
        pthread_join(readerThread_, NULL);
        isReading_ = false;
    }

    if (partXyt_ != NULL)
    {
        delete partXyt_;
        partXyt_ = NULL;
    }


    if (partSensores_ != NULL)
    {
        delete partSensores_;
        partSensores_ = NULL;
    }

    if (sensorRoomba_ != NULL)
    {
        delete sensorRoomba_;
        sensorRoomba_ = NULL;
    }

    printTextWindow("Roomba disconnected");

    close(commFd_);

    if(robotName_ != NULL)
    {
        free(robotName_);
        free(robotClass_);
        free(subClass_);
    }

    redrawWindows();
}

//---------------------------------------------------------------------
// Esta función limpia el buffer del puerto serie.
//---------------------------------------------------------------------
void Roomba::flushSerialLine_(void)
{
    byte_t buf[26];
    int cnum;

    do
    {
        cnum=read(commFd_, buf, 26);
        if (cnum == -1)
        {
            cnum = 0;
        }
    } while(cnum);
}


//---------------------------------------------------------------------
// Esta función lee los bytes disponibles de el descriptor de fichero.
//---------------------------------------------------------------------
int Roomba::nextRemainingBytes_(byte_t *bytes, int max)
{
    int cnum;
    int numTries = 25;
	struct pollfd ufd[1];
	int retval;
	int timeout=50000;


    do
    {
		cnum = read(commFd_, bytes, max);
		if (cnum == 0)
		{
			numTries--;
			if (numTries == 0)
			{
				return -1;
			}
		}
		if (cnum == -1)
		{
			perror("Read: ");
			return -1;
		}
    } while ((cnum == 0) || (cnum == -1));

    return cnum;
}


//---------------------------------------------------------------------
// Esta función actualiza la barra de estado.
//---------------------------------------------------------------------
void Roomba::updateStatusBar_(void)
{
    HSPose_t pose;
    char buf[100];

    statusStr[0] = 0;

    strcat(statusStr, "Roomba: ");

        getPose(pose);

        sprintf(
            buf,
            "%c %.2fV  x: %.2f  y: %.2f  t: %.2f ",
            statusbarSpinner_->getStatus(),
            battery_,
            pose.loc.x,
            pose.loc.y,
            pose.rot.yaw);

        strcat(statusStr, buf);

    statusbar->update(statusLine);
}

//---------------------------------------------------------------------
// Esta función envía los datos a través del puerto serie
//---------------------------------------------------------------------
int Roomba::enviar_(int tam, byte_t pack[], bool bloqueante)
{
    int devuelto;

    if(bloqueante)
    {
        // Se bloquea el mutex
        pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &serialMutex_);
        pthread_mutex_lock(&serialMutex_);

        devuelto = write(commFd_, pack, tam);

        // y se desbloquea
        pthread_cleanup_pop(1);
    }


    else
        devuelto = write(commFd_, pack, tam);

    return devuelto;
}


//---------------------------------------------------------------------
// Esta función actualiza la información relativa al robot.
//---------------------------------------------------------------------
void Roomba::updateRobotInfo_(RobotInfo_t robotInfo)
{
    HSPose_t pose;
    HSPose_t deltaPose;
    double tdif;
    static RobotInfo_t oldRobotInfo;
    static int batteryWarn = 0;
    double xdif, ydif;
	int i;
    static bool initialized = 0;

    if (!initialized)
    {
        oldRobotInfo = robotInfo;
        initialized = true;
        return;
    }

    xdif = (robotInfo.xPos - oldRobotInfo.xPos);
    ydif = (robotInfo.yPos - oldRobotInfo.yPos);
    tdif = (robotInfo.thetaPos - oldRobotInfo.thetaPos);

    ROBOT_CRAMPDEG(tdif, -200, 200);

    // Resetea la información del robot
    if ((robotInfo.xPos == 0) && (robotInfo.yPos == 0) && (robotInfo.thetaPos == 0))
    {
        xdif = 0;
        ydif = 0;
        tdif = 0;
    }

    memset((HSPose_t *)&deltaPose, 0x0, sizeof(HSPose_t));
    deltaPose.loc.x = xdif / (1000.0);
    deltaPose.loc.y = ydif / (1000.0);
    deltaPose.rot.yaw = tdif;
    deltaPose.extra.time = getCurrentEpochTime();

    advancePose_(deltaPose);
    getPose(pose);

    for(int j = 0; j < numSensores_; j++)
    {
        if(robotInfo.sensores[j] == 1){
            valores[j]=0.01;
        }
        else{
            valores[j] = MAX_SENSOR_READABLE_DISTANCE_;
        }
        sensorLoc_[j][0] = pose.loc.x;
        sensorLoc_[j][1] = pose.loc.y;
        sensorLoc_[j][2] = pose.rot.yaw;
    }

    if(robotInfo.power == 1){
        kill(getpid(), SIGINT);
    }

    battery_level = (double)(robotInfo.battery);

    battery_ = (double)(robotInfo.battery) / 1000.0;

    oldRobotInfo = robotInfo;
}

void Roomba::binario(char * cadena, byte_t num){
  int cociente, binario, i;
   char strbin;
   const int divisor = 2;

   for (i=0; i<8; i++)
		cadena[i] = '0';

	cadena[8] = '\0';

   cociente = (num / divisor);
   binario  = (num % divisor);
   i = 8;
   while (1==1)
   {
      if (binario == 1) {
         strbin = '1';
      }
      else if (binario == 0) {
         strbin = '0';
      }
      i = i - 1;
      cadena[i] = strbin;
      binario  = (cociente % divisor);
      cociente = (cociente / divisor);
      if (i == 1)
      {
         cadena[i-1] = strbin;
         break;
       }
   }
   if (num > 127)
		cadena[0] = '0';
	return;
}


//---------------------------------------------------------------------
// Esta función decodifica el paquete de datos
//---------------------------------------------------------------------
/**
 * Orden de almacenamiento de los datos provenientes de los sensores
 * 0 -> Bumper Izquierdo
 * 1 -> Bumper Derecho
 * 2 -> Cliff Izquierdo
 * 3 -> Cliff Frontal Izquierdo
 * 4 -> Cliff Frontal Derecho
 * 5 -> Cliff Derecho
 * 6 -> Sensor de pared
 * 7 -> Caida rueda Izquierda
 * 8 -> Caida rueda Central
 * 9 -> Caida rueda Derecha
 **/
void Roomba::decodeStatusPack_(byte_t pack[])
{
    RobotInfo_t robotInfo;
    int num, i, base;
	int anguloGrados;
	short angle;
	short distancia;
	char cadena[9];
	static bool initialized = 0;
	static double anguloTotal = 0.0, distanciaTotal = 0.0;
	static double yPosAnt = 0.0, xPosAnt = 0.0;
	double angulo;
	static double anguloAnt = 0.0;
	double xInc, yInc;


	if (!initialized)
    {
		robotInfo.xPos = 0;
		robotInfo.yPos = 0;
		robotInfo.thetaPos = 0;
        initialized = true;
        return;
    }

    robotInfo.sensores[0] = (pack[BUMPSWHEELDROPS] & BUMPLEFT_MASK) ? 1 : 0;
	robotInfo.sensores[1] = (pack[BUMPSWHEELDROPS] & BUMPRIGHT_MASK) ? 1 : 0;
	robotInfo.sensores[7] = (pack[BUMPSWHEELDROPS] & WHEELDROPLEFT_MASK) ? 1 : 0;
	robotInfo.sensores[8] = (pack[BUMPSWHEELDROPS] & WHEELDROPCENT_MASK) ? 1 : 0;
    robotInfo.sensores[9] = (pack[BUMPSWHEELDROPS] & WHEELDROPRIGHT_MASK) ? 1 : 0;


	robotInfo.sensores[2] = pack[CLIFFLEFT] ? 1 : 0;
	robotInfo.sensores[3] = pack[CLIFFFRONTLEFT] ? 1 : 0;
	robotInfo.sensores[4] = pack[CLIFFFRONTRIGHT] ? 1 : 0;
	robotInfo.sensores[5] = pack[CLIFFRIGHT] ? 1 : 0;

    robotInfo.sensores[6] = pack[WALL] ? 1 : 0;

	robotInfo.max = (pack[BUTTONS] & MAXBUTTON_MASK) ? 1 : 0;
	robotInfo.clean = (pack[BUTTONS] & CLEANBUTTON_MASK) ? 1 : 0;
	robotInfo.spot = (pack[BUTTONS] & SPOTBUTTON_MASK) ? 1 : 0;
	robotInfo.power = (pack[BUTTONS] & POWERBUTTON_MASK) ? 1 : 0;

	distancia = pack[DISTANCE_HI];
	distancia = (distancia << 8) + pack[DISTANCE_LO];

	angle = pack[ANGLE_HI];
	angle = (angle << 8) + pack[ANGLE_LO];


	angulo = 2.0*((double)angle)/258.0;

	anguloTotal += angulo;


	if(angulo != 0){
		double radio = (double)distancia/(double)angulo;
		xInc = radio * sin(angulo);
		yInc = radio - radio * cos(angulo);
	}else{
		xInc = (double)distancia;
		yInc = 0;
	}


	robotInfo.xPos = xPosAnt + xInc*cos(anguloAnt) - yInc*sin(anguloAnt);
	robotInfo.yPos = yPosAnt + xInc*sin(anguloAnt) + yInc*cos(anguloAnt);

	anguloAnt = anguloTotal;
	xPosAnt = robotInfo.xPos;
	yPosAnt = robotInfo.yPos;

    anguloGrados = (int)((anguloTotal*180/PI));  //Transformo el ángulo a grados
	robotInfo.thetaPos = (int)(anguloGrados%360);  //Transformo el ángulo para que se encuentre entre 0 y 360

	robotInfo.battery = pack[VOLTAGE_HI];
	robotInfo.battery = robotInfo.battery << 8 + pack[VOLTAGE_LO];

	robotInfo.temperatura = pack[TEMPERATURE];

    updateRobotInfo_(robotInfo);
    updateStatusBar_();
}


//-----------------------------------------------------------------------
// Esta función controla el robot con control speed.
//-----------------------------------------------------------------------
void Roomba::controlRobot_(void)
{
    HSSpeed_t controlSpeed;
	double velocidad;
	short vel, arc;
	double vGiro, giro;
	int arco;
	static double velocidadAnt, vGiroAnt, giroAnt = 0.0, velAnt = 0.0;
    double difAng = 0.0, difVel = 0.0;
	byte_t pack[5];
	short macro = 0x00FF;
	static int inicializar = 0;

    getControlSpeed_(controlSpeed);

	velocidad = (ROBOT_M2MM(controlSpeed.driveSpeed.value));
	vGiro = (controlSpeed.steerSpeed.value);

	giro=vGiro*PI/180.0;

    // ******** CAMBIOS *********
    difAng = fabs(giroAnt) - fabs(giro);
    difVel = fabs(velAnt) - fabs(velocidad);
    if(fabs(difAng) > ROOMBA_UMBRAL_GIRO)
    {
        if(giro != 0.0)
        {
            if(difAng > 0.0)
                giro = giroAnt + ROOMBA_INCREM_GIRO;
            else
                giro = giroAnt - ROOMBA_INCREM_GIRO;
        }

        printfTextWindow("Nuevo giro: %lf\n",giro);
    }
    giroAnt = giro;
    velAnt = velocidad;
    // ******** CAMBIOS *********

	if(giro == 0)
		arco = 32768;
	else{
		arco = (int)(velocidad / giro);
	}

	if(arco > MAX_ARCO || arco < MIN_ARCO )
		arco = 32768;

	if(velocidad > MAX_VELOCIDAD)
		velocidad = MAX_VELOCIDAD;

	if(velocidad < MIN_VELOCIDAD)
		velocidad = MIN_VELOCIDAD;

	if((giro != 0.0) && (velocidad == 0.0))
	{
		velocidad = 200.0;
        if(giro > 0.0) arco = 1;
        else           arco = -1;
	}

	if(inicializar != 0){
		if((velocidad != velocidadAnt) || (vGiro != vGiroAnt)){

			vel = (short)velocidad;
			arc = (short)arco;

			pack[0] = CMD_DRIVE_;
			pack[1] = (byte_t)((vel >> 8) & macro);
			pack[2] = (byte_t)(vel & macro);
			pack[3] = (byte_t)((arc >> 8) & macro);
			pack[4] = (byte_t)(arc & macro);

			enviar_(5, pack, true);

			velocidadAnt = velocidad;
			vGiroAnt = vGiro;
		}
	}else{
		vel = (short)velocidad;
		arc = (short)arco;

		pack[0] = CMD_DRIVE_;
		pack[1] = (byte_t)((vel >> 8) & macro);
		pack[2] = (byte_t)(vel & macro);
		pack[3] = (byte_t)((arc >> 8) & macro);
		pack[4] = (byte_t)(arc & macro);

		enviar_(5, pack, true);

		velocidadAnt = velocidad;
		vGiroAnt = vGiro;
		inicializar = 1;
	}
}


//---------------------------------------------------------------------
// Esta función inicializa los valores de  x, y, and theta
//---------------------------------------------------------------------
void Roomba::initRobot(void)
{
	byte_t comando;
	comando = CMD_START_;
	enviar_(1, &comando, true);
	comando = CMD_CONTROL_;
	enviar_(1, &comando, true);
    comando = CMD_FULL_;
    enviar_(1, &comando, true);
}

//---------------------------------------------------------------------
// Esta función lee un paquete del puerto.
//---------------------------------------------------------------------
int Roomba::readPack_(void)
{
    int n, len, numBytes;
    byte_t pack[26];

    // ******** CAMBIOS *********
    // Se bloquea el mutex
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &serialMutex_);
    pthread_mutex_lock(&serialMutex_);

    pack[0] = 142;
    pack[1] = 0;
    if(enviar_(2, pack,false)<0)
        printTextWindow("ERROR en el envio\n");

    usleep(T_MUESTREO);

    len = 26;
    numBytes = 0;
    while (len)
    {
        n = nextRemainingBytes_(pack + numBytes, len);

        if (n == -1) return -1; // Read timeout in testing

        len -= n;
        numBytes += n;
    }

    // y se desbloquea
    pthread_cleanup_pop(1);
    // ******** CAMBIOS *********

	decodeStatusPack_(pack);

    return len;
}


//---------------------------------------------------------------------
// This function runs the reader in a loop
//---------------------------------------------------------------------
void Roomba::readerLoop_(void)
{
    int count = 0;
	short arc, vel;
	byte_t pack[2];

    isReading_ = true;


    while(true)
    {
        // Se asegura de que no se debe cerrar la aplicación
        pthread_testcancel();

        // Lee datos del robot.
        int dev = readPack_();

        // Realiza el calculo de velociada.
        compControlSpeed_();

        // Controla el robot.
        controlRobot_();


        // Comprueba otra vez el hilo de terminación.
        pthread_testcancel();

        // Compute la frecuencia.
        compFrequency_();

        // Compute la velociadad.
        updateVelocity_();

        if (count > SKIP_STATUSBAR_UPDATE_)
        {
            updateStatusBar_();
            count = 0;
        }
        count++;

        usleep(READER_LOOP_USLEEP_);
    }
}

//---------------------------------------------------------------------
// Esta función configura los  sensores
//---------------------------------------------------------------------
void Roomba::addSensors_(void)
{

    partXyt_ = new RobotPartXyt();

    partSensores_ = new RobotPartRoomba(numSensores_, (float *)SENSORES_ANGLES_);

    sensorRoomba_ = new Sensor(SENSOR_ROOMBA, sizeof(sensorLoc_), (char*)sensorLoc_, numSensores_, (float *)SENSORES_ANGLES_, numSensores_, valores);

}


//---------------------------------------------------------------------
// This function sends commands to the robot based on the user input
//---------------------------------------------------------------------
void Roomba::control()
{
    int c;
    int done = 0;
    int msgData[3];

    msgData[0] = isRangeOn_;
    msgData[1] = 0; // "speed_factor" is obsolete.
    msgData[2] = 0; // "angular_speed_factor" is obsolete.
    messageDrawWindow(EnMessageType_ROOMBA, EnMessageErrType_NONE, msgData);

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
            isRangeOn_ = !isRangeOn_;
            msgData[0] = isRangeOn_;
            messageDrawWindow(EnMessageType_ROOMBA, EnMessageErrType_NONE, msgData);
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

//---------------------------------------------------------------------
// This function starts the reader thread
//---------------------------------------------------------------------
void *Roomba::startReaderThread_(void *roombaInstance)
{
    ((Roomba*)roombaInstance)->readerLoop_();
    return NULL;
}

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------
void Roomba::setMotores(int valor)
{
    byte_t pack[2];
    pack[0] = CMD_MOTORS_;

    if(valor == 0)
        pack[1] = (byte_t)valor;
    else
        pack[1] = (byte_t)7;
    enviar_(2, pack,true);
}


//-----------------------------------------------------------------------
// This function computes the input speed of the robot that would be
// commanded.
//-----------------------------------------------------------------------
void Roomba::compControlSpeed_(void)
{
    HSSpeed_t desiredSpeed;
    HSRotation_t currentRot;
    HSRotation_t desiredRot;
    double curTime;
    double minPointTurnAngle, desiredHeading, angle, factor, sign;
    bool reversable = false;

    if (gPoseCalc == NULL)
{
        return;
}

    curTime = getCurrentEpochTime();
    //setReversableDrive(false);
    getReversableDrive_(reversable);
    getMinimumPointTurnAngle_(minPointTurnAngle);
    getDesiredSpeed_(desiredSpeed);
    getDesiredRotation_(desiredRot);


    // If both desired rotation and desired speed was specified long
    // time ago. Set the control speed (both driving and steering) to
    // be zero. If the desired rotation is requested after desired
    // steering speed is specified, compute the control steering speed
    // based on the rotation.
    if ((desiredRot.extra.time < (curTime - MAX_NO_COMMAND_TIME_SEC_)) &&
        (desiredSpeed.extra.time < (curTime - MAX_NO_COMMAND_TIME_SEC_)))
    {
        desiredSpeed.driveSpeed.value = 0;
        desiredSpeed.steerSpeed.value = 0;
        //printfTextWindow("Parar motores de limpieza \n");
        ((Roomba *)gRobot)->setMotores(0);
    }
    else if (desiredRot.extra.time > (desiredSpeed.steerSpeed.extra.time))
    {
            gPoseCalc->getRotation(currentRot);
            desiredHeading = desiredRot.yaw - currentRot.yaw;
            ROBOT_CRAMPDEG(desiredHeading, -180.0, 180.0);

            if (reversable)
            {
                if (fabs(desiredHeading) > 90.0)
                {
                    desiredHeading += 180.0;
                    desiredSpeed.driveSpeed.value *= -1.0;
                }
            }

            ROBOT_CRAMPDEG(desiredHeading, -180.0, 180.0);

            if (fabs(desiredHeading) >= minPointTurnAngle)
            {
                desiredSpeed.driveSpeed.value = 0;
            }

            // Determine the steering speed based on the sine function of the
            // angle from the desired heading.
            sign = (desiredHeading >= 0)? 1.0 : -1.0;
            angle = fabs(desiredHeading);
            angle = (angle > MAX_STEER_SPEED_ANGLE_)? MAX_STEER_SPEED_ANGLE_ : angle;
            factor = sin(0.5*M_PI*angle/MAX_STEER_SPEED_ANGLE_);
            desiredSpeed.steerSpeed.value = sign * factor* maxSteerSpeed_;

        if (fabs(desiredSpeed.steerSpeed.value) < MIN_STEER_SPEED_)
        {
            if (fabs(angle) >= minPointTurnAngle)
            {
                desiredSpeed.driveSpeed.value = 0;
            printfTextWindow("Warning: Minimum point-turn angle (%f) too small.\n", minPointTurnAngle);
            }
            else
            {
                desiredSpeed.steerSpeed.value = 0.0;
            }
        }
    }
    else
    {
        // Use the desired speed as the control speed.
    }
    pthread_mutex_lock(&controlSpeedMutex_);
    shControlSpeed_ = desiredSpeed;
    pthread_mutex_unlock(&controlSpeedMutex_);
}
