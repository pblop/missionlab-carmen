#include <string.h>
#include <termio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include "statusbar.h"
#include "hserver.h"
#include "message.h"
#include "ipt_handler.h"
#include "hserver_ipt.h"
#include "ipt/callbacks.h"
#include "ipt/connection.h"
#include "robot.h"
#include "PoseCalculatorInterface.h"
#include "Watchdog.h"
#include "LogManager.h"
#include "gps_default.h"

const int GpsDefault::WATCHDOG_CHECK_INTERVAL_SEC_ = 10;

static void start_thread_base(void* gps_instance);
static void start_thread(void* gps_instance);

declareHandlerCallback(GpsDefault)
implementHandlerCallback(GpsDefault)

void GpsDefault::update_jbox(char* gps_string)
{
    pthread_mutex_lock (&mutexgps2jbox);
    strcpy(gps_string, msg2jbox);
    pthread_mutex_unlock(&mutexgps2jbox);
}

void GpsDefault::updateStatusBar_(void)
{

    double Latitude;
    double Longitude;
    int Altitude;
    int Heading;

    char buf[100];
    if (base_relay)
    {
        sprintf(statusStr, "Gps base relay: ");
    }
    else
    {
        sprintf(statusStr, "Gps: ");
    }
    if (!connected)
    {
        sprintf(buf, "Connecting  time left: %d", max_retries);
        strcat(statusStr, buf);
    }
    else
    {
        if (base_relay)
        {
            strcat(statusStr, "Connected ");
        }

        if (!base_relay)
        {
            pthread_mutex_lock(&mutexStatsLock);

            sprintf(
                buf,
                "%c num_sats %d lat %f lon %f height %.3f direction %.2f rt20: %d",
                statusbarSpinner_->getStatus(),
                (int) stats[EnGpsInfoType_NUM_SATS],
                stats[EnGpsInfoType_LAT],
                stats[EnGpsInfoType_LON],
                stats[EnGpsInfoType_HEIGHT],
                stats[EnGpsInfoType_DIRECTION],
                rt20Status);

            Latitude = stats[EnGpsInfoType_LAT];
            Longitude = stats[EnGpsInfoType_LON];
            Altitude = (int) stats[EnGpsInfoType_HEIGHT];

            if (Altitude < 0)
            {
                // Note: JBox does not accept large negative values.
                // Therefore, we use the lower bound to be 0.
                Altitude = 0;
            }

            Heading = (int) stats[EnGpsInfoType_DIRECTION];
            //Heading = (int) (359.0*rand()/(RAND_MAX+1.0));
            pthread_mutex_unlock(&mutexStatsLock);

            pthread_mutex_lock (&mutexgps2jbox);
            sprintf(msg2jbox, "\"%f %f %d %d 0 0 0\"", Latitude, Longitude, Altitude, Heading);
            pthread_mutex_unlock(&mutexgps2jbox);

            //printTextWindow(msg2jbox);
            strcat(statusStr, buf);
        }
    }
    statusbar->update(statusLine);
}

void GpsDefault::printBLogHeader(bLogHeader* h)
{
    printfTextWindow("id %d %s count %d", h->id, idString(h->id), h->byte_count);
}


int GpsDefault::readBLog()
{
    bLogHeader header;
    int remaining;
    int rtn;

    pthread_mutex_lock(&serialMutex);
    if (!connected)
    {
        rtn = readN((char*) &header, 1, true);
        if (rtn)
        {
            pthread_mutex_unlock(&serialMutex);
            return -1;
        }
        readN(&(((char*) &header)[1]), sizeof(header) - 1, true);
    }
    else
    {
        header.sync[0] = 0;
        while ((header.sync[0] != 0xAA))
        {
            readN((char*) &header, 1); // Read one charactor into the header buffer.
        }
        readN(&(((char*) &header)[1]), sizeof(header) - 1, true);
    }
    if ((header.sync[1] != 0x44) || (header.sync[2] != 0x11))
    {
        printTextWindow("Error in blog header");
        pthread_mutex_unlock(&serialMutex);
        return -1;
    }
    remaining = header.byte_count - 12; /* remove header length */
    rtn = readN(bLogBuf, remaining, true);
    pthread_mutex_unlock(&serialMutex);
    if (rtn)
    {
        return rtn;
    }
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printBLogHeader(&header);
    }
    decodeBLog(header.id);
    if (save_packets)
    {
        pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*) &savePacketMutex);
        pthread_mutex_lock(&savePacketMutex);
        write(packetFd, (void*)&header, sizeof(header));
        write(packetFd, bLogBuf, remaining);
        pthread_cleanup_pop(1);
    }
    updateStatusBar_();
    return 0;
}

char *GpsDefault::idString(int id)
{
    switch(id)
    {
    case 1:
        return "POSB";
    case 2:
        return "CLKB";
    case 3:
        return "TM1B";
    case 4:
        return "MKTB";
    case 5:
        return "MKPB";
    case LOG_SPHB:
        return "SPHB";
    case 7:
        return "DOPB";
    case 8:
        return "NAVB";
    case 9:
        return "DCSB";
    case 12:
        return "SATB";
    case 13:
        return "RCSB";
    case 14:
        return "REPB";
    case 15:
        return "RALB";
    case 16:
        return "IONB";
    case 17:
        return "UTCB";
    case 18:
        return "ALMB";
    case 19:
        return "CTSB";
    case 26:
        return "PXYB";
    case 27:
        return "GGAB";
    case 32:
        return "RGEB";
    case 33:
        return "RGEC";
    case 34:
        return "VLHB";
    case 35:
        return "RT20B";
    case 36:
        return "SVDB";
    case LOG_P20B:
        return "P20B";
    case LOG_CDSB:
        return "CDSB";
    default:
        return "unknown";
    }
}

void GpsDefault::decodeBLog(int id)
{
    logSPHB *psuLogSPHB = NULL;
    logP20B* psuLogP20B = NULL;
    logCDSB* psuLogCDSB = NULL;
    HSPose_t pose;
    float x, y, theta;

    memset((HSPose_t *)&pose, 0x0, sizeof(HSPose_t));

    switch(id) {

    case LOG_SPHB:

        psuLogSPHB = (logSPHB*) bLogBuf;

        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printSPHB(psuLogSPHB);
        }

        pthread_mutex_lock(&mutexStatsLock);

        stats[EnGpsInfoType_DIRECTION       ] = psuLogSPHB->tog;
        stats[EnGpsInfoType_HORIZONTAL_SPEED] = psuLogSPHB->hspeed;
        stats[EnGpsInfoType_VERTICAL_SPEED  ] = psuLogSPHB->vspeed;

        theta = stats[EnGpsInfoType_DIRECTION];
        GPS_CRAMPDEG(theta, 0.0, 360.0);

        xyt[2] = theta;
        pthread_mutex_unlock(&mutexStatsLock);

        if ((gPoseCalc != NULL) && (poseCalcID_ >= 0) && (rt20Status < 2))
        {
            pose.rot.yaw = theta;
            pose.rot.extra.time = getCurrentEpochTime();
            gPoseCalc->updateModuleRotation(poseCalcID_, pose.rot);
        }

        break;

    case LOG_P20B:

        psuLogP20B = (logP20B*) bLogBuf;

        pthread_mutex_lock(&mutexStatsLock);

        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printP20B(psuLogP20B);
        }

        rt20Status = psuLogP20B->rt20Status;

        stats[EnGpsInfoType_LAT     ] = psuLogP20B->lat;
        stats[EnGpsInfoType_LON     ] = psuLogP20B->lon;
        stats[EnGpsInfoType_HEIGHT  ] = psuLogP20B->height;
        stats[EnGpsInfoType_NUM_SATS] = float(psuLogP20B->num_sats);

        LatLon2XY(psuLogP20B->lat, psuLogP20B->lon);
        x = xyt[0];
        y = xyt[1];

        pthread_mutex_unlock(&mutexStatsLock);

        if ((gPoseCalc != NULL) && (poseCalcID_ >= 0) && (rt20Status < 2))
        {
            pose.loc.x = x;
            pose.loc.y = y;
            pose.loc.extra.time = getCurrentEpochTime();
            gPoseCalc->updateModuleLocation(poseCalcID_, pose.loc);
        }

        break;

    case LOG_CDSB:

        psuLogCDSB = (logCDSB*) bLogBuf;

        pthread_mutex_lock(&mutexStatsLock);

        if (firstCDSB)
        {
            firstCDSB = false;
            rtcmMsgsRcvdAtGPSbeforeSession = psuLogCDSB->RTCMrecordsPassed;
            rtcmMsgsFailedAtGPSbeforeSession = psuLogCDSB->RTCMchecksumFails;
        }

        rtcmMsgsRcvdAtGPS = psuLogCDSB->RTCMrecordsPassed - rtcmMsgsRcvdAtGPSbeforeSession;
        rtcmMsgsFailedAtGPS = psuLogCDSB->RTCMchecksumFails - rtcmMsgsFailedAtGPSbeforeSession;

        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printCDSB(psuLogCDSB);
        }

        pthread_mutex_unlock(&mutexStatsLock);

        break;
    }
}



int GpsDefault::readN(char* buf, int n, bool no_timeout)
{
    int nchar = 0, idx = 0, remaining;

    remaining = n;
    do
    {
        nchar = read(gpsFd, &buf[idx], remaining);
        if (nchar == -1)
        {
            printfTextWindow("GPS read error %d", errno);
        }
        if (nchar == 0)
        {
            if (no_timeout)
            {
                return 1;
            }
        }
        idx += nchar;
        remaining -= nchar;
    } while (remaining);
    return 0;
}


void GpsDefault::send(char* str)
{
    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*) &serialMutex);
    pthread_mutex_lock(&serialMutex);
    write(gpsFd, str, strlen(str));
    pthread_cleanup_pop(1);
}

int GpsDefault::initializeBase(const string& strPortString)
{
    int rtn;

    rtn = setupSerial(&gpsFd, strPortString.c_str(), B9600, report_level, "Gps", 1);
    if (rtn)
    {
        gpsFd = -1;
        return -1;
    }
    pthread_create(&reader, NULL, (void*(*)(void*))&start_thread_base, (void*) this);

    if (gWatchdogEnabled)
    {
        watchdog_ = new Watchdog(NAME_, WATCHDOG_CHECK_INTERVAL_SEC_, getpid(), reader);
    }

    printfTextWindow("Gps base connected on %s", strPortString.c_str());
    updateStatusBar_();
    return 0;
}

int GpsDefault::initialize(const string& strPortString)
{
    int rtn;

    rtn = setupSerial(&gpsFd, strPortString.c_str(), B115200, report_level, "Gps", 3);
    if (rtn)
    {
        gpsFd = -1;
        return -1;
    }
    updateStatusBar_();
    pthread_create(&reader, NULL, (void*(*)(void*))&start_thread, (void*) this);

    if (gWatchdogEnabled)
    {
        watchdog_ = new Watchdog(NAME_, WATCHDOG_CHECK_INTERVAL_SEC_, getpid(), reader);
    }

    sem_wait(&connect_sem);
    if (connected)
    {
        printfTextWindow("Gps connected on %s", strPortString.c_str());
        return 0;
    }
    else
    {
        printTextWindow("Failed to connect to gps.");
        pthread_join(reader, NULL);
        return -1;
    }
}

GpsDefault::~GpsDefault()
{
    if ((gPoseCalc != NULL) && (poseCalcID_ >= 0))
    {
        gPoseCalc->disconnectModule(poseCalcID_);
        poseCalcID_ = -1;
    }

    if (listenIpt)
    {
        shutdownIpt();
    }
    pthread_cancel(reader);
    pthread_join(reader, NULL);
    if (gpsFd >= 0)
    {
        close(gpsFd);
    }
    delete [] sendBuf;
    delete [] rcvBuf;
    delete [] flushBuf;
    delete [] bLogBuf;
    if (!base_relay)
    {
        delete callback;
    }
    if (version != NULL)
    {
        free(version);
    }
    printTextWindow("Gps disconnected");
}

GpsDefault::GpsDefault(
    Gps** a,
    const string& strPortString,
    double base_lat,
    double base_lon,
    double base_x,
    double base_y,
    double mperlat,
    double mperlon,
    bool base) :
    Gps(a)
{
    int rtn, i;

    pthread_mutex_init(&mutexgps2jbox, NULL);
    sem_init(&connect_sem, 0, 0);
    connected = false;
    max_retries = 20;
    save_packets = false;
    for (i = 0; i < EnGpsInfoType_COUNT; i++)
    {
        stats[i] = 0;
    }
    report_level = HS_REP_LEV_NORMAL;
    base_latitude = base_lat;
    base_longitude = base_lon;
    this->base_x = base_x;
    this->base_y = base_y;
    m_per_deg_lon = mperlon;
    m_per_deg_lat = mperlat;
    base_relay = base;
    firstCDSB = true;
    version = NULL;
    rt20Status = -1;
    poseCalcID_ = -1;
    if (base)
    {
        printTextWindow("Starting GPS differential relay");
    }
    else
    {
        printfTextWindow("Starting GPS. port: %s  base_lat: %f  base_lon: %f",
                         strPortString.c_str(), base_lat,  base_lon);
        printfTextWindow("   base_x: %f  base_y: %f", base_x, base_y);
        printfTextWindow("   mperlat: %f  mperlon: %f", mperlat, mperlon);
    }
    gpsFd = -1;
    sendSize = 100;
    sendBuf = new char[sendSize];
    rcvSize = 100;
    rcvBuf = new char[rcvSize];
    flushSize = 100;
    flushBuf = new char[flushSize];
    bLogBufSize = 1024;
    bLogBuf = new char[bLogBufSize];
    rtcmBufSize = 1024;
    rtcmBuf = new char[rtcmBufSize];
    pthread_mutex_init(&serialMutex, NULL);
    if (!base_relay)
    {
        rtn = initialize(strPortString);
    }
    else
    {
        rtn = initializeBase(strPortString);
    }
    if (rtn)
    {
        delete this;
        return;
    }
    rtn = setupIpt();
    if (rtn)
    {
        listenIpt = false;
        if (base_relay)
        {
            printTextWindow("Need ipt for gps base differential relay");
            delete this;
            return;
        }
    }
    else
    {
        listenIpt = true;
    }
    if (!base_relay)
    {
        addSensors();
    }
    strcpy(msg2jbox, "\"0 0 0 0 0 0 0\"");
    updateStatusBar_();
}

void GpsDefault::logData_(double logTime)
{
	SuGpsStats stats;

	if (logfile_ != NULL)
	{
		stats = GetStats();

		fprintf(
				logfile_,
				"%f %f %f %f %f %f %f %d %.2f %.2f %.2f %d %d %d %f %f %f %f %f %f\n",
				logTime,
				stats.dLat,
				stats.dLon,
				stats.dHeight,
				stats.dDirection,
				stats.dHorizontalSpeed,
				stats.dVerticalSpeed,
				stats.iNumSats,
				stats.afXyt[0],
				stats.afXyt[1],
				stats.afXyt[2],
				stats.iRT20Status,
				stats.iRtcmMsgsRcvdAtGps,
				stats.iRtcmMsgsFailedAtGps,
				base_longitude,
				base_latitude,
				m_per_deg_lon,
				m_per_deg_lat,
				base_x,
				base_y);

		fflush(logfile_);
	}
}

void GpsDefault::callbackRTCM(IPMessage* message)
{
    REC_GpsRTCM* rec = (REC_GpsRTCM*) message->FormattedData();
    printfTextWindow("Gps RTCM message recieved, size %d  %x %x %x %x ... %x %x %x %x",
                      rec->size, rec->data[0], rec->data[1], rec->data[2], rec->data[3],
                      rec->data[rec->size-4], rec->data[rec->size-3], rec->data[rec->size-2],
                      rec->data[rec->size-1]);
    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*) &serialMutex);
    pthread_mutex_lock(&serialMutex);
    write(gpsFd, rec->data, rec->size);
    pthread_cleanup_pop(1);
}

int GpsDefault::setupIpt()
{
    bool done = false;
    REC_ModuleId msg, *msg_rply;
    if (iptHandler == NULL)
    {
        return -1;
    }
    else
    {
        communicator = iptHandler->communicator();
        iptHandler->registerMessages(iptGpsMessageArray);
        if (!base_relay)
        {
            callback = new HandlerCallback(GpsDefault)(this, &GpsDefault::callbackRTCM);
            IPMsgCallbackHandlerSpec hndArray[] = {
                { MSG_GpsRTCM, callback, IPT_HNDL_STD },
            };
            iptHandler->registerCallbackHandlers(1, hndArray);
            iptHandler->set_state(IPT_MOD_GPS, true, false);
        }
        else
        {
            do
            {
                updateStatusBar_();
                gps_connection = communicator->Connect("fred:control", IPT_OPTIONAL);
                if (gps_connection->FD() > 0)
                {
                    done = true;
                }
                else
                {
                    if (max_retries-- <= 0)
                    {
                        return -1;
                    }
                    sleep(1);
                }
            } while (!done);
            msg.id = MODULE_ID_GPS;
            IPMessage* pmsg = communicator->Query(gps_connection, MSG_ModuleId, &msg, MSG_ModuleId, 4);
            if (pmsg==NULL)
            {
                return -1;
            }
            msg_rply = (REC_ModuleId*) pmsg->FormattedData();
            if (msg_rply->id == MODULE_NOT_CONNECTED)
            {
                printTextWindow("Gps not running on other hserver");
                return -1;
            }
            delete pmsg;
            connected = true;
        }
    }
    return 0;
}

void GpsDefault::shutdownIpt()
{
    if (!base_relay)
    {
        char* hndNames[] = {
            MSG_GpsRTCM
        };
        iptHandler->disableHandlers(1, hndNames);
    }
}

static void start_thread(void* gps_instance)
{
    ((GpsDefault*) gps_instance)->reader_thread();
}

void GpsDefault::reader_thread()
{
    int rtn;

    if (!base_relay)
    {
        send("log com1 P20B ontime 1\n\r");
        send("log com1 SPHB ontime 1\n\r");
        send("accept com2 rt20\n\r");
        flush();
    }

    while (true)
    {
        pthread_testcancel();
        rtn = readBLog();
        if (!connected && !rtn)
        {
            connected = true;
            sem_post(&connect_sem);
        }
        else
        {
            if (rtn && !connected)
            {
                if (max_retries-- <= 0) {
                    sem_post(&connect_sem);
                    pthread_exit((void*) 1);
                }
                updateStatusBar_();
            }
        }

        if (gLogManager != NULL)
        {
            manageLog_(reader);
        }

        if (watchdog_ != NULL)
        {
            watchdog_->notifyUpdate();
        }
    }
}

static void start_thread_base(void* gps_instance)
{
    ((GpsDefault*) gps_instance)->base_reader_thread();
}

void GpsDefault::base_reader_thread()
{
    flush();
    while (true)
    {
        pthread_testcancel();
        readRtcm();
        updateStatusBar_();

        if (gLogManager != NULL)
        {
            manageLog_(reader);
        }

        if (watchdog_ != NULL)
        {
            watchdog_->notifyUpdate();
        }
    }
}



// mutexStatsLock should always be LOCKED when this function is called. -- jbl
void GpsDefault::LatLon2XY(double dLatitude, double dLongitude)
{
    xyt[0] = ((dLongitude - base_longitude) * m_per_deg_lon) + base_x;
    xyt[1] = ((dLatitude  - base_latitude ) * m_per_deg_lat) + base_y;
}



void GpsDefault::sendRtcmMessage(int size, char* data)
{
    REC_GpsRTCM rec;
    rec.size = size;
    rec.data = (char*) data;
    communicator->SendMessage(gps_connection, MSG_GpsRTCM, &rec);
}

void GpsDefault::readRtcm()
{
    int idx, nchar;
    do
    {
        do
        {
            //printTextWindow("Spin");
            readN(rtcmBuf, 1);
        } while ((rtcmBuf[0] != 0x66) && (rtcmBuf[0] != 0x59));
        readN(&rtcmBuf[1], 1);
    } while ((!((rtcmBuf[0]==0x66)&&((rtcmBuf[1]==0x41)||(rtcmBuf[1]==0x5d))))&&
              (!((rtcmBuf[0]==0x59)&&((rtcmBuf[1]==0x62)||(rtcmBuf[1]==0x7e)))));

    nchar = 0;
    idx = 2;
    do
    {
        idx += nchar;
        pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*) &serialMutex);
        pthread_mutex_lock(&serialMutex);
        nchar = read(gpsFd, &rtcmBuf[idx], 1); // really shouldn't read one at a time
        pthread_cleanup_pop(1);
    } while (rtcmBuf[idx] != 0x0a);
    printfTextWindow("got %d   %x %x %x %x ... %x %x %x %x", idx+1, rtcmBuf[0],
                      rtcmBuf[1], rtcmBuf[2], rtcmBuf[3], rtcmBuf[idx-3],
                      rtcmBuf[idx-2], rtcmBuf[idx-1], rtcmBuf[idx]);
    //rtcmBuf[idx-1] = 0x0a; // dave code
    if (iptHandler != NULL)
    {
        sendRtcmMessage(idx + 1, rtcmBuf);
    }
}


char* GpsDefault::solutionString(int status)
{
    switch(status)
    {
    case 0:
        return "Solution computed";
    case 1:
        return "Insufficient observations";
    case 2:
        return "No convergence";
    case 3:
        return "Singular AtPA Matrix";
    case 4:
        return "Covariance trace exceeds maximum";
    case 5:
        return "Test distance exceeded";
    case 6:
        return "Not yet converged from cold start";
    default:
        return "error: unknown solution status";
    }
}

char* GpsDefault::RT20StatusString(int status)
{
    switch(status)
    {
    case 0:
        return "RT20 solution computed";
    case 1:
        return "Solving ambiguities";
    case 2:
        return "Modelling monitor phase";
    case 3:
        return "Not sufficient observations";
    case 4:
        return "Varience exceeds limit";
    case 5:
        return "Root sum of squares residual exceeds maximum";
    case 6:
        return "Bridge exceeds maximum";
    case 7:
        return "Negative variance";
    case 8:
        return "Pseudorange position";
    default:
        return "error: unknown RT20 status";
    }
}

void GpsDefault::printP20B(logP20B* l)
{
    printfTextWindow("P20B  week %d sec %f lag %f sats %d datId %d cpu %d fix %d",
                      l->week, l->seconds, l->lag, l->num_sats, l->datId, l->cpuIdleTime, l->fix);
    printfTextWindow("      lat %f lon %f height %f und %f", l->lat, l->lon,
                      l->height, l->undulation);
    printfTextWindow("      %s", solutionString(l->solutionStatus));
    printfTextWindow("      %s", RT20StatusString(l->rt20Status));
}


void GpsDefault::printCDSB(logCDSB *l)
{
    printfTextWindow("CDSB week %d sec %d COM2: Xon %d CTS %d pe %d or %d fe %d "
                      "RTCMerr %d RTCMpass %d", l->week, l->seconds, l->xonCOM2,
                      l->ctsCOM2, l->parityErrorsCOM2, l->overrunErrorsCOM2,
                      l->framingErrorsCOM2, l->RTCMparityFails, l->RTCMrecordsPassed);
}

void GpsDefault::printSPHB(logSPHB *l)
{
    printfTextWindow("SPHB  week %d sec %f h %f v %f tog %f", l->week, l->seconds,
                      l->hspeed, l->vspeed, l->tog);
    printfTextWindow("      %s", solutionString(l->solutionStatus));
}


void GpsDefault::flush()
{
    int nchar = 1;
    while (nchar > 0)
    {
        pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*) &serialMutex);
        pthread_mutex_lock(&serialMutex);
        nchar = read(gpsFd, flushBuf, flushSize - 1);
        pthread_cleanup_pop(1);
    }
}

int GpsDefault::getLine()
{
    int nchar = 0, totchar = 0;
    char* c;
    do
    {
        totchar += nchar;
        pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*) &serialMutex);
        pthread_mutex_lock(&serialMutex);
        nchar = read(gpsFd, &rcvBuf[totchar], rcvSize - 1 - totchar);
        pthread_cleanup_pop(1);
        rcvBuf[totchar + nchar] = 0;
    } while (!(strchr((char*) &rcvBuf[totchar], 10))); /* Will miss data */
    c = strchr((char*) rcvBuf, '\n');
    *c = 0;
    return totchar;
}

SuGpsStats GpsDefault::GetStats()
{
    SuGpsStats suStats;

    pthread_mutex_lock(&mutexStatsLock);

    suStats.dLat                 = stats[EnGpsInfoType_LAT];
    suStats.dLon                 = stats[EnGpsInfoType_LON];
    suStats.dHeight              = stats[EnGpsInfoType_HEIGHT];
    suStats.dDirection           = stats[EnGpsInfoType_DIRECTION];
    suStats.dHorizontalSpeed     = stats[EnGpsInfoType_HORIZONTAL_SPEED];
    suStats.dVerticalSpeed       = stats[EnGpsInfoType_VERTICAL_SPEED];
    suStats.iNumSats             = (int) stats[EnGpsInfoType_NUM_SATS];
    suStats.afXyt[0]           = xyt[0];
    suStats.afXyt[1]           = xyt[1];
    suStats.afXyt[2]           = xyt[2];
    suStats.iRT20Status          = rt20Status;
    suStats.iRtcmMsgsRcvdAtGps   = rtcmMsgsRcvdAtGPS;
    suStats.iRtcmMsgsFailedAtGps = rtcmMsgsFailedAtGPS;

    pthread_mutex_unlock(&mutexStatsLock);

    return suStats;
}



void GpsDefault::updateStats_(void)
{
}


/**********************************************************************
# $Log: gps.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.4  2004/09/10 19:41:06  endo
# New PoseCalc integrated.
#
# Revision 1.3  2004/05/02 09:44:42  endo
# *** empty log message ***
#
# Revision 1.2  2004/04/13 19:25:33  yangchen
# JBox module integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.7  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.6  2001/05/29 22:32:26  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.5  2001/03/23 21:33:00  blee
# altered to use a config file
#
# Revision 1.4  2000/12/12 23:21:58  blee
# Changed #defines for the user interface to enumerated types.
#
# Revision 1.3  2000/10/16 19:57:26  endo
# Read buffer problem fixed. Support for CDSB msg added.
#
# Revision 1.2  2000/09/19 03:11:10  endo
# RCS log added.
#
#
#**********************************************************************/

