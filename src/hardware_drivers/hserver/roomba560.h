/**********************************************************************
 **                                                                  **
 **                            Roomba560.h                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alberto Rodriguez Valle                            **
 **                                                                  **
 **  Roomba560 robot driver for HServer                              **
 **                                                                  **
 **                                                                  **
 **********************************************************************/

/* $Id: Roomba560.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef ROOMBA560_H
#define ROOMBA560_H


#include <pthread.h>
#include <string>
#include <math.h>
#include "hserver.h"
#include "robot.h"
#include "sensors.h"
#include "robot_config.h"


#define PI 3.14159
//#define T_MUESTREO 25000
#define T_MUESTREO 25000

// ******** CAMBIOS *********
// Giros expresados en grados
#define ROOMBA560_UMBRAL_GIRO  2       // Umbral máximo de incremento del giro
#define ROOMBA560_INCREM_GIRO  0.56    // Incremento del giro cuando supera el umbral
// ******** CAMBIOS *********

#define NORMALIZE(z) atan2(sin(z), cos(z))
#define NUM_SENSORES 10

//El orden de los sensores es el siguiente
// 1-> Bumper Izquierdo         [0]
// 2-> Bumper Derecho           [1]
// 3-> Cliff Izquierdo          [2]
// 4-> Cliff Front Izquierdo    [3]
// 5-> Cliff Front Derecho      [4]
// 6-> Cliff Derecho            [5]
// 7-> Wall                     [6]
// 8-> Whell Drop Izquiedo      [7]
// 9-> Whell Drop Caster        [8]
// 10-> Whell Drop Derecho      [9]


class Roomba560 : public Robot
{
    typedef struct RobotInfo_t {
        byte_t status;
        double xPos;
        double yPos;
        int thetaPos;
        unsigned short battery;
        int sensores[NUM_SENSORES];
        byte_t temperatura;
        byte_t power;
        byte_t spot;
        byte_t clean;
        byte_t max;
    };

protected:

    Sensor *sensorRoomba_;
    RobotPartXyt *partXyt_;
    RobotPartRoomba *partSensores_;
    pthread_t readerThread_;
    pthread_mutex_t serialMutex_;
    char *robotName_;
    char *robotClass_;
    char *subClass_;
    int numSensores_;
    int commFd_;
    bool isReading_, isRangeOn_;
    float valores[NUM_SENSORES];
    float sensorLoc_[NUM_SENSORES][3];
    double battery_;
    FILE * fd;

    static const int CMD_START_;
    static const int CMD_BAUD_;
    static const int CMD_CONTROL_;
    static const int CMD_SAFE_;
    static const int CMD_FULL_;
    static const int CMD_POWER_;
    static const int CMD_SPOT_;
    static const int CMD_CLEAN_;
    static const int CMD_MAX_;
    static const int CMD_DRIVE_;
    static const int CMD_DRIVE_DIRECT_;
    static const int CMD_MOTORS_;
    static const int CMD_SENSORS_;
    static const int CMD_DIGOUT_;
    static const int CMD_LED_;
    static const int CMD_SONG_;
    static const int CMD_PLAY_;

    static const int ANCHURA;

    static const int READ_BACKOFF_USEC_;
    static const int READER_LOOP_USLEEP_;
    static const int SYNCER_LOOP_USLEEP_;

    static const int MAX_VELOCIDAD;
    static const int MIN_VELOCIDAD;
    static const int MAX_ARCO;
    static const int MIN_ARCO;
    static const float CONVERSION;

    static const double ANGLE_CONV_FACTOR_RAD_;
    static const double DIST_CONV_FACTOR_;

    static const int MAX_ENCODER_VALUE_;
    static const int SKIP_STATUSBAR_UPDATE_;

//---------------------------------------------------------------------
//Indices del paquete de datos
//---------------------------------------------------------------------
    static const int BUMPSWHEELDROPS;
    static const int WALL;
    static const int CLIFFLEFT;
    static const int CLIFFFRONTLEFT;
    static const int CLIFFFRONTRIGHT;
    static const int CLIFFRIGHT;
    static const int VIRTUALWALL;
    static const int MOTOROVERCURRENTS;
    static const int DIRTLEFT;
    static const int DIRTRIGHT;
    static const int REMOTEOPCODE;
    static const int BUTTONS;
    static const int DISTANCE_HI;
    static const int DISTANCE_LO;
    static const int ANGLE_HI;
    static const int ANGLE_LO;
    static const int CHARGINGSTATE;
    static const int VOLTAGE_HI;
    static const int VOLTAGE_LO;
    static const int CURRENT_HI;
    static const int CURRENT_LO;
    static const int TEMPERATURE;
    static const int CHARGE_HI;
    static const int CHARGE_LO;
    static const int CAPACITY_HI;
    static const int CAPACITY_LO;
    static const int ENCODER_RIGHT_HI;
    static const int ENCODER_RIGHT_LO;
    static const int ENCODER_LEFT_HI;
    static const int ENCODER_LEFT_LO;
    static const int VEL_LEFT_HI;
    static const int VEL_LEFT_LO;
    static const int VEL_RIGHT_HI;
    static const int VEL_RIGHT_LO;
    static const int VELOCIDAD_HI;
    static const int VELOCIDAD_LO;
    static const int RADIO_HI;
    static const int RADIO_LO;

//---------------------------------------------------------------------
//Mascaras para desentramar los datos
//---------------------------------------------------------------------
    static const int WHEELDROP_MASK;
    static const int BUMP_MASK;
    static const int BUMPRIGHT_MASK;
    static const int BUMPLEFT_MASK;
    static const int WHEELDROPRIGHT_MASK;
    static const int WHEELDROPLEFT_MASK;
    static const int WHEELDROPCENT_MASK;
    static const int MOVERDRIVELEFT_MASK;
    static const int MOVERDRIVERIGHT_MASK;
    static const int MOVERMAINBRUSH_MASK;
    static const int MOVERVACUUM_MASK;
    static const int MOVERSIDEBRUSH_MASK;
    static const int POWERBUTTON_MASK;
    static const int SPOTBUTTON_MASK;
    static const int CLEANBUTTON_MASK;
    static const int MAXBUTTON_MASK;

    static const float SENSORES_ANGLES_[];
    static const double MAX_SENSOR_READABLE_DISTANCE_;

    static const double MIN_ROBOT_VOLTAGE_;

    byte_t nextByte_(void);
    int testRobotConnection_(void);
    int readPack_(void);
    int nextRemainingBytes_(byte_t* bytes, int max);
    int decodePack_(byte_t pack[]);
    void flushSerialLine_(void);
    void decodeStatusPack_(byte_t pack[]);
    void updateRobotInfo_(RobotInfo_t robotInfo);
    void addSensors_(void);
    void readerLoop_(void);
    void updateStatusBar_(void);
    void controlRobot_(void);
    int enviar_(int tam, byte_t pack[], bool bloqueante);
    void compControlSpeed_(void);

    static void *startReaderThread_(void *RoombaInstance);

public:
    Roomba560(Robot** a, const string& strPortString);
    ~Roomba560(void);
    void control(void);
    void initRobot(void);
    void binario(char *, byte_t);
    void setMotores(int );

};

#endif

