#ifndef GPS_DEFAULT_H
#define GPS_DEFAULT_H

#include "gps.h"

class GpsDefault : public Gps
{

protected:
	IPHandlerCallback* callback;
	IPCommunicator* communicator;
	IPConnection* gps_connection;
	pthread_mutex_t serialMutex;
	pthread_t reader;
	pthread_t base_reader;
	pthread_mutex_t mutexgps2jbox;
	sem_t connect_sem;
	bool connected, listenIpt;
	int max_retries;
	double base_longitude;
	double base_latitude;
	bool base_relay;
	int solutionStatus;
	int gpsFd;
	int rtcmBufSize;
	char* rtcmBuf;
	int rtcmMsgsRcvdAtGPSbeforeSession;
	int rtcmMsgsRcvdAtGPS;
	int rtcmMsgsFailedAtGPSbeforeSession;
	int rtcmMsgsFailedAtGPS;
	int firstCDSB;
	int bLogBufSize;
	char* bLogBuf;
	char* sendBuf;
	int sendSize;
	char* rcvBuf;
	int rcvSize;
	char* flushBuf;
	int flushSize;
	char msg2jbox[100];

	void sendRtcmMessage( int size, char* data );
	void callbackRTCM( IPMessage* message );
	int setupIpt();
	void shutdownIpt();
	char* idString( int id );
	char* RT20StatusString( int status );
	char* solutionString( int status );
	void decodeBLog( int id );
	int readN( char* buf, int n, bool no_timeout = false );
	void printBLogHeader( bLogHeader* h );
	void printP20B( logP20B* l );
	void printSPHB( logSPHB* l );
	void printCDSB( logCDSB* l );
	void flush();
	int getLine();
	int initialize( const string& strPortString );
	int initializeBase( const string& strPortString );
	void send( char* str );
	int readBLog();
	void readRtcm();
	void LatLon2XY( double dLatitude, double dLongitude );
	void updateStats_(void);
	void updateStatusBar_(void);
	void logData_(double logTime);

    static const int WATCHDOG_CHECK_INTERVAL_SEC_;


public:
	void update_jbox(char* gps_string);
	void reader_thread();
	void base_reader_thread();
	SuGpsStats GetStats();

	//Undefined??
	void control();


    GpsDefault( Gps** a, const string& strPortString, double base_lat, double base_lon, double base_x,
         double base_y, double mperlat, double mperlon, bool base = false);
    ~GpsDefault();


};


#endif //GPS_DEFAULT_H

