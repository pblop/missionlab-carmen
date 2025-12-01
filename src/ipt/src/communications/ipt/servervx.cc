#include <stdio.h>

#include <ipt/server.h>
#include <ipt/internal_messages.h>

#define MAX_PEERS 100

extern "C" {

static int _server_done = -1;

void iptRunServer(char* log_file, char* domain_name,
                  const int num_peers, char** peer_names, char** peer_hosts)
{
  int i;
  IPDomainSpec peers[MAX_PEERS];

  if (_server_done != -1) {
    printf("IPT Server(Vx):  Cannot be run two times without exiting\n");
    return;
  }

  _server_done = 0;

  for (i=0;i<num_peers;i++) {
    peers[num_peers].name = peer_names[i];
    peers[num_peers].host = peer_hosts[i];
  }
  peers[num_peers].name = peers[num_peers].host = NULL;

  IPServer* server = new IPServer(SERVER_NAME, NULL, domain_name,
                                  peers, log_file);

  while (_server_done == 0) {
    server->ScanForMessages(0.3);
    server->ProcessEvents();
  }

  printf("IPT Server(Vx): Exiting\n");

  delete server;

  _server_done = -1;
}

void iptKillServer()
{
  if (_server_done == -1) {
    printf("IPT Server(Vx): Server never started\n");
    return;
  }

  if (_server_done == 1) {
    printf("IPT Server(vx): Already attempting to stop server, being ignored\n");
    return;
  }

  _server_done = 1;
}

void iptResetServer()
{
  if (_server_done != -1) {
    printf("IPT Server(Vx): Server never started, resetting anyway\n");
  }

  if (_server_done == 1) {
    printf("IPT Server(vx): Already attempting to stop server, being ignored, resetting anyway\n");
  }

  _server_done = -1;
}

}
