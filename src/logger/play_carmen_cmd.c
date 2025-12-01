#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "playback_messages.h"
#include "playback_interface.h"

void carmen_ipc_initialize(int argc, char **argv);

int main(int argc, char *argv[])
{
	if(argc!= 3)
	{
		printf("Usage: %s <command> <parameter>\n", argv[0]);
		return 0;
	}

	carmen_ipc_initialize(argc, argv);

	if (strcmp(argv[1], "STOP") == 0)
	    carmen_playback_command(CARMEN_PLAYBACK_COMMAND_STOP, atoi(argv[2]), 1);
	  else if (strcmp(argv[1], "PLAY") == 0)
	    carmen_playback_command(CARMEN_PLAYBACK_COMMAND_PLAY, atoi(argv[2]), 1);
	  else if (strcmp(argv[1], "RRW") == 0)
	    carmen_playback_command(CARMEN_PLAYBACK_COMMAND_REWIND, atoi(argv[2]), 1);
	  else if (strcmp(argv[1], "RW") == 0)
	    carmen_playback_command(CARMEN_PLAYBACK_COMMAND_RWD_SINGLE, atoi(argv[2]), 1);
	  else if (strcmp(argv[1], "FFWD") == 0)
	    carmen_playback_command(CARMEN_PLAYBACK_COMMAND_FORWARD, atoi(argv[2]), 1);
	  else if (strcmp(argv[1], "FWD") == 0)
	    carmen_playback_command(CARMEN_PLAYBACK_COMMAND_FWD_SINGLE, atoi(argv[2]), 1);
	  else if (strcmp(argv[1], "RESET") == 0)
	    carmen_playback_command(CARMEN_PLAYBACK_COMMAND_RESET, atoi(argv[2]), 1);

	return 0;
}
