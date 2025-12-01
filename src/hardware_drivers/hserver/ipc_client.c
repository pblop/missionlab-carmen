#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

#include "ipc_client.h"

const char *centralServer = NULL;
IPC_CONTEXT_PTR centralContext = NULL;

char *missionlab_get_host(void)
{
  FILE *bin_host;
  char hostname[1024];

  if (getenv("HOST") == NULL) {
    if (getenv("HOSTNAME") != NULL)
      setenv("HOST", getenv("HOSTNAME"), 1);
    else if (getenv("host") != NULL)
      setenv("HOST", getenv("host"), 1);
    else if (getenv("hostname") != NULL)
      setenv("HOST", getenv("hostname"), 1);
    else {
      bin_host = popen("/bin/hostname", "r");
      if (bin_host == NULL)
    	  fprintf(stderr, "\n\tCan't get machine name from $HOST, $host, $hostname or /bin/hostname.\n"
		   "\tPlease set one of these environment variables properly.\n\n");
      if (fscanf(bin_host, "%s", hostname) != 1) {
    	  fprintf(stderr, "\n\tCan't get machine name from $HOST, $host, $hostname or /bin/hostname.\n"
		   "\tPlease set one of these environment variables properly.\n\n");
      }
      setenv("HOST", hostname, 1);
      pclose(bin_host);
    }
  }

  return getenv("HOST");
}

void ipc_send_localize_initialize(double x, double y, double theta)
{
	static double last_time = 0;
	static char *hostname = missionlab_get_host();
	struct timeval time;
	double t;
	carmen_point_t mean, std;

	if(ipc_preferred_server_name() == NULL)
		return;

	gettimeofday(&time, NULL);
	t = time.tv_sec + time.tv_usec/1000000.0;

	//if((t - last_time)>IPC_SEND_MIN_TIME)
	{
		IPC_CONTEXT_PTR context = IPC_getContext();
		ipc_connect_preferred_server();

		carmen_localize_initialize_message msg;

		msg.distribution = CARMEN_INITIALIZE_GAUSSIAN;
		msg.num_modes 	= 1;

		mean.x = x;
		mean.y = y;
		mean.theta = theta;
		msg.mean = &mean;

		std.x = std.y = std.theta = 0;
		msg.std = &std;

		msg.timestamp = t;
		msg.host = hostname;

    	IPC_publishData(CARMEN_LOCALIZE_INITIALIZE_NAME, &msg);
    	last_time = t;

		if(context)
			IPC_setContext(context);
	}
}


void ipc_send_simulator_set_truepose(double x, double y, double theta)
{
	static double last_time = 0;
	static char *hostname = missionlab_get_host();
	struct timeval time;
	double t;

	if(ipc_preferred_server_name() == NULL)
		return;

	gettimeofday(&time, NULL);
	t = time.tv_sec + time.tv_usec/1000000.0;

	if((t - last_time)>IPC_SEND_MIN_TIME)
	{
		IPC_CONTEXT_PTR context = IPC_getContext();
		ipc_connect_preferred_server();

		carmen_simulator_set_truepose_message msg;

		msg.pose.x	   = x;
		msg.pose.y     = y;
		msg.pose.theta = theta;

		msg.timestamp = t;
		msg.host = hostname;

    	IPC_publishData(CARMEN_SIMULATOR_SET_TRUEPOSE_NAME, &msg);
    	last_time = t;

		if(context)
			IPC_setContext(context);
	}
}


void ipc_send_base_odometry(double x, double y, double theta, double tv, double rv)
{
	static double last_time = 0;
	static char *hostname = missionlab_get_host();
	struct timeval time;
	double t;

	if(ipc_preferred_server_name() == NULL)
		return;

	gettimeofday(&time, NULL);
	t = time.tv_sec + time.tv_usec/1000000.0;

	if((t - last_time)>IPC_SEND_MIN_TIME)
	{
		IPC_CONTEXT_PTR context = IPC_getContext();
		ipc_connect_preferred_server();

		carmen_base_odometry_message msg;

		msg.x	  = x;
		msg.y     = y;
		msg.theta = theta;

		msg.tv = tv;
		msg.rv = rv;
		msg.acceleration = 0;

		msg.timestamp = t;
		msg.host = hostname;

    	IPC_publishData(CARMEN_BASE_ODOMETRY_NAME, &msg);
    	last_time = t;

		if(context)
			IPC_setContext(context);
	}
}

void ipc_send_base_sonar(int numSonars, float *readings, int lenExtraBytes, float *extra)
{
	static double last_time = 0;
	static char *hostname = missionlab_get_host();
	struct timeval time;
	double t;

	if((numSonars==0) || (ipc_preferred_server_name() == NULL))
		return;

	gettimeofday(&time, NULL);
	t = time.tv_sec + time.tv_usec/1000000.0;

	if((t - last_time)>IPC_SEND_MIN_TIME)
	{
		IPC_CONTEXT_PTR context = IPC_getContext();
		ipc_connect_preferred_server();

		carmen_base_sonar_message msg;
		carmen_point_t *sonar_offsets = new carmen_point_t[numSonars];
		double *range = new double[numSonars];

		int lenExtra = lenExtraBytes/(sizeof(float)*numSonars);

		msg.num_sonars = numSonars;

		if(lenExtra>5)
			msg.cone_angle = extra[5] * M_PI / 180.0;
		else
			msg.cone_angle = 0.5236;

		for(int i=0;i<msg.num_sonars;i++)
		{
			range[i] = readings[i];
			sonar_offsets[i].theta = extra[i*lenExtra+2] * M_PI / 180.0;

			if(lenExtra>4)
			{
				sonar_offsets[i].x = extra[i*lenExtra+3];
				sonar_offsets[i].y = extra[i*lenExtra+4];
			}
			else
			{
				sonar_offsets[i].x = 0;
				sonar_offsets[i].y = 0;
			}
		}

		msg.range = range;
		msg.sonar_offsets = sonar_offsets;
		msg.timestamp = t;
		msg.host = hostname;

    	IPC_publishData(CARMEN_BASE_SONAR_NAME, &msg);
    	last_time = t;

    	delete sonar_offsets;
    	delete range;
		if(context)
			IPC_setContext(context);
	}
}

void ipc_send_localize_globalpos(double x, double y, double theta,
							double odomX, double odomY, double odomTheta)
{
	static double last_time = 0;
	static char *hostname = missionlab_get_host();
	struct timeval time;
	double t;

	if(ipc_preferred_server_name() == NULL)
		return;

	gettimeofday(&time, NULL);
	t = time.tv_sec + time.tv_usec/1000000.0;

	if((t - last_time)>IPC_SEND_MIN_TIME)
	{
		IPC_CONTEXT_PTR context = IPC_getContext();
		ipc_connect_preferred_server();

		carmen_localize_globalpos_message msg;

		msg.globalpos.x		= x;
		msg.globalpos.y     = y;
		msg.globalpos.theta = theta;

		msg.globalpos_std.x     = 0;
		msg.globalpos_std.y     = 0;
		msg.globalpos_std.theta = 0;

		msg.odometrypos.x 		= odomX;
		msg.odometrypos.y 		= odomY;
		msg.odometrypos.theta 	= odomTheta;

		msg.globalpos_xy_cov    = 0;
		msg.converged           = 1;

		msg.timestamp = t;
		msg.host = hostname;

    	IPC_publishData(CARMEN_LOCALIZE_GLOBALPOS_NAME, &msg);
    	last_time = t;

		if(context)
			IPC_setContext(context);
	}
}

void ipc_send_base_velocity(double tv, double rv)
{
	static double last_time = 0;
	static char *hostname = missionlab_get_host();
	struct timeval time;
	double t;

	if(ipc_preferred_server_name() == NULL)
		return;

	gettimeofday(&time, NULL);
	t = time.tv_sec + time.tv_usec/1000000.0;

	if((t - last_time)>IPC_SEND_MIN_TIME)
	{
		IPC_CONTEXT_PTR context = IPC_getContext();
		ipc_connect_preferred_server();

		carmen_base_velocity_message msg;
		msg.tv = tv;
		msg.rv = rv;

		msg.timestamp = t;
		msg.host = hostname;

		IPC_publishData(CARMEN_BASE_VELOCITY_NAME, &msg);
		last_time = t;

		if(context)
			IPC_setContext(context);
	}
}

const char *ipc_preferred_server_name(const char *server)
{
	if(centralServer != NULL)
		return centralServer;

	if((server == NULL) || (strlen(server)==0))
		return NULL;

	centralServer = strdup(server);
	return centralServer;
}

void defineCARMENMsgs()
{
    IPC_defineMsg(CARMEN_LASER_FRONTLASER_NAME, 		IPC_VARIABLE_LENGTH, CARMEN_LASER_LASER_FMT);
    IPC_defineMsg(CARMEN_ROBOT_SONAR_NAME, 				IPC_VARIABLE_LENGTH, CARMEN_ROBOT_SONAR_FMT);
    IPC_defineMsg(CARMEN_ROBOT_VECTOR_MOVE_NAME, 		IPC_VARIABLE_LENGTH, CARMEN_ROBOT_VECTOR_MOVE_FMT);
    IPC_defineMsg(CARMEN_BASE_SONAR_NAME, 				IPC_VARIABLE_LENGTH, CARMEN_BASE_SONAR_FMT);
    IPC_defineMsg(CARMEN_SIMULATOR_SET_TRUEPOSE_NAME, 	IPC_VARIABLE_LENGTH, CARMEN_SIMULATOR_SET_TRUEPOSE_FMT);
    IPC_defineMsg(CARMEN_BASE_ODOMETRY_NAME, 			IPC_VARIABLE_LENGTH, CARMEN_BASE_ODOMETRY_FMT);
    IPC_defineMsg(CARMEN_LOCALIZE_GLOBALPOS_NAME, 		IPC_VARIABLE_LENGTH, CARMEN_LOCALIZE_GLOBALPOS_FMT);
    IPC_defineMsg(CARMEN_SIMULATOR_TRUEPOS_NAME, 		IPC_VARIABLE_LENGTH, CARMEN_SIMULATOR_TRUEPOS_FMT);
    IPC_defineMsg(CARMEN_BASE_VELOCITY_NAME, 			IPC_VARIABLE_LENGTH, CARMEN_BASE_VELOCITY_FMT);
    IPC_defineMsg(CARMEN_LOCALIZE_INITIALIZE_NAME, 		IPC_VARIABLE_LENGTH, CARMEN_LOCALIZE_INITIALIZE_FMT);
    IPC_defineMsg(CARMEN_GLOBAL_HOOK_MSG_NAME, 			IPC_VARIABLE_LENGTH, CARMEN_GLOBAL_HOOK_MSG_FMT);
}

int ipc_connect_preferred_server(const char *server)
{
	if(centralContext != NULL)
	{
		IPC_setContext(centralContext);
		return IPC_ALREADY_CONNECTED;
	}

	if(centralServer != NULL)
	{
		if(IPC_connectModule("hserver", centralServer)!=IPC_OK)
				return IPC_CONNECTION_ERROR;
		centralContext = IPC_getContext();
		IPC_setCapacity(4);
		defineCARMENMsgs();

		//Take control of the robot hooking key messages
	    ipc_hook_message(CARMEN_BASE_ODOMETRY_NAME, MISSIONLAB_BASE_ODOMETRY_NAME, CARMEN_BASE_ODOMETRY_FMT);
	    ipc_hook_message(CARMEN_BASE_SONAR_NAME, MISSIONLAB_BASE_SONAR_NAME, CARMEN_BASE_SONAR_FMT);
	    ipc_hook_message(CARMEN_BASE_VELOCITY_NAME, MISSIONLAB_BASE_VELOCITY_NAME, CARMEN_BASE_VELOCITY_FMT);
	    ipc_hook_message(CARMEN_LOCALIZE_GLOBALPOS_NAME, MISSIONLAB_LOCALIZE_GLOBALPOS_NAME,
	    		CARMEN_LOCALIZE_GLOBALPOS_FMT);

	    return IPC_CONNECTION_SUCCESS;
	}

	if((server == NULL) || (strlen(server)==0))
		server = "127.0.0.1:1381";

	if(IPC_connectModule("hserver", server)!=IPC_OK)
		return IPC_CONNECTION_ERROR;

	centralServer = strdup(server);
	centralContext = IPC_getContext();
	IPC_setCapacity(4);

	//Take control of the robot hooking key messages
	ipc_hook_message(CARMEN_BASE_ODOMETRY_NAME, MISSIONLAB_BASE_ODOMETRY_NAME, CARMEN_BASE_ODOMETRY_FMT);
    ipc_hook_message(CARMEN_BASE_VELOCITY_NAME, MISSIONLAB_BASE_VELOCITY_NAME, CARMEN_BASE_VELOCITY_FMT);
    ipc_hook_message(CARMEN_LOCALIZE_GLOBALPOS_NAME, MISSIONLAB_LOCALIZE_GLOBALPOS_NAME,
    		CARMEN_LOCALIZE_GLOBALPOS_FMT);

    return IPC_CONNECTION_SUCCESS;
}


void ipc_hook_message(char *name, char *newName, char *format)
{
	struct timeval tv;
	double t;
    carmen_global_hook_message hookMessage;

	gettimeofday(&tv, NULL);
	t = tv.tv_sec + tv.tv_usec/1000000.0;

    IPC_defineMsg(newName, IPC_VARIABLE_LENGTH, format);
    hookMessage.message_name = name;
    hookMessage.new_message_name = newName;
    hookMessage.timestamp = t;
    hookMessage.host = missionlab_get_host();

    IPC_publishData(CARMEN_GLOBAL_HOOK_MSG_NAME, &hookMessage);
}
