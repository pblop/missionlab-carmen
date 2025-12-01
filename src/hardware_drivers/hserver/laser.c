/**********************************************************************
 **                                                                  **
 **                                laser.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  SICK LMS 200 driver for HServer                                 **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: laser.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

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
#include <getopt.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "HServerTypes.h"
#include "laser.h"
#include "message.h"
#include "hserver.h"
#include "statusbar.h"
#include "robot_config.h"
#include "sensors.h"
#include "robot.h"
#include "hserver_ipt.h"
#include "PoseCalculatorInterface.h"
#include "ipt/callbacks.h"
#include "ipt/connection.h"
#include "ipt_handler.h"
#include "Watchdog.h"
#include "LogManager.h"

using namespace std;

declareHandlerCallback(Laser)
implementHandlerCallback(Laser)

// 732 is the length of a laser data package, everything else is shorter
const int Laser::MAX_TELEGRAM_SIZE = 732;
const int Laser::WATCHDOG_CHECK_INTERVAL_SEC_ = 4;

void *start_carmenLaserTread(void * laser_instance)
{
    ((Laser*) laser_instance)->carmenLaserReaderThread();
	return NULL;
}

void laserCARMENHandler(MSG_INSTANCE msgInstance, void *callData, void *clientData)
{
	static FORMATTER_PTR formatter = IPC_msgInstanceFormatter(msgInstance);
	((Laser *)clientData)->callbackCARMENLaserScan((carmen_laser_laser_message *)callData);
	IPC_freeData(formatter, callData);
}

unsigned char sick_config_tgm[] =
{
    0x00, 0x00,  // Blanking
    0x46, 0x00,  // Stop threshold
    0x00,        // Fog
    0x00,        // meas. value flag
    0x00,        // units
    0x00,        // temp field
    0x00,        // Fields A&B subt
    0x02,        // Multiple evaluation
    0x02,        // Restart
    0x01,        // Restart time
    0x00,        // Restart test
    0x00,        // Cont A
    0x0a,
    0x0a,
    0x50,
    0x64,
    0x00,        // Cont B
    0x0a,
    0x0a,
    0x50,
    0x64,
    0x00,        // Cont C
    0x0a,
    0x0a,
    0x50,
    0x64,
    0x00,        // Pixel-oriented eval data
    0x00,        // Single meas eval
    0x00,
    0x00
};

int sick_config_tgm_size = 32;

char sick_password[] = "SICK_LMS";

// instantiation and initialization of static member variables
float* Laser::m_pfAllReadings = NULL;
float* Laser::m_pfAllAngles = NULL;
unsigned int* Laser::m_puAllRawRanges = NULL;

int Laser::m_iTotalReadings = 0;
int Laser::m_iOverallMinReading = -1;

float Laser::m_fAngleOfOverallMinReading = -1.0;

pthread_mutex_t Laser::m_mutexRanges    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Laser::m_mutexAngles    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Laser::m_mutexRawRanges = PTHREAD_MUTEX_INITIALIZER;

Sensor* Laser::m_pclSensorLaser = NULL;
RobotPartLaser* Laser::m_pclPartLaser = NULL;

int Laser::m_bFirsttime = true;

string Laser::m_strStreamAddress = "212.128.171.130";
vector<Laser*> Laser::m_clLaserList;
const string Laser::m_strDEFAULT_NAME = "toro";

int  Laser::m_iStreamFd  = -1;
bool Laser::m_bStreaming = false;

void Laser::RecalculateMinReading(void)
{
    if (m_clLaserList.size() > 0)
    {
        Laser* pclLaser = m_clLaserList[0];
        m_iOverallMinReading = pclLaser->m_iMinReading;
        m_fAngleOfOverallMinReading = pclLaser->IndexToDegrees(pclLaser->m_iMinIndex);

        for (unsigned int i = 1; i < m_clLaserList.size(); i++)
        {
            pclLaser = m_clLaserList[i];
            if (pclLaser->m_iMinReading < m_iOverallMinReading)
            {
                m_iOverallMinReading = pclLaser->m_iMinReading;
                m_fAngleOfOverallMinReading = pclLaser->IndexToDegrees(pclLaser->m_iMinIndex);
            }
        }
    }
}

void Laser::updateStatusBar_(void)
{
    char buf[100];
    statusStr[0] = 0;

    strcat(statusStr, "Laser: ");

    sprintf(buf, "%c ", statusbarSpinner_->getStatus());
    strcat(statusStr, buf);

    if (!m_bConnected)
    {
        if (!(m_bReceiveIpt || m_bReceiveCarmen))
        {
            sprintf(buf, "Connecting  time left: %d", m_iMaxRetries);
        }
        else
        {
            sprintf(buf, "Waiting for connection");
        }
        strcat(statusStr, buf);
    }
    else
    {
        if (m_bReceiveIpt || m_bReceiveCarmen)
        {
            strcat(statusStr, " ipt");
        }
        if (m_bStreamIpt)
        {
            strcat(statusStr, " ipt connected ");
        }

        //        sprintf(buf, " %.1f up/sec min %d", m_fUpdateSpeed, m_iOverallMinReading);
        // kaess 020715: want to see min reading of each laser separately
        sprintf(buf, " %.1f up/sec min %d", m_fUpdateSpeed, m_iMinReading);

        strcat(statusStr, buf);
    }

    statusbar->update(statusLine);
}


void Laser::connectRemote(void)
{
    printTextWindow("Laser Got connection from laser relay");
    iptHandler->set_state(IPT_MOD_LASER, true, true);
    m_bConnected = true;
}

int Laser::setupIpt(void)
{
    bool done = false;
    REC_ModuleId msg;
    REC_ModuleId* msg_rply;

    if (iptHandler == NULL)
    {
        printTextWindow("No ipt handler, laser messages not registered");
        return -1;
    }
    else
    {
        m_pclCommunicator = iptHandler->communicator();
        iptHandler->registerMessages(iptLaserMessageArray);
        if (m_bReceiveIpt)
        {
            m_pclIptCallback = new HandlerCallback(Laser)(this, &Laser::callbackLaserScan);
            IPMsgCallbackHandlerSpec hndArray[] = {
                { MSG_LaserScan, m_pclIptCallback, IPT_HNDL_STD }
            };
            iptHandler->registerCallbackHandlers(1, hndArray);
            iptHandler->set_state(IPT_MOD_LASER, true, false);
        }
        else if (m_bReceiveCarmen)
        {
        	/*m_pclIptCallback = new HandlerCallback(Laser)(this, &Laser::callbackLaserScan);
        	IPMsgCallbackHandlerSpec hndArray[] = {
        			{ CARMEN_LASER_FRONTLASER_NAME, m_pclIptCallback, IPT_HNDL_STD }
        	};
        	iptHandler->registerCallbackHandlers(1, hndArray);
        	iptHandler->set_state(IPT_MOD_LASER, true, false);*/
        	pthread_create(&m_CarmenReaderThread, NULL, start_carmenLaserTread, (void *)this);
        }
        else if (m_bStreamIpt)
        {
            do
            {
                updateStatusBar_();
                m_pclLaserConnection = m_pclCommunicator->Connect("fred:control", IPT_OPTIONAL);
                if (m_pclLaserConnection->FD() > 0)
                {
                    done = true;
                }
                else
                {
                    if (m_iMaxRetries-- <= 0)
                    {
                        return -1;
                    }
                    sleep(1);
                }
            } while (!done);
            msg.id = MODULE_ID_LASER;
            IPMessage* pmsg = m_pclCommunicator->Query(m_pclLaserConnection, MSG_ModuleId,
                                                        &msg, MSG_ModuleId, 4);
            if (pmsg == NULL)
            {
                return -1;
            }
            msg_rply = (REC_ModuleId*) pmsg->FormattedData();
            if (msg_rply->id == MODULE_NOT_CONNECTED)
            {
                printTextWindow("Laser not running on other hserver");
                return -1;
            }
            delete pmsg;
            m_bConnected = true;
        }
    }

    return 0;
}

void Laser::AccountForXYOffset(float& fRange, float& fReadingAngle)
{
    fReadingAngle = fReadingAngle * M_PI / 180.0; // convert to radians

    // get X and Y components of the reading
    float fReadingXOffset = fRange * cos(fReadingAngle);
    float fReadingYOffset = fRange * sin(fReadingAngle);

    fReadingXOffset += m_fXOffset;
    fReadingYOffset += m_fYOffset;

    fRange = sqrt(fReadingXOffset * fReadingXOffset + fReadingYOffset * fReadingYOffset);

    fReadingAngle = (fReadingXOffset == 0.0) ?
        (M_PI / 2.0) : atan(fReadingYOffset / fReadingXOffset);

    // quadrants 2 and 3
    if (fReadingXOffset < 0.0)
    {
        fReadingAngle += M_PI;
    }

    // convert back to degrees and normalize
    fReadingAngle = NormalizeAngle(fReadingAngle * 180.0 / M_PI);
}

void Laser::callbackCARMENLaserScan(carmen_laser_laser_message* rec)
{

	if(m_bReceiveCarmen)
		{
			if(!m_bConnected)
			{
	            printfTextWindow("Laser config: StartAngle: %g  FoView: %g  Resolution: %g  Max: %g  Accuracy: %g RemMode: %d  NumReads: %d  NumRem: %d",
	            		rec->config.start_angle, rec->config.fov,
	            		rec->config.angular_resolution,
	            		rec->config.maximum_range, rec->config.accuracy,
	            		rec->config.remission_mode, rec->num_readings,
	            		rec->num_remissions);
				connectRemote();
			}

			// aquire the lock for the readings -- this could cause deadlock if
			// something tries to aquire both of these locks in an order opposite of what
			// is here, but I don't know how to work around that. Just make sure that if
			// these two locks are aquired at the same time, it's in this order -- and
			// it wouldn't hurt to repeat this comment wherever that happens
			pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &m_mutexRanges);
			pthread_mutex_lock(&m_mutexRanges);

			pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &m_mutexRawRanges);
			pthread_mutex_lock(&m_mutexRawRanges);


			float* pfCurrent = &(m_pfSickRanges[m_iCurrentBuffer * m_iNumReadings]);
			m_iMinReading = rec->config.maximum_range;
			for(int i = 0; i < rec->num_readings; i++)
			{
				*pfCurrent = rec->range[i];

				if((*pfCurrent)<m_iMinReading)
				{
					m_iMinReading = (*pfCurrent);
					m_iMinIndex = i;
				}

				m_pfCurrentSickRanges[i] = *pfCurrent;
				m_pfAngles[i] = m_pfRawAngles[i];
				AccountForXYOffset(m_pfCurrentSickRanges[i], m_pfAngles[i]);
				pfCurrent++;
			}

			pthread_cleanup_pop(1);
			pthread_cleanup_pop(1);

			RecalculateMinReading();
			m_suExtraInfo.bufnum = m_iCurrentBuffer;
			if (callback != NULL)
			{
				callback(&m_pfSickRanges[m_iCurrentBuffer * m_iNumReadings], m_iCurrentBuffer);
			}
			pthread_cond_broadcast(&m_condReader);
			updateStatusBar_();

			// increment the current buffer with wrap-around
			m_iCurrentBuffer = (m_iCurrentBuffer + 1) % m_iNumBuffers;

		}
}


void Laser::callbackLaserScan(IPMessage* message)
{
	if(m_bReceiveIpt)
	{
		REC_LaserScan* rec = (REC_LaserScan*) message->FormattedData();

		// aquire the lock for the readings -- this could cause deadlock if
		// something tries to aquire both of these locks in an order opposite of what
		// is here, but I don't know how to work around that. Just make sure that if
		// these two locks are aquired at the same time, it's in this order -- and
		// it wouldn't hurt to repeat this comment wherever that happens
		pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &m_mutexRanges);
		pthread_mutex_lock(&m_mutexRanges);

		pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &m_mutexRawRanges);
		pthread_mutex_lock(&m_mutexRawRanges);

		float* pfCurrent = &(m_pfSickRanges[m_iCurrentBuffer * m_iNumReadings]);
		for(int i = 0; i < rec->size; i++)
		{
			*pfCurrent = float(rec->scan[i] / 100.0);
			m_pfCurrentSickRanges[i] = *pfCurrent;
			m_pfAngles[i] = m_pfRawAngles[i];
			AccountForXYOffset(m_pfCurrentSickRanges[i], m_pfAngles[i]);
			pfCurrent++;
		}

		pthread_cleanup_pop(1);
		pthread_cleanup_pop(1);

		m_iMinIndex = rec->min;
		m_iMinReading = rec->scan[m_iMinIndex];
		RecalculateMinReading();
		m_suExtraInfo.bufnum = m_iCurrentBuffer;
		if (callback != NULL)
		{
			callback(&m_pfSickRanges[m_iCurrentBuffer * m_iNumReadings], m_iCurrentBuffer);
		}
		pthread_cond_broadcast(&m_condReader);
		updateStatusBar_();

		// increment the current buffer with wrap-around
		m_iCurrentBuffer = (m_iCurrentBuffer + 1) % m_iNumBuffers;
	}
}

void Laser::shutdownIpt()
{
    char *hndNamesIpt[] = {
        MSG_LaserScan
    };

    char *hndNamesCarmen[] = {
    	CARMEN_LASER_FRONTLASER_NAME
    };

    if (m_bListenIpt)
    {
    	if(m_bReceiveIpt)
    		iptHandler->disableHandlers(1, hndNamesIpt);
    	else if(m_bReceiveCarmen)
    		iptHandler->disableHandlers(1, hndNamesCarmen);
    }
}

void Laser::setCallback(void (*callback)(float *,int))
{
     this->callback = callback;
}

#define CRC16_GEN_POL 0x8005
#define MKSHORT(a,b) ((unsigned short)(a)|((unsigned short)(b)<<8))
uint Laser::createCRC(unsigned char* CommData, uint uLen)
{
    unsigned short uCrc16;
    unsigned char abData[2];

    uCrc16 = 0;
    abData[0] = 0;
    while (uLen--)
    {
        abData[1] = abData[0];
        abData[0] = *CommData++;
        if(uCrc16 & 0x8000)
        {
            uCrc16 = (uCrc16 & 0x7fff) << 1;
            uCrc16 ^= CRC16_GEN_POL;
        }
        else
        {
            uCrc16 <<= 1;
        }
        uCrc16 ^= MKSHORT(abData[0],abData[1]);
    }

    return uCrc16;
}

// Open up internet socket for streaming
int Laser::connectISocket(const char* addrstr, int port)
{
    struct sockaddr_in sin;
    struct hostent* hent;
    int sockfd;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    if (isdigit(*addrstr))
    {
        sin.sin_addr.s_addr = inet_addr(addrstr);
    }
    else
    {
        if((hent = gethostbyname(addrstr)) == NULL)
        {
            printTextWindow("laser error: Unknown host.\n");
            return -1;
        }
        memcpy((char*) &sin.sin_addr, hent->h_addr, sizeof(sin.sin_addr)); // as returned by inet_addr
    }
    bzero(&(sin.sin_zero), 8);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printTextWindow("laser error: Socket error.");
        return -1;
    }

    if (connect(sockfd, (sockaddr*) &sin, sizeof(sin)))
    {
        close(sockfd);
        switch(errno)
        {
        case ECONNREFUSED:
            printTextWindow("laser error: Connection refused.\n");
            break;
        case ENETUNREACH:
            printTextWindow("laser error: The network is not reachable from this host.\n");
            break;
        default:
            printTextWindow("laser error: connect error.\n");
        }
        return -1;
    }

    printTextWindow("laser isocket connected.");

    return sockfd;
}

 /* Start streaming */
void Laser::streamStart()
{
    printfTextWindow("Attempting to start laser streaming to %s port %d",
                      m_strStreamAddress.c_str(), LASER_STREAMING_PORT);

    m_iStreamFd = connectISocket(m_strStreamAddress.c_str(), LASER_STREAMING_PORT);
    if (m_iStreamFd == -1)
    {
        printTextWindow("Failed streaming laser data");
    }
    else
    {
        m_bStreaming = true;
        printTextWindow("Streaming laser data");
    }
}

string Laser::commandName(byte_t command)
{
    // jbl    static char buf[30];
    char* szCommand = "";

    switch (command)
    {
    case SICK_STX:
        szCommand = "STX";
        break;
    case SICK_ACK:
        szCommand = "ACK";
        break;
    case SICK_NACK_:
        szCommand = "NACK_";
        break;
    case SICK_INIT:
        szCommand = "INIT";
        break;
    case SICK_BM:
        szCommand = "BM";
        break;
    case SICK_MWANF:
        szCommand = "MWANF";
        break;
    case SICK_SSANF:
        szCommand = "SSANF";
        break;
    case SICK_ERRANF:
        szCommand = "ERRANF";
        break;
    case SICK_MMWANF:
        szCommand = "MMWANF";
        break;
    case SICK_MWPANF:
        szCommand = "MWPANF";
        break;
    case SICK_LMIKFGANF:
        szCommand = "LMIKFGANF";
        break;
    case SICK_EVCANF:
        szCommand = "EVCANF";
        break;
    case SICK_TYPANF:
        szCommand = "TYPANF";
        break;
    case SICK_VARDEF:
        szCommand = "VARDEF";
        break;
    case SICK_MMWPANF:
        szCommand = "MMWPANF";
        break;
    case SICK_FIELDKFG:
        szCommand = "FIELDKFG";
        break;
    case SICK_FIELDCHG:
        szCommand = "FIELDCHG";
        break;
    case SICK_SFANF:
        szCommand = "SFANF";
        break;
    case SICK_LMOD:
        szCommand = "LMOD";
        break;
    case SICK_DYNRECKFG:
        szCommand = "DYNRECKFG";
        break;
    case SICK_DYNSEGKFG:
        szCommand = "DYNSEGKFG";
        break;
    case SICK_SIMINDEX:
        szCommand = "SIMINDEX";
        break;
    case SICK_OUTPUTSET:
        szCommand = "OUTPUTSET";
        break;
    case SICK_INPUTRD:
        szCommand = "INPUTRD";
        break;
    case SICK_SIMINPUT:
        szCommand = "SIMINPUT";
        break;
    case SICK_BRPERMDEF:
        szCommand = "BRPERMDEF";
        break;
    case SICK_LASER:
        szCommand = "LASER";
        break;
    case SICK_LMIKFGDEF:
        szCommand = "LMIKFGDEF";
        break;
    case SICK_EVCDEF:
        szCommand = "EVCDEF";
        break;
    case SICK_LMSKFGANF:
        szCommand = "LMSKFGANF";
        break;
    case SICK_MWXYANF:
        szCommand = "MWXYANF";
        break;
    case SICK_LMSKFGDEF:
        szCommand = "LMSKFGDEF";
        break;
    case SICK_CORRDEF:
        szCommand = "CORRDEF";
        break;
    case SICK_PWON:
        szCommand = "PWON";
        break;
    case SICK_INIT_ACK:
        szCommand = "INIT_ACK";
        break;
    case SICK_NACK:
        szCommand = "NACK";
        break;
    case SICK_BMACK:
        szCommand = "BMACK";
        break;
    case SICK_MW:
        szCommand = "MW";
        break;
    case SICK_SS:
        szCommand = "SS";
        break;
    case SICK_ERR:
        szCommand = "ERR";
        break;
    case SICK_MMW:
        szCommand = "MMW";
        break;
    case SICK_MWP:
        szCommand = "MWP";
        break;
    case SICK_LMIKFG:
        szCommand = "LMIKFG";
        break;
    case SICK_EVC:
        szCommand = "EVC";
        break;
    case SICK_TYPE:
        szCommand = "TYPE";
        break;
    case SICK_VARDEFACK:
        szCommand = "VARDEFACK";
        break;
    case SICK_MMWP:
        szCommand = "MMWP";
        break;
    case SICK_FIELDACK:
        szCommand = "FIELDACK";
        break;
    case SICK_FILEDCHGACK:
        szCommand = "FILEDCHGACK";
        break;
    case SICK_FIELDDAT:
        szCommand = "FIELDDAT";
        break;
    case SICK_LMODACK:
        szCommand = "LMODACK";
        break;
    case SICK_DYNRECACK:
        szCommand = "DYNRECACK";
        break;
    case SICK_DYNSEGACK:
        szCommand = "DYNSEGACK";
        break;
    case SICK_SIMINDEXACK:
        szCommand = "SIMINDEXACK";
        break;
    case SICK_OUTPUTSETACK:
        szCommand = "OUTPUTSETACK";
        break;
    case SICK_INPUTDATA:
        szCommand = "INPUTDATA";
        break;
    case SICK_SIMINPUTACK:
        szCommand = "SIMINPUTACK";
        break;
    case SICK_BRPERMACK:
        szCommand = "BRPERMACK";
        break;
    case SICK_LASERACK:
        szCommand = "LASERACK";
        break;
    case SICK_LMIKFGACK:
        szCommand = "LMIKFGACK";
        break;
    case SICK_EVCACK:
        szCommand = "EVCACK";
        break;
    case SICK_MWXY:
        szCommand = "MWXY";
        break;
    case SICK_LMSKFGAC:
        szCommand = "LMSKFGAC";
        break;
    default:
        char buf[30];
        sprintf(buf, "Telegram unknown %x", command);
        szCommand = buf;
        break;
    }

    return string(szCommand);
}

int Laser::initializeTelegram(telegram_t* tgm, uint command, uint len)
{
    int dataLen;

    dataLen = len - 4;
    tgm->len = len + 2;
    tgm->d = new byte_t[tgm->len];
    tgm->d[STGM_STX ] = 0x02;
    tgm->d[STGM_ADR ] = 0;
    tgm->d[STGM_LENL] = dataLen & 0x00ff;
    tgm->d[STGM_LENH] = dataLen >> 8;
    tgm->d[STGM_CMD ] = command;

    return 0;
}

int Laser::sendCommand(int command, uint arg)
{
    telegram_t tgm;
    uint crc;
    int i, index, passlen, count;
    byte_t c;

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("SICK Send command: %s", commandName(command).c_str()); // jbl
    }

    tgm.d = NULL;

    switch (command)
    {
    case SICK_MWANF:
        initializeTelegram(&tgm, SICK_MWANF, SICK_MWANF_SIZE);
        tgm.d[STGM_DATA    ] = 0; // LMS No.
        tgm.d[STGM_DATA + 1] = 1; // MODE
        crc = createCRC(&(tgm.d[STGM_STX]), SICK_MWANF_SIZE);
        tgm.d[STGM_DATA + 2] = crc & 0xff;
        tgm.d[STGM_DATA + 3] = crc >> 8;
        break;
    case SICK_INIT:
    case SICK_SSANF:
    case SICK_LMSKFGANF:
    case SICK_ERRANF:
        initializeTelegram(&tgm, command, SICK_INIT_SIZE);
        crc = createCRC(&(tgm.d[STGM_STX]), SICK_INIT_SIZE);
        tgm.d[STGM_DATA    ] = crc & 0xff;
        tgm.d[STGM_DATA + 1] = crc >> 8;
        break;
    case SICK_BM:
        index = 1;
        if (arg == SMOD_INSTALLATION)
        {
            passlen = strlen(sick_password);
            initializeTelegram(&tgm, SICK_BM, SICK_BM_SIZE + passlen);
            tgm.d[STGM_DATA] = arg;
            memcpy((char*) &(tgm.d[STGM_DATA + 1]), sick_password, passlen);
            index = 1 + passlen;
            crc = createCRC(&(tgm.d[STGM_STX]), SICK_BM_SIZE + passlen);
        }
        else
        {
            initializeTelegram(&tgm, SICK_BM, SICK_BM_SIZE);
            tgm.d[STGM_DATA] = arg;
            crc = createCRC(&(tgm.d[STGM_STX]), SICK_BM_SIZE);
        }
        tgm.d[STGM_DATA + index    ] = crc & 0xff;
        tgm.d[STGM_DATA + index + 1] = crc >> 8;
        break;
    case SICK_LMSKFGDEF:
        initializeTelegram(&tgm, SICK_LMSKFGDEF, SICK_LMSKFGDEF_SIZE);
        for(i = 0; i < sick_config_tgm_size; i++)
        {
            tgm.d[STGM_DATA + i] = sick_config_tgm[i];
        }
        crc = createCRC(&(tgm.d[STGM_STX]), SICK_LMSKFGDEF_SIZE);
        tgm.d[STGM_DATA + sick_config_tgm_size    ] = crc & 0xff;
        tgm.d[STGM_DATA + sick_config_tgm_size + 1] = crc >> 8;
        break;
    default:
        printfTextWindow("SICK Error: Unknown command: %d", command);
        break;
    }
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &m_mutexSerial);
    pthread_mutex_lock(&m_mutexSerial);
    write(m_iSerialPortFd, tgm.d, tgm.len);
    count = read(m_iSerialPortFd, &c, 1);
    pthread_cleanup_pop(1);

    if (tgm.d != NULL)
    {
        delete [] tgm.d;
    }

    return ((c == SICK_ACK) ? 0 : -1);
}

void Laser::printTelegram(byte_t* buf, int len)
{
    int i, c;
    char tbuf[20];
    return; // I'm not sure why this is here now. I think it's because this function is
            // only used for debuging

    printfTextWindow("SICK Telegram: %s", commandName(buf[STGM_CMD]).c_str()); // jbl
    printfTextWindow("ADR: %x LEN: %d\n", buf[STGM_ADR], len);
    rstr[0] = 0;
    for(i = 0; i < len; i++)
    {
        c++;
        sprintf(tbuf, "%2x ", buf[i]);
        strcat(rstr, tbuf);
        if (c == 20)
        {
            printTextWindow(rstr);
            c = 0;
            rstr[0] = 0;
        }
    }
    printTextWindow(rstr);
}

 /* Stream to socket */
void Laser::sendLaserStream()
{

    // note -- Right now the only program that uses streaming is laser_fit, which makes
    // the assumption that there's only one laser facing forward, so this will only stream
    // the first 361 raw readings. To make this general-purpose, uncomment the lines
    // pertaining to the angles, the number of readings, and the pthread_ stuff. -- jbl

    // aquire the lock for the readings and angles -- this could cause deadlock if
    // something tries to aquire both of these locks in an order opposite of what
    // is here, but I don't know how to work around that. Just make sure that if
    // these two locks are aquired at the same time, it's in this order -- and
    // it wouldn't hurt to repeat this comment wherever that happens

    //pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*) &m_mutexAngles);
    //pthread_mutex_lock(&m_mutexAngles);
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &m_mutexRawRanges);
    pthread_mutex_lock(&m_mutexRawRanges);

    // send the current time
    timeval tv;
    gettimeofday(&tv, NULL);
    send(m_iStreamFd, (void*) &tv, sizeof(timeval), 0);

    // send the XYT value of the robot
    /*
    float dummy[3] = { 0, 0, 0 };
    float* pfXyt = (robot != NULL) ? robot->xyt : dummy;
    send(m_iStreamFd, pfXyt, sizeof(float) * 3, 0);
    */
    HSPose_t pose;
    float xyt[3];

    memset(xyt, 0x0, sizeof(float)*3);
    memset((HSPose_t *)&pose, 0x0, sizeof(HSPose_t));

    if (gPoseCalc != NULL)
    {
        gPoseCalc->getPose(pose);
        xyt[0] = pose.loc.x;
        xyt[1] = pose.loc.y;
        xyt[2] = pose.rot.yaw;
    }
    send(m_iStreamFd, (float *)xyt, sizeof(float)*3, 0);


    // send how many readings there are
    //send(m_iStreamFd, &m_iTotalReadings, sizeof(m_iTotalReadings), 0);

    // send the angles and raw ranges
    //send(m_iStreamFd, m_pfAllAngles, sizeof(float) * m_iTotalReadings, 0);
    send(m_iStreamFd, m_puAllRawRanges, sizeof(unsigned int) * m_iTotalReadings, 0);

    //pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
}

void Laser::sendLaserStreamIpt()
{
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock,
                          (void*) &m_mutexRawRanges);
    pthread_mutex_lock(&m_mutexRawRanges);

    REC_LaserScan rec;
    rec.size = m_iNumReadings;
    rec.min = m_iMinIndex;
    rec.scan = m_puRawRanges;
    m_pclCommunicator->SendMessage(m_pclLaserConnection, MSG_LaserScan, &rec);

    pthread_cleanup_pop(1);
}

 /* Process laser range readings */
void Laser::processMeasuredValues(byte_t* buf)
{
    uint bunits, numval, range, i, minhit, minhit_i;

    minhit = 9000;

    numval = (buf[1] << 8) | buf[0];
    bunits = (numval & 0xC000) >> 14;
    numval = numval & 0x3fff;

    // aquire the lock for the readings -- this could cause deadlock if
    // something tries to aquire both of these locks in an order opposite of what
    // is here, but I don't know how to work around that. Just make sure that if
    // these two locks are aquired at the same time, it's in this order -- and
    // it wouldn't hurt to repeat this comment wherever that happens
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &m_mutexRanges);
    pthread_mutex_lock(&m_mutexRanges);

    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &m_mutexRawRanges);
    pthread_mutex_lock(&m_mutexRawRanges);

    float* pfCurrent = &(m_pfSickRanges[m_iCurrentBuffer * m_iNumReadings]);
    for(i = 0; i < numval * 2; i += 2)
    {
        range = (buf[3 + i] << 8) | buf[2 + i];
        if (range < minhit)
        {
            minhit = range;
            minhit_i = i;
        }
        m_puRawRanges[i / 2] = range;
        *pfCurrent = (float)range / 100.0;
        m_pfCurrentSickRanges[i / 2] = *pfCurrent;
        m_pfAngles[i / 2] = m_pfRawAngles[i / 2];
        AccountForXYOffset(m_pfCurrentSickRanges[i / 2], m_pfAngles[i / 2]);
        pfCurrent++;
    }
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    m_iMinReading = minhit;
    m_iMinIndex = minhit_i / 2;
    RecalculateMinReading();
    m_suExtraInfo.bufnum = m_iCurrentBuffer;
    pthread_cond_broadcast(&m_condReader);
    if (callback != NULL)
    {
        callback(&m_pfSickRanges[m_iCurrentBuffer * m_iNumReadings], m_iCurrentBuffer);
    }
    updateStatusBar_();
    if (m_bStreaming)
    {
        sendLaserStream();
    }
    if (m_bStreamIpt)
    {
        sendLaserStreamIpt();
    }

    // increment the current buffer with wrap-around
    m_iCurrentBuffer = (m_iCurrentBuffer + 1) % m_iNumBuffers;
}

void Laser::printStatus(uint status)
{
    char buf[200];

    sprintf(buf, "SICK Status: ");
    switch (status & 0x07)
    {
    case 0:
        strcat(buf, "no error ");
        break;
    case 1:
        strcat(buf, "info ");
        break;
    case 2:
        strcat(buf, "warning ");
        break;
    case 3:
        strcat(buf, "error ");
        break;
    case 4:
        strcat(buf, "fatal error ");
        break;
    }
    switch((status & 0x18) >> 3)
    {
    case 0:
        strcat(buf, "LMS1-4 ");
        break;
    case 1:
        strcat(buf, "LMI ");
        break;
    case 2:
        strcat(buf, "LMS6 ");
        break;
    case 3:
        strcat(buf, "reserved ");
        break;
    }
    if (status & 0x20)
    {
        strcat(buf, "Reset-H ");
    }
    else
    {
        strcat(buf, "Reset-L ");
    }
    if (status & 0x40)
    {
        strcat(buf, "Implausible ");
    }
    if (status & 0x80)
    {
        strcat(buf, "Pollution ");
    }
    printTextWindow(buf);
}

void Laser::printPwonString(const char* str, int len)
{
    char* scpy = new char[len + 1];
    strncpy(scpy, str, len);
    scpy[len] = ';';
    char* sp = scpy;
    printfTextWindow("Product ID: %s  Type Key: %s  sw version: %s",
                      strsep(&sp, ";"), strsep(&sp, ";"), strsep(&sp, ";"));
    delete [] scpy;
}

void Laser::printBmack(byte_t code)
{
    sprintf(rstr, "Mode change ");
    if (code == 0x00)
    {
        strcat(rstr, "successfull");
    }
    else if (code == 0x01)
    {
        strcat(rstr, "incorrect pasword");
    }
    else if (code == 0x02)
    {
        strcat(rstr, "LMI fault");
    }
    printTextWindow(rstr);
}

void Laser::printSensorStatus(byte_t* buf, uint len)
{
    char tbuf[10];
    uint tint, tint2;
    strncpy(tbuf, (char*) &buf[SS_SW_VERSION], SS_SW_VERSION_LEN);
    tbuf[SS_SW_VERSION_LEN] = 0;
    sprintf(rstr, "Sensor Status: SW Version %s Mode: ", tbuf);
    switch(buf[SS_MODE])
    {
    case 0x00:
        strcat(rstr, "Installation");
        break;
    case 0x01:
        strcat(rstr, "Calibration");
        break;
    case 0x10:
        strcat(rstr, "Diagnostic");
        break;
    case 0x20:
        strcat(rstr, "Mon min seg");
        break;
    case 0x21:
        strcat(rstr, "Mon min seg on request");
        break;
    case 0x22:
        strcat(rstr, "Mon min perp per seg");
        break;
    case 0x23:
        strcat(rstr, "Mon min perp per seg on request");
        break;
    case 0x24:
        strcat(rstr, "Mon all");
        break;
    case 0x25:
        strcat(rstr, "Mon all on request");
        break;
    default:
        strcat(rstr, "Unknown sensor mode");
        return;
    }
    printTextWindow(rstr);
    if (buf[SS_STATUS])
    {
        //printStatusError(buf[SS_STATUS]);
    }
    strncpy(tbuf, (char*) &buf[SS_PROD_CODE], SS_PROD_CODE_LEN);
    tbuf[SS_PROD_CODE_LEN] = 0;
    sprintf(rstr, "Prod Code: %s  Fieldset in ", tbuf);
    if (buf[SS_VARIANT])
    {
        strcat(rstr, "non-");
    }
    strcat(rstr, "volatile memory");
    printTextWindow(rstr);
    tint =  (int) buf[SS_SCAN_ANGLE + 1] << 8 | buf[SS_SCAN_ANGLE];
    tint2 = (int) buf[SS_SSHOT_RES  + 1] << 8 | buf[SS_SSHOT_RES ];
    printfTextWindow("Scan angle %d degrees  single-shot res %.2f degrees",
                      tint, (float) tint2 / 100.0);
    sprintf(rstr, "Baud ");
    tint = (int) buf[SS_BAUD + 1] << 8 | buf[SS_BAUD];
    switch(tint)
    {
    case 0x8001:
        strcat(rstr, "500K");
        break;
    case 0x8002:
        strcat(rstr, "334K");
        break;
    case 0x8003:
        strcat(rstr, "250K");
        break;
    case 0x8004:
        strcat(rstr, "200K");
        break;
    case 0x8008:
        strcat(rstr, "111K");
        break;
    case 0x8010:
        strcat(rstr, "58.8K");
        break;
    case 0x8019:
        strcat(rstr, "38.4K");
        break;
    case 0x8033:
        strcat(rstr, "19.2K");
        break;
    case 0x8067:
        strcat(rstr, "9.6K");
        break;
    }
    if (buf[SS_PERM_BAUD])
    {
        strcat(rstr,"  rate kept");
    }
    else
    {
        strcat(rstr,"  rate not kept");
    }
    printTextWindow(rstr);
    printfTextWindow("Active field %d  unit %s  laser %s\n",
                      buf[SS_ACTIVE_FIELD], tbuf, buf[SS_LASER] ? "on" : "off");
}


int Laser::processTelegram(byte_t* buf, int len)
{
    int tgmLen, command;
    struct timeval nowtime;

    long timediff = 0;
    uint crc;
    unsigned char crc1, crc2, crc3, crc4;

    tgmLen = len;

    if (m_bFirsttime)
    {
        m_bFirsttime = false;
        gettimeofday(&m_oldtime, NULL);
    }
    gettimeofday(&nowtime, NULL);
    timediff = (nowtime.tv_sec - m_oldtime.tv_sec) * 1000000 + (nowtime.tv_usec - m_oldtime.tv_usec);

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTelegram(buf,tgmLen);
    }

    crc = createCRC(buf, tgmLen - 2);
    crc1 = crc & 0xff;
    crc2 = crc >> 8;
    crc3 = buf[tgmLen - 2];
    crc4 = buf[tgmLen - 1];
    if ((crc1 != crc3) || (crc2 != crc4))
    {
        printTextWindow("Laser packet bad CRC");

        return tgmLen;
    }
    command = buf[STGM_CMD];
    switch (command)
    {
    case SICK_MW:
        m_oldtime = nowtime;
        m_fUpdateSpeed = (1000000.0 / timediff);

        processMeasuredValues(&buf[STGM_DATA]);
        break;
    case SICK_INIT_ACK:
        break;
    case SICK_PWON:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printPwonString((char*) &buf[STGM_DATA], tgmLen - 7);
        }
        break;
    case SICK_BMACK:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printBmack(buf[STGM_DATA]);
        }
        break;
    case SICK_SS:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printSensorStatus(&buf[STGM_DATA], tgmLen - 7);
        }
        break;
    case SICK_LMSKFGAC:
        if (report_level >= HS_REP_LEV_DEBUG)
        {
            if (buf[STGM_DATA])
            {
                printTextWindow("LMS Config activated");
            }
            else
            {
                printTextWindow("LMS Config not accepted");
            }
        }
        break;
    case SICK_LMSKFG:
    case SICK_NACK:
    case SICK_ERR:
        break;
    default:
        printfTextWindow("SICK Error Unknown telegram process: %d", command);
        return tgmLen;
    }

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printStatus(buf[tgmLen - 3]);
    }

    return tgmLen;
}

bool Laser::readN(unsigned char* buf, int n)
{
    int idx = 0;
    int remaining = n;

    do
    {
        int nchar = read(m_iSerialPortFd, &buf[idx], remaining);
        if (nchar == -1)
        {
            printfTextWindow("SICK read error %d", errno);
            return false;
        }
        idx += nchar;
        remaining -= nchar;
    } while (remaining);

    return true;
}

void Laser::readTelegram()
{
    int tgmLen;
    bool process = false;
    bool received = false;

    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *) &m_mutexSerial);
    pthread_mutex_lock(&m_mutexSerial);

    // wait for sync byte (0x02)
    do
    {
        received = readN(&m_pbyteTelegramBuf[STGM_STX], 1);
    }
    // leave when either read error (not received),
    // or when correct start byte received
    while (received && (m_pbyteTelegramBuf[STGM_STX] != SICK_STX));

    if (received)
    {

        // next byte has to be 0x80 (address used for laser on RS232)
        received = readN(&m_pbyteTelegramBuf[STGM_ADR], 1);
        if ((received) && (m_pbyteTelegramBuf[STGM_ADR] == 0x80))
        {

            // read length of package, encoded in next 2 bytes
            received = readN(&m_pbyteTelegramBuf[STGM_LENL], 2);
            if (received)
            {
                tgmLen = (((int) m_pbyteTelegramBuf[STGM_LENH]) << 8) |
                    ((int) m_pbyteTelegramBuf[STGM_LENL]);

                // add number of bytes not included in tgmLen:
                // header (STX, ADR, LenH, LenL) and trailer (2 CRC bytes)
                tgmLen += 6;

                // read remaining package if length in acceptable range
                if (tgmLen <= MAX_TELEGRAM_SIZE)
                {
                    process = readN(&m_pbyteTelegramBuf[STGM_CMD], tgmLen - STGM_CMD);
                }
            }

        }

    }

    pthread_cleanup_pop(1);
    if (process)
    {
        processTelegram(m_pbyteTelegramBuf, tgmLen);
    }
}


void Laser::sickClose()
{
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Closing laser connection");
    }
    sendCommand(SICK_BM, SMOD_MON_REQUEST);
    pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void *) &m_mutexSerial);
    pthread_mutex_lock(&m_mutexSerial);
    close(m_iSerialPortFd);
    pthread_cleanup_pop(1);
}

void Laser::sickContinual()
{
    if (m_bContinual)
    {
        sendCommand(SICK_BM, SMOD_MON_REQUEST);
        printTextWindow("Laser continual stopped");
        m_bContinual = false;
    }
    else
    {
        sendCommand(SICK_BM, SMOD_MON_CONTINUAL);
        printTextWindow("Laser continual started");
        m_bContinual = true;
    }
}

void Laser::sickOneReading()
{
    if (!m_bContinual)
    {
        sendCommand(SICK_MWANF);
    }
}


int Laser::sickInitialize()
{
    if ((report_level > HS_REP_LEV_NONE) && (report_level != HS_REP_LEV_DEBUG))
    {
        printTextWindow("Connecting to laser... ", false);
    }
    else if (report_level > HS_REP_LEV_NONE)
    {
        char buf[100];
        sprintf(buf, "Connecting to laser on %s.", m_strPortStr.c_str());
        printTextWindow(buf, true);
    }// original : B38400, tried B500000

    //icializar(m_strPortStr.c_str());
    sleep(2);


    if (setupSerial(&m_iSerialPortFd, m_strPortStr.c_str(), B38400,
                      report_level, "Laser", 3) != 0)
    {
        return -1;
    }
    sendCommand(SICK_BM, SMOD_MON_CONTINUAL);
    m_bContinual = true;
    if ((report_level > HS_REP_LEV_NONE) && (report_level != HS_REP_LEV_DEBUG))
    {
        printTextWindow("connected", true);
    }
    else if (report_level == HS_REP_LEV_DEBUG)
    {
        printTextWindow("Laser connected", true);
    }
    m_bConnected = true;
    updateStatusBar_();
    return 0;
}


void Laser::carmenLaserReaderThread(void)
{
	printTextWindow("Laser CARMEN reader thread started");

	switch(ipc_connect_preferred_server(m_strName.c_str()))
	{
	case IPC_CONNECTION_ERROR:
		printTextWindow("Error connecting carmenLaser");
		return;
	case IPC_ALREADY_CONNECTED:
		IPC_subscribeData(CARMEN_LASER_FRONTLASER_NAME, laserCARMENHandler, this);
		iptHandler->set_state(IPT_MOD_LASER, true, false);
		return;
	}

	printfTextWindow("Connected to CARMEN laser at %s\n", ipc_preferred_server_name());

	IPC_subscribeData(CARMEN_LASER_FRONTLASER_NAME, laserCARMENHandler, this);
	iptHandler->set_state(IPT_MOD_LASER, true, false);

	 while (true)
	 {
		 pthread_testcancel();
	     IPC_listenWait(1000);
	 }
}

static void* start_thread(void* laser_instance)
{
    ((Laser*) laser_instance)->reader_thread();
    return NULL;
}

void Laser::reader_thread(void)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while (true)
    {
        pthread_testcancel();
        readTelegram();

        if (gLogManager != NULL)
        {
            manageLog_(m_threadReader);
        }

        if (watchdog_ != NULL)
        {
            watchdog_->notifyUpdate();
        }
    }
}

void Laser::waitNext()
{
    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*) &m_mutexCond);
    pthread_mutex_lock(&m_mutexCond);
    pthread_cond_wait(&m_condReader, &m_mutexCond);
    pthread_cleanup_pop(1);

}

void Laser::addSensor()
{
    // only create a new sensor for the laser if one doesn't already exist
    if (m_pclSensorLaser == NULL)
    {
        m_pclSensorLaser = new Sensor(
            SENSOR_SICKLMS200,
            sizeof(extra_laser_t),
            (char*)&m_suExtraInfo,
            m_iTotalReadings,
            m_pfAllAngles,
            m_iTotalReadings,
            m_pfAllReadings,
            &m_mutexAngles,
            &m_mutexRanges);
    }
}

void Laser::UpdateLaserPart()
{
    if (m_pclPartLaser != NULL)
    {
        delete m_pclPartLaser;
        m_pclPartLaser = NULL;
    }

    if (m_iTotalReadings > 0)
    {
        m_pclPartLaser = new RobotPartLaser(m_iTotalReadings, 0, SICK_FOV, SICK_RANGE);
    }
}

void Laser::control()
{
    bool bDone = false;

    messageDrawWindow(EnMessageType_LASER_CONTROL, EnMessageErrType_NONE);
    do
    {
        int c = getch();
        switch(c)
        {
        case 'd':   // disconnect
            messageHide();
            delete this;
            return;
            break;
        case 's':   // stream data
			printTextWindow("comenzado stream\n");
            streamStart();
            break;
        case 'x':   // exit
        case KEY_ESC:
            bDone = true;
            break;
        case 'r':   // refresh screen
            refreshScreen();
            break;
        case 'c':   // start/stop continual
            sickContinual();
            break;
        case 'u':   // update laser readings
            sickOneReading();
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
    } while (!bDone);

    messageHide();
}

float Laser::NormalizeAngle(float fAngle) // jbl
{
    const float fDEGREES_IN_CIRCLE = 360.0;

    // make sure the offset is less than the number of degrees in a circle
    while (fAngle >= fDEGREES_IN_CIRCLE)
    {
        fAngle -= fDEGREES_IN_CIRCLE;
    }

    // make sure the offset is positive
    while (fAngle < 0.0)
    {
        fAngle += fDEGREES_IN_CIRCLE;
    }

    return fAngle;
}

void Laser::convertAddress(char* address)
{
    hostent* hent;

    if ((hent = gethostbyname(address)) != NULL)
    {
        printfTextWindow("streaming address changed %s -> %s", address, hent->h_name);
        address = strdup(hent->h_addr);
    }
    else
    {
        printTextWindow("error resolving stream address");
    }
}


Laser::~Laser()
{
    if (m_bReaderRunning)
    {
        pthread_cancel(m_threadReader);
        pthread_join(m_threadReader, NULL);
        sickClose();
    }
    if (m_pfSickRanges != NULL)
    {
        delete [] m_pfSickRanges;
    }
    if (m_pbyteTelegramBuf != NULL)
    {
        delete [] m_pbyteTelegramBuf;
    }
    if (m_pfRawAngles != NULL)
    {
        delete [] m_pfRawAngles;
    }
    if (m_bStreamIpt || m_bReceiveIpt || m_bReceiveIpt)
    {
        shutdownIpt();
    }

    // only delete the sensor if this is the LAST laser being deleted
    if ((m_clLaserList.size() == 1) && (m_pclSensorLaser != NULL))
    {
        delete m_pclSensorLaser;
        m_pclSensorLaser = NULL;
    }

    // remove this laser object from the static list
    for (unsigned int i = 0; i < m_clLaserList.size(); i++)
    {
        if (m_clLaserList[i] == this)
        {
            m_clLaserList.erase(m_clLaserList.begin() + i);
            break;
        }
    }

    RemoveFromOverallBuffer();
    UpdateLaserPart();

    printTextWindow("Laser disconnected");
}

Laser::Laser(
    EnConnectType enConnectType,
    const string& strPort,
    const string& strName,
    float fDegreeOffset,
    float fXOffset,
    float fYOffset) :
    Module((Module**) NULL, HS_MODULE_NAME_LASER_SICK)
{
    int iRtn = -1;

    m_clLaserList.insert(m_clLaserList.begin(), (Laser*) NULL);
    m_clLaserList[0] = this;

    m_strName = strName;
    m_strPortStr = strPort;
    m_iMaxRetries = 20;
    m_iTelegramBufSize = MAX_TELEGRAM_SIZE;
    m_iNumReadings = SDAT_NUM_READINGS;
    m_iNumBuffers = 5;
    m_iCurrentBuffer = 0;
    m_bConnected = false;
    m_fUpdateSpeed = 0;
    m_fDegreeOffset = NormalizeAngle(fDegreeOffset);

    if (m_iNumReadings > 0)
    {
        if ((m_pfRawAngles = new float[m_iNumReadings]) == NULL)
        {
            printfTextWindow("Error: Couldn't space for raw angles for laser with name: %s\n",
                              strName.c_str());
            delete this;
            return;
        }

        // calculate the new angles for this laser
        for (int i = 0; i < m_iNumReadings; i++)
        {
            m_pfRawAngles[i] = IndexToDegrees(i);
        }
    }

    SetXYOffsets(fXOffset, fYOffset);

    if ((m_pfSickRanges = new float[m_iNumBuffers * m_iNumReadings]) == NULL)
    {
        printfTextWindow("Error: Couldn't allocate space for SICK ranges for "
                          "laser with name: %s\n", strName.c_str());
        delete this;
        return;
    }

    m_pbyteTelegramBuf = new byte_t[m_iTelegramBufSize];

    if (m_pbyteTelegramBuf == NULL)
    {
        printfTextWindow("Error: Couldn't allocate space for SICK telegram buffer for "
                          "laser with name: %s\n", strName.c_str());
        delete this;
        return;
    }

    m_bReaderRunning = false;

    if (enConnectType == EnConnectType_DIRECT_AND_STREAM_IPT)
        m_bStreamIpt = true;
    else
        m_bStreamIpt = false;

    if (enConnectType == EnConnectType_LISTEN_IPT)
        m_bReceiveIpt = true;
    else
        m_bReceiveIpt = false;

    if(enConnectType == EnConnectType_CARMEN)
        m_bReceiveCarmen = true;
    else
    	m_bReceiveCarmen = false;


    AddToOverallBuffer();

    callback = NULL;
    pthread_cond_init(&m_condReader, NULL);
    pthread_mutex_init(&m_mutexCond, NULL);
    pthread_mutex_init(&m_mutexSerial, NULL);

    updateStatusBar_();
    addSensor();
    UpdateLaserPart();

    if (m_bStreamIpt || m_bReceiveIpt || m_bReceiveCarmen)
    {
        iRtn = setupIpt();
        m_bListenIpt = false;

        if (iRtn != 0)
        {
            delete this;
            return;
        }
        else if (m_bReceiveIpt || m_bReceiveCarmen)
        {
            m_bListenIpt = true;
        }
    }

    if ((!m_bReceiveIpt) && (!m_bReceiveCarmen))
    {
        if (sickInitialize())
        {
            delete this;
            return;
        }
        pthread_create(&m_threadReader, NULL, start_thread, this);

        m_bReaderRunning = true;

        if (gWatchdogEnabled)
        {
            watchdog_ = new Watchdog(
                NAME_,
                WATCHDOG_CHECK_INTERVAL_SEC_,
                getpid(),
                m_threadReader);
        }
    }

    if (iRtn != 0)
    {
        m_bStreamIpt  = false;
        m_bReceiveIpt = false;
        m_bReceiveCarmen = false;
    }
}

// this function assumes that the new laser has already been inserted into
// the static laser list
void Laser::AddToOverallBuffer()
{
    unsigned int i;

    // save the static buffers temporarily so we can delete them later
    float* pfOldReadings = m_pfAllReadings;
    float* pfOldAngles   = m_pfAllAngles;
    unsigned int* puOldRawRanges = m_puAllRawRanges;

    // make new buffers large enough to encorporate the info of the new laser
    float* pfNewReadings         = new float[m_iTotalReadings + m_iNumReadings];
    float* pfNewAngles           = new float[m_iTotalReadings + m_iNumReadings];
    unsigned int* puNewRawRanges = new unsigned int[m_iTotalReadings + m_iNumReadings];

    // aquire the lock for the readings and angles -- this could cause deadlock if
    // something tries to aquire both of these locks in an order opposite of what
    // is here, but I don't know how to work around that. Just make sure that if
    // these two locks are aquired at the same time, it's in this order -- and
    // it wouldn't hurt to repeat this comment wherever that happens
    pthread_mutex_lock(&m_mutexRanges);
    pthread_mutex_lock(&m_mutexAngles);
    pthread_mutex_lock(&m_mutexRawRanges);

    // give the new laser space after the old buffers. This is un-allocated memory,
    // but it makes the upcoming transition (in the next "for" loop) easier.
    m_pfCurrentSickRanges = pfOldReadings + m_iTotalReadings;
    m_pfAngles = pfOldAngles + m_iTotalReadings;
    m_puRawRanges = puOldRawRanges + m_iTotalReadings;

    // alter all laser objects so they point to the new buffers. Offset the lasers'
    // readings in the new buffer by the same amount that they were offset in the
    // old buffer
    for (i = 0; i < m_clLaserList.size(); i++)
    {
        m_clLaserList[i]->m_pfCurrentSickRanges = pfNewReadings +
            (m_clLaserList[i]->m_pfCurrentSickRanges - pfOldReadings);
        m_clLaserList[i]->m_pfAngles = pfNewAngles +
            (m_clLaserList[i]->m_pfAngles - pfOldAngles);
        m_clLaserList[i]->m_puRawRanges = puNewRawRanges +
            (m_clLaserList[i]->m_puRawRanges - puOldRawRanges);
    }

    // copy the from the old buffers to the new ones
    if (m_iTotalReadings > 0)
    {
        memcpy(pfNewReadings,  pfOldReadings,
                sizeof(m_pfAllReadings[0] ) * m_iTotalReadings);
        memcpy(pfNewAngles,    pfOldAngles,
                sizeof(m_pfAllReadings[0] ) * m_iTotalReadings);
        memcpy(puNewRawRanges, puOldRawRanges,
                sizeof(m_puAllRawRanges[0]) * m_iTotalReadings);
    }

    // initialize the reading angles. the raw angles here will only be an approximation,
    // but it's better than initializing to 0 if the readings are needed between the time
    // that this laser is added and the time another laser reading comes in
    for (i = 0; i < (unsigned int) m_iNumReadings; i++)
    {
        m_pfAngles[i] = m_pfRawAngles[i];
    }

    // alter the static laser data
    m_pfAllReadings  =  pfNewReadings;
    m_pfAllAngles    =  pfNewAngles;
    m_puAllRawRanges =  puNewRawRanges;
    m_iTotalReadings += m_iNumReadings;

    // if the laser sensor exists, then update it, otherwise it will be created
    // later and initialized appropriately
    if (m_pclSensorLaser != NULL)
    {
        m_pclSensorLaser->setReadingInfo(m_pfAllReadings, m_iTotalReadings);
        m_pclSensorLaser->setAngleInfo(  m_pfAllAngles,   m_iTotalReadings);
    }

    // release the lock for the readings and angles
    pthread_mutex_unlock(&m_mutexRawRanges);
    pthread_mutex_unlock(&m_mutexRanges);
    pthread_mutex_unlock(&m_mutexAngles);

    // get rid of the old buffers
    if (pfOldReadings != NULL)
    {
        delete [] pfOldReadings;
    }
    if (pfOldAngles != NULL)
    {
        delete [] pfOldAngles;
    }
    if (puOldRawRanges != NULL)
    {
        delete [] puOldRawRanges;
    }
}

// this assumes that this laser has already been removed from the static
// laser list
void Laser::RemoveFromOverallBuffer()
{
    // save the static buffers temporarily so we can delete them later
    float* pfOldReadings         = m_pfAllReadings;
    float* pfOldAngles           = m_pfAllAngles;
    unsigned int* puOldRawRanges = m_puAllRawRanges;

    float* pfNewReadings         = NULL;
    float* pfNewAngles           = NULL;
    unsigned int* puNewRawRanges = NULL;

    // check to see if there are laser readings other than this laser's
    if (m_iTotalReadings != m_iNumReadings)
    {
        // make new, smaller buffers
        pfNewReadings  = new float[m_iTotalReadings - m_iNumReadings];
        pfNewAngles    = new float[m_iTotalReadings - m_iNumReadings];
        puNewRawRanges = new unsigned int[ m_iTotalReadings - m_iNumReadings];

        // aquire the lock for the readings and angles -- this could cause deadlock if
        // something tries to aquire both of these locks in an order opposite of what
        // is here, but I don't know how to work around that. Just make sure that if
        // these two locks are aquired at the same time, it's in this order -- and
        // it wouldn't hurt to repeat this comment wherever that happens
        pthread_mutex_lock(&m_mutexRanges);
        pthread_mutex_lock(&m_mutexAngles);
        pthread_mutex_lock(&m_mutexRawRanges);

        // get the number of readings in the buffer in front of and behind this laser's
        int iReadingsInFront = m_pfCurrentSickRanges - m_clLaserList[0]->m_pfCurrentSickRanges;

        // we'll get a negative number if we're killing what was the first laser in the list
        if (iReadingsInFront < 0)
        {
            iReadingsInFront = 0;
        }
        int iReadingsBehind  = m_iTotalReadings - iReadingsInFront - m_iNumReadings;

        // copy readings and angles up to those belonging to this laser to the new buffer
        if (iReadingsInFront > 0)
        {
            memcpy(pfNewReadings,  pfOldReadings,
                    iReadingsInFront * sizeof(pfOldReadings[0] ));
            memcpy(pfNewAngles,    pfOldAngles,
                    iReadingsInFront * sizeof(pfOldReadings[0] ));
            memcpy(puNewRawRanges, puOldRawRanges,
                    iReadingsInFront * sizeof(puOldRawRanges[0]));
        }

        // copy readings and angles after those belonging to this laser to the new buffer
        if (iReadingsBehind > 0)
        {
            memcpy(pfNewReadings + iReadingsInFront,
                    pfOldReadings + iReadingsInFront + m_iNumReadings,
                    iReadingsBehind * sizeof(pfOldAngles[0]));
            memcpy(pfNewAngles + iReadingsInFront,
                    pfOldAngles + iReadingsInFront + m_iNumReadings,
                    iReadingsBehind * sizeof(pfOldAngles[0]));
            memcpy(puNewRawRanges + iReadingsInFront,
                    puOldRawRanges + iReadingsInFront + m_iNumReadings,
                    iReadingsBehind * sizeof(puOldRawRanges[0]));
        }

        // alter all laser objects so they point to the new buffers
        for (unsigned int i = 0; i < m_clLaserList.size(); i++)
        {
            // find out how far into the overall buffer the current laser's readings are
            int iOffset = m_clLaserList[i]->m_pfCurrentSickRanges - pfOldReadings;

            // if the current laser's readings came after the readings of the laser being
            // deleted, move it's pointer up in the buffer by the amount of readings from
            // the laser being deleted
            if (m_clLaserList[i]->m_pfCurrentSickRanges > m_pfCurrentSickRanges)
            {
                iOffset -= m_iNumReadings;
            }

            // place the pointer to the current laser's readings in the new buffer, using
            // the beginning of the buffer as a reference
            m_clLaserList[i]->m_pfCurrentSickRanges = pfNewReadings  + iOffset;
            m_clLaserList[i]->m_pfAngles            = pfNewAngles    + iOffset;
            m_clLaserList[i]->m_puRawRanges         = puNewRawRanges + iOffset;
        }

        // alter the static laser data
        m_pfAllReadings  =  pfNewReadings;
        m_pfAllAngles    =  pfNewAngles;
        m_puAllRawRanges =  puNewRawRanges;
        m_iTotalReadings -= m_iNumReadings;

        // update the sensor to point to the new buffers
        if (m_pclSensorLaser != NULL)
        {
            m_pclSensorLaser->setReadingInfo(m_pfAllReadings, m_iTotalReadings);
            m_pclSensorLaser->setAngleInfo(  m_pfAllAngles,   m_iTotalReadings);
        }

        // release the lock for the readings and angles
        pthread_mutex_unlock(&m_mutexRawRanges);
        pthread_mutex_unlock(&m_mutexRanges);
        pthread_mutex_unlock(&m_mutexAngles);
    }
    else
    {
        // alter the static laser data. It's OK to do so outside of the locks here since
        // there are no other lasers reading or writing to the buffers, and new lasers
        // are only created by the main thread.
        m_pfAllReadings  = NULL;
        m_pfAllAngles    = NULL;
        m_puAllRawRanges = NULL;
        m_iTotalReadings = 0;
    }

    // get rid of the old buffers
    if (pfOldReadings != NULL)
    {
        delete [] pfOldReadings;
	pfOldReadings = NULL;
    }
    if (pfOldAngles != NULL)
    {
        delete [] pfOldAngles;
	pfOldAngles = NULL;
    }
    if (puOldRawRanges != NULL)
    {
        delete [] puOldRawRanges;
	puOldRawRanges = NULL;
    }
}

void Laser::ConnectLaser()
{
    // display the connection options
    int c = messageGetChar(EnMessageFilterType_LASER_TYPE, EnMessageType_LASER_TYPE,
                            EnMessageErrType_LASER_TYPE);
    if (c == 'x') // 'x' is exit
    {
        return;
    }

    EnConnectType enConnectType = (EnConnectType) (c - '0');

    unsigned const int uPORT_ECHO_LINE = 4;
    string strPort;

    // get the connection port from the user if need be
    if((enConnectType != EnConnectType_LISTEN_IPT) && (enConnectType != EnConnectType_CARMEN))
    {
        strPort = messageGetString(EnMessageType_LASER, uPORT_ECHO_LINE);
    }

    unsigned const int uNAME_ECHO_LINE = 6;
    string strName;

    // get the name for the laser
    strName = messageGetString(EnMessageType_ENTER_A_LASER_NAME, uNAME_ECHO_LINE);

    // user the default name if none was entered
    if (strName == "")
    {
        strName = m_strDEFAULT_NAME;
    }

    // don't create another laser with the same name as an existing laser
    if (!ValidName(strName))
    {
        printTextWindow("Error: Invalid Name\n");
        printTextWindow("  Either the name you typed was empty,\n");
        printTextWindow("  or the name was already in use.\n");
        return;
    }

    const unsigned int uANGLE_OFFSET_ECHO_LINE = 8;
    const unsigned int uX_OFFSET_ECHO_LINE     = 8;
    const unsigned int uY_OFFSET_ECHO_LINE     = 8;
    string strTemp;
    char* pcEnd;
    float fAngleOffset;
    float fXOffset;
    float fYOffset;

    // get the angle offset for the laser. repeat the prompt until the user gets it right
    while (true)
    {
        strTemp = messageGetString(EnMessageType_LASER_ANGLE_OFFSET, uANGLE_OFFSET_ECHO_LINE);
        fAngleOffset = (float) strtod(strTemp.c_str(), &pcEnd);

        // if the user wants to quit, then quit
        if ((strTemp == "x") || (strTemp == "X"))
        {
            return;
        }

        // if the string was a valid number, then don't prompt again
        if (*pcEnd == '\0')
        {
            break;
        }

        // if we got here, the user screwed up
        messageDrawWindow(EnMessageType_NONE, EnMessageErrType_INVALID_NUMBER_FORMAT);
        sleep(1);
    };

    // get the x-offset for the laser. repeat the prompt until the user gets it right
    while (true)
    {
        strTemp = messageGetString(EnMessageType_LASER_X_OFFSET, uX_OFFSET_ECHO_LINE);
        fXOffset = strtod(strTemp.c_str(), &pcEnd) / 100.0; // convert cm to meters

        // if the user wants to quit, then quit
        if ((strTemp == "x") || (strTemp == "X"))
        {
            return;
        }

        // if the string was a valid number, then don't prompt again
        if (*pcEnd == '\0')
        {
            break;
        }

        // if we got here, the user screwed up
        messageDrawWindow(EnMessageType_NONE, EnMessageErrType_INVALID_NUMBER_FORMAT);
        sleep(1);
    };

    // get the y-offset for the laser. repeat the prompt until the user gets it right
    while (true)
    {
        strTemp = messageGetString(EnMessageType_LASER_Y_OFFSET, uY_OFFSET_ECHO_LINE);
        fYOffset = strtod(strTemp.c_str(), &pcEnd) / 100.0; // convert cm to meters

        // if the user wants to quit, then quit
        if ((strTemp == "x") || (strTemp == "X"))
        {
            return;
        }

        // if the string was a valid number, then don't prompt again
        if (*pcEnd == '\0')
        {
            break;
        }

        // if we got here, the user screwed up
        messageDrawWindow(EnMessageType_NONE, EnMessageErrType_INVALID_NUMBER_FORMAT);
        sleep(1);
    };

    new Laser(enConnectType, strPort, strName, fAngleOffset, fXOffset, fYOffset);
}

// this checks to see if a laser name is valid by making sure the name isn't
// the empty string and making sure no other lasers already have the name
bool Laser::ValidName(const string& strName)
{
    bool bValid = (strName != "");
    bool bMatchFound = false;

    if (bValid)
    {
        // test all the names in the existing lasers
        for (unsigned int i = 0; i < m_clLaserList.size(); i++)
        {
            if (strName == m_clLaserList[i]->m_strName)
            {
                bMatchFound = true;
                break;
            }
        }
    }

    return bValid && !bMatchFound;
}

// this presents a list of laser names to the user and returns the address
// of the laser with the name the user chose
Laser* Laser::GetLaserByName()
{
    Laser* pclLaser = NULL;
    unsigned int uLaserCount;
    char c;

    // make an array of the names
    char** aszNames = new char*[m_clLaserList.size() + 1];
    for (uLaserCount = 0; uLaserCount < m_clLaserList.size(); uLaserCount++)
    {
        aszNames[uLaserCount] = (char*) m_clLaserList[uLaserCount]->m_strName.c_str();
    }
    aszNames[uLaserCount] = NULL;   // cap off the end of the list

    // get the laser number, which will be '1' based
    c = messageGetChar(EnMessageFilterType_LASER_NAME, EnMessageType_CHOOSE_A_LASER_NAME,
                        EnMessageErrType_GENERAL_INVALID, aszNames, true, &uLaserCount);

    if (c != 'x') // 'x' is exit
    {
        pclLaser = m_clLaserList[c - '0' - 1]; // subtract 1 to make 0 based
    }

    delete [] aszNames;

    return pclLaser;
}

void Laser::logData_(double logTime)
{
    int i, numReadings;
    float *readings = NULL, *angles = NULL;

    if (logfile_ != NULL)
    {
        if (m_clLaserList[0] == this)
        {
            numReadings = m_iTotalReadings;

            fprintf(logfile_, "%f %d", logTime, numReadings);

            if (numReadings > 0)
            {
                // Copy the angles
                angles = new float[numReadings];

                pthread_mutex_lock(&m_mutexAngles);
                memcpy(angles, m_pfAllAngles, sizeof(float)*numReadings);
                pthread_mutex_unlock(&m_mutexAngles);

                // Copy the readings.
                readings = new float[numReadings];

                pthread_mutex_lock(&m_mutexRanges);
                memcpy(readings, m_pfAllReadings, sizeof(float)*numReadings);
                pthread_mutex_unlock(&m_mutexRanges);

                for (i = 0; i < numReadings; i++)
                {
                    fprintf(logfile_, " %.1f %.2f", angles[i], readings[i]);
                }

                delete [] angles;
                angles = NULL;

                delete [] readings;
                readings = NULL;
            }

            fprintf(logfile_, "\n");
            fflush(logfile_);
        }
        else
        {
            fprintf(logfile_, "%f [See logfile for the other laser.]\n", logTime);
        }
    }
}

/**********************************************************************
# $Log: laser.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:41:06  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.9  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.8  2002/08/21 21:23:18  kaess
# hserver crashes due to laser solved
#
# Revision 1.7  2002/01/16 21:53:40  ernest
# Added WatchDog function
#
# Revision 1.6  2001/05/31 17:41:03  blee
# fixed an infinite-loop bug in readN
#
# Revision 1.5  2001/03/23 21:23:28  blee
# altered to use a config file and allow for a laser's physical
# offset on a robot
#
# Revision 1.4  2000/12/12 22:58:40  blee
# Extensive updates were made to support multiple lasers.
#
# Revision 1.3  2000/09/22 20:29:29  blee
# Fixed bug that caused hserver to send old data to MissionLab
# part of the time.
#
# Revision 1.2  2000/09/19 03:43:50  endo
# Streaming bug fixed.
#
#
#**********************************************************************/
