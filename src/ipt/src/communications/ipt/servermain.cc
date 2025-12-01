///////////////////////////////////////////////////////////////////////////////
//
//                                 server.cc
//
// This file declares the is the main program for the IPT server
// 
// Usage
//    The program takes several command line switchs
//       -d <domain name> 
//            gives the server the domain name
//       -m <message file>
//            you can give the server a file consisting of a list of message
//            names to avoid going to the domain master for interdomain 
//            messages as is the norm.  By giving the same file to all 
//            servers you can assure system wide consistent mappings for
//            the messages listed in the file.
//       -l <log file>
//
//    The program also takes one environment variable, IPTMASTER.  If IPTMASTER
//    is unset, the program assumes it is the IPTMASTER.  If IPTMASTER is set
//    the program assumes it is set to the name of the machine on which 
//    the IPT master server is running.  It then connects to the master server
//    and uses it for doing the message names->ID mappings it doesn't already
//    know about and also uses the master server to tell it where the servers
//    for other domains are.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include "./libc.h"

#define MAX_PEERS 100

#include <ipt/server.h>
#include <ipt/internal_messages.h>

static IPServer* _Server = NULL;

void killHnd(int)
{
    printf("IPT(Server): Cleaning up\n");
    delete _Server;
    exit(0);
}

/* main routine for IPT server */
main(int argc, char** argv)
{
    char* domain_name = NULL;
    char* message_file = NULL;
    char* log_file = NULL;
    int num_peers = 0;
    IPDomainSpec peers[MAX_PEERS+1];

    int i;
    for (i=1;i<argc;i++) 
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'd') {
                if (++i < argc)
                    domain_name = argv[i];
            } else if (argv[i][1] == 'm') {
                if (++i < argc)
                    message_file = argv[i];
            } else if (argv[i][1] == 'l') {
                if (++i < argc)
                    log_file = argv[i];
            } else if (argv[i][1] == 'C') {
                if (++i < argc)
                    peers[num_peers].name = argv[i];
                if (++i < argc)
                    peers[num_peers++].host = argv[i];
            }
        }

    peers[num_peers].name = peers[num_peers].host = NULL;
    
    _Server = new IPServer(SERVER_NAME, message_file, 
                           domain_name, peers, log_file);

    // Typecast is no longer needed or correct in RedHat 7.1 (endo)
    //signal(SIGPIPE, (SIG_PF) killHnd);
    signal(SIGPIPE, killHnd);

    _Server->MainLoop();

    return 0;
}
