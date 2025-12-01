/**********************************************************************
 **                                                                  **
 **                            Roomba560.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alberto Rodriguez Valle                            **
 **                                                                  **
 **  Roomba560 robot driver for HServer                              **
 **                                                                  **

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
#include "roomba560.h"
#include "hserver.h"
#include "camera.h"
#include "statusbar.h"
#include "robot_config.h"
#include "sensors.h"
#include "robot.h"
#include "PoseCalculatorInterface.h"

#define TAM_TRAMA 80
// #define RADIO

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//---------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------
const int Roomba560::CMD_START_ = 128;
const int Roomba560::CMD_BAUD_ = 129;
const int Roomba560::CMD_CONTROL_ = 130;
const int Roomba560::CMD_SAFE_ = 131;
const int Roomba560::CMD_FULL_ = 132;
const int Roomba560::CMD_POWER_ = 133;
const int Roomba560::CMD_SPOT_ = 134;
const int Roomba560::CMD_CLEAN_ = 135;
const int Roomba560::CMD_MAX_ = 136;
const int Roomba560::CMD_DRIVE_ = 137;
const int Roomba560::CMD_MOTORS_ = 138;
const int Roomba560::CMD_SENSORS_ = 142;
const int Roomba560::CMD_LED_ = 139;
const int Roomba560::CMD_SONG_ = 140;
const int Roomba560::CMD_PLAY_ = 141;
const int Roomba560::CMD_DRIVE_DIRECT_ = 145;

const int Roomba560::READ_BACKOFF_USEC_ = 100;
const int Roomba560::READER_LOOP_USLEEP_ = 1000;
const int Roomba560::SKIP_STATUSBAR_UPDATE_ = 10;

const int Roomba560::MAX_VELOCIDAD = 500;
const int Roomba560::MIN_VELOCIDAD = -500;
const int Roomba560::MAX_ARCO = 2000;
const int Roomba560::MIN_ARCO = -2000;

const int Roomba560::ANCHURA = 258;


const int Roomba560::BUMPSWHEELDROPS     = 0;
const int Roomba560::WALL                = 1;
const int Roomba560::CLIFFLEFT           = 2;
const int Roomba560::CLIFFFRONTLEFT      = 3;
const int Roomba560::CLIFFFRONTRIGHT     = 4;
const int Roomba560::CLIFFRIGHT          = 5;
const int Roomba560::VIRTUALWALL         = 6;
const int Roomba560::MOTOROVERCURRENTS   = 7;
const int Roomba560::DIRTLEFT            = 8;
const int Roomba560::DIRTRIGHT           = 9;
const int Roomba560::REMOTEOPCODE        = 10;
const int Roomba560::BUTTONS             = 11;
const int Roomba560::DISTANCE_HI         = 12;
const int Roomba560::DISTANCE_LO         = 13;
const int Roomba560::ANGLE_HI            = 14;
const int Roomba560::ANGLE_LO            = 15;
const int Roomba560::CHARGINGSTATE       = 16;
const int Roomba560::VOLTAGE_HI          = 17;
const int Roomba560::VOLTAGE_LO          = 18;
const int Roomba560::CURRENT_HI          = 19;
const int Roomba560::CURRENT_LO          = 20;
const int Roomba560::TEMPERATURE         = 21;
const int Roomba560::CHARGE_HI           = 22;
const int Roomba560::CHARGE_LO           = 23;
const int Roomba560::CAPACITY_HI         = 24;
const int Roomba560::CAPACITY_LO         = 25;
const int Roomba560::ENCODER_RIGHT_HI    = 52;
const int Roomba560::ENCODER_RIGHT_LO    = 53;
const int Roomba560::ENCODER_LEFT_HI     = 54;
const int Roomba560::ENCODER_LEFT_LO     = 55;
const int Roomba560::VEL_RIGHT_HI        = 48;
const int Roomba560::VEL_RIGHT_LO        = 49;
const int Roomba560::VEL_LEFT_HI         = 50;
const int Roomba560::VEL_LEFT_LO         = 51;
const int Roomba560::VELOCIDAD_HI        = 44;
const int Roomba560::VELOCIDAD_LO        = 45;
const int Roomba560::RADIO_HI            = 46;
const int Roomba560::RADIO_LO            = 47;
const float Roomba560::CONVERSION        = 0.458f;



const int Roomba560::WHEELDROP_MASK      = 0x1C;
const int Roomba560::BUMP_MASK           = 0x03;
const int Roomba560::BUMPRIGHT_MASK      = 0x01;
const int Roomba560::BUMPLEFT_MASK       = 0x02;
const int Roomba560::WHEELDROPRIGHT_MASK = 0x04;
const int Roomba560::WHEELDROPLEFT_MASK  = 0x08;
const int Roomba560::WHEELDROPCENT_MASK  = 0x10;

const int Roomba560::MOVERDRIVELEFT_MASK = 0x10;
const int Roomba560::MOVERDRIVERIGHT_MASK= 0x08;
const int Roomba560::MOVERMAINBRUSH_MASK = 0x04;
const int Roomba560::MOVERVACUUM_MASK    = 0x02;
const int Roomba560::MOVERSIDEBRUSH_MASK = 0x01;

const int Roomba560::POWERBUTTON_MASK    = 0x08;
const int Roomba560::SPOTBUTTON_MASK     = 0x04;
const int Roomba560::CLEANBUTTON_MASK    = 0x02;
const int Roomba560::MAXBUTTON_MASK      = 0x01;



const float Roomba560::SENSORES_ANGLES_[] = {45,-45,15,75,-75,-15,-20,90,0,-90};
const double Roomba560::MAX_SENSOR_READABLE_DISTANCE_ = 5.0;
const double Roomba560::MIN_ROBOT_VOLTAGE_ = 10.8;

//---------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------
Roomba560::Roomba560(Robot** a, const string& strPortString) :
    Robot(a, HS_ROBOT_TYPE_ROOMBA560, HS_MODULE_NAME_ROBOT_ROOMBA560)
{
    partXyt_ = NULL;
    partSensores_ = NULL;
    sensorRoomba_ = NULL;
    robotName_ = NULL;
    isReading_ = false;
    numSensores_ = NUM_SENSORES;

    pthread_mutex_init(&serialMutex_, NULL);

    if ((report_level > HS_REP_LEV_NONE) && (report_level != HS_REP_LEV_DEBUG))
    {
        printTextWindow("Connecting to Roomba560... ", false);
    }

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Connecting to Roomba560");
        printfTextWindow("robot serial port: %s", strPortString.c_str());
    }

     if(setupSerial(&commFd_, strPortString.c_str(), B57600, report_level, "Robot", 1) == -1){
        printTextWindow("Sin conexion\n");
        exit(0);
    }

    aspirar = true;

    redrawWindows();
    updateStatusBar_();
    redrawWindows();

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
        printTextWindow("Roomba560 connected", true);
    }

    pthread_create(&readerThread_, NULL, &startReaderThread_,(void*)this);

    addSensors_();

}

//---------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------
Roomba560::~Roomba560(void)
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

    printTextWindow("Roomba560 disconnected");

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
// This function flushes the buffer in the serial port.
//---------------------------------------------------------------------
void Roomba560::flushSerialLine_(void)
{
    byte_t buf[80];
    int cnum;

    do
    {
        cnum=read(commFd_, buf, 80);
        if (cnum == -1)
        {
            cnum = 0;
        }
    } while(cnum);
}


//---------------------------------------------------------------------
// This function reads the next remaining bytes from the fd
//---------------------------------------------------------------------
int Roomba560::nextRemainingBytes_(byte_t *bytes, int max)
{
    int cnum;
    int numTries = 25;

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
// This function updates the status bar
//---------------------------------------------------------------------
void Roomba560::updateStatusBar_(void)
{
    HSPose_t pose;
    char buf[100];

    statusStr[0] = 0;

    strcat(statusStr, "Roomba560: ");

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
// Envio de datos a través del puerto serie
//---------------------------------------------------------------------
int Roomba560::enviar_(int tam, byte_t pack[], bool bloqueante)
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
// This function updates the robot info.
//---------------------------------------------------------------------
void Roomba560::updateRobotInfo_(RobotInfo_t robotInfo)
{
    HSPose_t pose;
    HSPose_t deltaPose;
    double tdif;
    static RobotInfo_t oldRobotInfo;
    double xdif, ydif;
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

    // Resetea el robot
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

void Roomba560::binario(char * cadena, byte_t num){
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
// This function decodes the status packet.
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
void Roomba560::decodeStatusPack_(byte_t pack[])
{
    RobotInfo_t robotInfo;
    int anguloGrados;
    float distancia;
    static bool initialized = 0;
    static double anguloTotal = 0.0, distanciaTotal = 0.0;
    static double yPosAnt = 0.0, xPosAnt = 0.0;
    double angulo;
    static double anguloAnt = 0.0;
    double xInc, yInc;
    unsigned short encoderDer, encoderIzq;
    static unsigned short encoderDerAnt, encoderIzqAnt;
    short velDer, velIzq;
    static int sentidoIzq = 0, sentidoDer = 0;
    short velocidad, radio;
    int sumaIzq, sumaDer;

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

    robotInfo.max = 0;
    robotInfo.clean = 0;
    robotInfo.spot = 0;
    robotInfo.power = 0;

    encoderIzq = pack[ENCODER_RIGHT_HI];
    encoderIzq = (encoderIzq << 8) + pack[ENCODER_RIGHT_LO];

    encoderDer = pack[ENCODER_LEFT_HI];
    encoderDer = (encoderDer << 8) + pack[ENCODER_LEFT_LO];

    velIzq = pack[VEL_LEFT_HI];
    velIzq = (velIzq << 8) + pack[VEL_LEFT_LO];

    velDer = pack[VEL_RIGHT_HI];
    velDer= (velDer << 8) + pack[VEL_RIGHT_LO];

    velocidad = pack[VELOCIDAD_HI];
    velocidad = (velocidad << 8) + pack[VELOCIDAD_LO];

    radio = pack[RADIO_HI];
    radio = (radio << 8) + pack[RADIO_LO];


    if (!initialized)
    {
        robotInfo.xPos = 0;
        robotInfo.yPos = 0;
        robotInfo.thetaPos = 0;
        initialized = true;
        encoderIzqAnt = encoderIzq;
        encoderDerAnt = encoderDer;
        return;
    }

    if(velIzq > 0){
        sentidoIzq = 0;
        sumaIzq = encoderIzq - encoderIzqAnt;
        if(sumaIzq < 0)
            sumaIzq = encoderIzq + 65535 - encoderIzqAnt;
    }else if(velIzq < 0){
        sentidoIzq = 1;
        sumaIzq = encoderIzqAnt - encoderIzq;
        if(sumaIzq > 0)
            sumaIzq = encoderIzq - 65535 - encoderIzqAnt;
    }else{
        if(sentidoIzq == 0){
            sumaIzq = encoderIzq - encoderIzqAnt;
            if(sumaIzq < 0)
                sumaIzq = encoderIzq + 65535 - encoderIzqAnt;
        }
        if(sentidoIzq == 1){
            sumaIzq = encoderIzqAnt - encoderIzq;
            if(sumaIzq > 0)
                sumaIzq = encoderIzq - 65535 - encoderIzqAnt;
        }
    }

    if(velDer > 0){
        sentidoDer = 0;
        sumaDer = encoderDer - encoderDerAnt;
        if(sumaDer < 0)
            sumaDer = encoderIzq + 65535 - encoderDerAnt;
    }else if(velDer < 0){
        sentidoDer = 1;
        sumaDer = encoderDerAnt - encoderDer;
        if(sumaDer > 0)
            sumaDer = encoderDer - 65535 - encoderDerAnt;
    }else{
        if(sentidoDer == 0){
            sumaDer = encoderDer - encoderDerAnt;
            if(sumaDer < 0)
                sumaDer = encoderDer + 65535 - encoderDerAnt;
        }
        if(sentidoDer == 1){
            sumaDer = encoderDerAnt - encoderDer;
            if(sumaDer > 0)
                sumaDer = encoderDer - 65535 - encoderDerAnt;
        }
    }


    distancia = (sumaIzq + sumaDer) / 2 *  CONVERSION;

    angulo = ((float)(sumaDer - sumaIzq))* CONVERSION/ANCHURA;
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


    encoderIzqAnt = encoderIzq;
    encoderDerAnt = encoderDer;

    updateRobotInfo_(robotInfo);
    updateStatusBar_();
}


//-----------------------------------------------------------------------
// This function controls the robot with the control speed.
//-----------------------------------------------------------------------
void Roomba560::controlRobot_(void)
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
    short velocidadIzq, velocidadDer;

    getControlSpeed_(controlSpeed);

    velocidad = (ROBOT_M2MM(controlSpeed.driveSpeed.value));
    vGiro = (controlSpeed.steerSpeed.value);

     giro=vGiro*PI/180.0;

    giroAnt = giro;
    velAnt = velocidad;
    // ******** CAMBIOS *********

    if(velocidad > MAX_VELOCIDAD){
        float factor = velocidad / MAX_VELOCIDAD;
        giro = giro / factor;
        velocidad = MAX_VELOCIDAD;
    }

    velocidadDer = (short) ((ANCHURA * giro / 2 ) + velocidad);
    velocidadIzq = (short) (2 * velocidad - velocidadDer);

    if(inicializar != 0){
        if((velocidad != velocidadAnt) || (vGiro != vGiroAnt)){

            pack[0] = CMD_DRIVE_DIRECT_;
            pack[1] = (byte_t)((velocidadDer >> 8) & macro);
            pack[2] = (byte_t)(velocidadDer & macro);
            pack[3] = (byte_t)((velocidadIzq >> 8) & macro);
            pack[4] = (byte_t)(velocidadIzq & macro);

            printfTextWindow("[%d|%d]*[%d|%d]\n", pack[1], pack[2], pack[3], pack[4]);

            enviar_(5, pack, true);

            velocidadAnt = velocidad;
            vGiroAnt = vGiro;
        }
    }else{

        pack[0] = CMD_DRIVE_DIRECT_;
        pack[1] = (byte_t)((velocidadDer >> 8) & macro);
        pack[2] = (byte_t)(velocidadDer & macro);
        pack[3] = (byte_t)((velocidadIzq >> 8) & macro);
        pack[4] = (byte_t)(velocidadIzq & macro);

        printfTextWindow("[%d|%d]*[%d|%d]\n", pack[1], pack[2], pack[3], pack[4]);

        enviar_(5, pack, true);

        velocidadAnt = velocidad;
        vGiroAnt = vGiro;
        inicializar = 1;
    }
}


//---------------------------------------------------------------------
// This function intialize the x, y, and theta
//---------------------------------------------------------------------
void Roomba560::initRobot(void)
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
// This function reads a packet from the port.
//---------------------------------------------------------------------
int Roomba560::readPack_(void)
{
    int n, len, numBytes;
    byte_t pack[TAM_TRAMA];

    // ******** CAMBIOS *********
    // Se bloquea el mutex
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &serialMutex_);
    pthread_mutex_lock(&serialMutex_);

    pack[0] = 142;
    pack[1] = 100;
    if(enviar_(2, pack,false)<0)
        printTextWindow("ERROR en el envio\n");

    usleep(T_MUESTREO);

    len = TAM_TRAMA;
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
void Roomba560::readerLoop_(void)
{
    int count = 0;
    short arc, vel;
    byte_t pack[2];

    isReading_ = true;


    while(true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Read the data from the robot.

        int dev = readPack_();

        // Compute the control speed.
        compControlSpeed_();

        // Control the robot.
        controlRobot_();


        // Check for the pthread termination again.
        pthread_testcancel();

        // Compute the frequency.
        compFrequency_();

        // Compute the velocity.
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
// This function configure the sensors
//---------------------------------------------------------------------
void Roomba560::addSensors_(void)
{

    partXyt_ = new RobotPartXyt();

    partSensores_ = new RobotPartRoomba(numSensores_, (float *)SENSORES_ANGLES_);

    sensorRoomba_ = new Sensor(SENSOR_ROOMBA, sizeof(sensorLoc_), (char*)sensorLoc_, numSensores_, (float *)SENSORES_ANGLES_, numSensores_, valores);

}


//---------------------------------------------------------------------
// This function sends commands to the robot based on the user input
//---------------------------------------------------------------------
void Roomba560::control()
{
    int c;
    int done = 0;
    int msgData[3];

    msgData[0] = isRangeOn_;
    msgData[1] = 0; // "speed_factor" is obsolete.
    msgData[2] = 0; // "angular_speed_factor" is obsolete.
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
            isRangeOn_ = !isRangeOn_;
            msgData[0] = isRangeOn_;
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

//---------------------------------------------------------------------
// This function starts the reader thread
//---------------------------------------------------------------------
void *Roomba560::startReaderThread_(void *Roomba560Instance)
{
    ((Roomba560*)Roomba560Instance)->readerLoop_();
    return NULL;
}

//---------------------------------------------------------------------
//Esta función activa los motores de limpieza
//---------------------------------------------------------------------
void Roomba560::setMotores(int valor)
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
void Roomba560::compControlSpeed_(void)
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
        ((Roomba560 *)gRobot)->setMotores(0);
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

