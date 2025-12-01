/**********************************************************************
 **                                                                  **
 **  operatorConsole - text interface for controlling the CMDLi      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1997, Mobile Intelligence Corporation                 **
 **  33150 Schoolcraft Road, Suite 108                               **
 **  Livonia, Michigan  48150-1646                                   **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: operatorConsole.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: operatorConsole.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:14:58  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.6  2004/11/12 21:55:16  doug
* tweaks for final Ft. Benning
*
* Revision 1.5  2004/06/11 18:18:40  doug
* move to sara namespace
*
* Revision 1.4  2004/04/30 17:36:01  doug
* works for UPenn
*
* Revision 1.3  2004/04/27 20:04:35  doug
* working on opcon
*
* Revision 1.2  2004/04/27 18:36:07  doug
* fixed build problems with win32
*
* Revision 1.1  2004/04/27 16:21:45  doug
* support an operator console
*
**********************************************************************/

#include "mic.h"
#include "myGetOpt.h"
#include "MsgTypes.h"
#include "objcomms.h"
#include "CMDLcommand.h"
#include "CMDLcommandAck.h"
#undef ERROR

namespace sara
{
// declare a pointer to the communications subsystem.
objcomms *comobj = NULL;

/// the addresses of all the team members, only used if multicastIsUnicast is true
typedef vector< ComAddr > addrList;
addrList teamAddrs;

bool verbose = false;

// *********************************************************************
/// generate unique message IDs
static uword nextID = 999;

// timeouts
static const TimeOfDay AckMsg_Timeout(1.0);
static const int Max_MsgSendRetries = 10;

// ****************************************************************************
// Provide usage information for the command line parameters
static void usage(char *name)
{
   fprintf(stderr,"usage: %s [-a xx.yy.zz.255] [-C bitflags] [-p yy] [-r xx] [-u filename] [-v]\n",name);
   fprintf(stderr,"       -a xx.yy.zz.255   : set the multicast IP address.\n");
   fprintf(stderr,"               The default is %s\n", "234.5.6.7");
   fprintf(stderr,"       -C bitflags       : set the debug bit flags for the communications subsystem\n");
   fprintf(stderr,"                           1 = transmitter debug\n");
   fprintf(stderr,"                           2 = receiver debug\n");
   fprintf(stderr,"                           4 = receiver detailed debug\n");
   fprintf(stderr,"       -p yy : set the port number to use for communications\n");
   fprintf(stderr,"               Ports yy through yy+xx will be used\n");
   fprintf(stderr,"               The default is %d\n", 10827);
   fprintf(stderr,"       -r xx : set the number of robots per processor\n");
   fprintf(stderr,"               Note: the console also counts as one instance!\n");
   fprintf(stderr,"       -u filename       : load a list of addresses from the file to use for multicasting\n");
   fprintf(stderr,"       -v                : enable verbose mode\n");
   fprintf(stderr,"\n");
}

// ****************************************************************************
// provide reliable sends
// return true if all receipients acknowledged the message
//        false otherwise. 
// Loads responded with true for each sender that acked the message and false for the others
typedef map< ComAddr, bool > responses;
bool reliableSend(const CMDLcommand &cmd, responses &responded)
{
   // Register for acks 
   objcomms::QueueingHandle AckCommandHandle = NULL;
   if( AckCommandHandle == NULL )
   {
	    AckCommandHandle = comobj->beginQueueing(MsgTypes::CMDLCOMMANDACK);
	    if( AckCommandHandle == NULL )
	    {
	       ERROR("Internal Error: beginQueueing failed - Unable to register for CMDLcommandAck messages!");
               exit(1);
	    }
   }

   TimeOfDay endTime;
   TimeOfDay remainingTime;
   TimeOfDay timeOfTaskStatus;

         // The current retry
         int retry = 0;
         bool failed = false;
         bool msgsDone = false;
         enum {S_SEND=0, S_WAITFORACKS} curstate = S_SEND;
         for(uint i=0; i<teamAddrs.size(); ++i)
         {
            responded[ teamAddrs[i] ] = false;
	 }
         int numFalse = teamAddrs.size();

         while( !msgsDone )
         {
            switch(curstate)
            {
               case S_SEND:
	          // send/re-send the command to robots that have not acked yet
                  for(uint i=0; i<teamAddrs.size(); ++i)
                  {
                     if( !responded[teamAddrs[i]] )
                     {
                        if( !comobj->sendMsg(teamAddrs[i], cmd) )
	                {
	                   ERROR("Internal Error: beginQueueing failed - Unable to register for CMDLcommandAck messages!");
                           exit(1);
	                }
                     }
                  }

	          // init the timer
	          endTime = TimeOfDay::now() + AckMsg_Timeout;
	          remainingTime = AckMsg_Timeout;

	          curstate = S_WAITFORACKS;
                  break;

               case S_WAITFORACKS:
               {
                  commMsg *msg = comobj->waitFor(AckCommandHandle, remainingTime);
	          if( msg )
	          {
	             // Got one
                     CMDLcommandAck *ack = dynamic_cast<CMDLcommandAck *>(msg);
                     if( ack == NULL )
                     {
                        cerr << "Unable to upcast commMsg to CMDLcommandAck!" << endl;
			delete msg;
		     }
		     else if( ack->opcon != cmd.opcon || ack->opconID != cmd.opconID )
		     {
	                if( verbose )
	                {
                           cerr << "Ignoring extranous ack." << endl;
	                }
	                delete msg;
	             }
	             else
	             {
	                if( ack->response != CMDLcommandAck::OK )
	                {
                           cerr << "robot " << ack->senderAddr.str() << " return a response of ";
                           switch( ack->response )
                           {
                              case CMDLcommandAck::INVALID:
                                 cerr << "FAILED" << endl;
                                 break;

                              case CMDLcommandAck::FAILED:
                                 cerr << "FAILED" << endl;
                                 break;

                              case CMDLcommandAck::OK:
                                 cerr << "OK" << endl;
                                 break;
                           }
	                }

	                if( verbose )
	                {
                           cerr << "got an ack from " << ack->senderAddr.str() << endl;
	                }
         
                        // good one.  Figure out who it was from
                        responses::iterator it = responded.find( ack->senderAddr );
                        if( it == responded.end() )
	                {
                           cerr << "ignoring ack from " << ack->senderAddr.str() << " - not from one of our team members!" << endl;
	                }
                        else if( it->second == false )
                        {
                           responded[ ack->senderAddr ] = true;
                           numFalse --;
                        }
                         
                        // If that is the last one, then we are done.
                        if( numFalse <= 0 ) 
                        {
                           msgsDone = true;
                           break;
                        }
	             }
	          }

                  // If time has expired, and we don't have them all, then check if we have retries left.
	          remainingTime = endTime - TimeOfDay::now();
	          if( !remainingTime.isTimeLeft() )
	          {
                     if( ++retry < Max_MsgSendRetries )
                     {
                        fprintf(stderr,".");
                        curstate = S_SEND;
                     }
                     else
                     {
                        // nope, out of luck
                        failed = true;
                        msgsDone = true;
                     }
	          }
               }
               break;
            }
         }

         // Cleanup
         if( AckCommandHandle )
         {
            comobj->endQueueing(AckCommandHandle);
            AckCommandHandle = NULL;
         }

   return !failed;
}


// *********************************************************************
int 
main(int argc, char **argv)
{
   uint max_robots_per_machine = 1;
   uint broadcast_port = 50346;
   string broadcast_IP = "234.5.6.7";
   uint comms_debug_flags = 0;
   bool multicastIsUnicast = false;
   strings addressTable;
   bool useCompression = true;
   bool useEncryption = false;
   string encryptionKey = "This is a weak key";

   GetOpt getopt(argc, argv, "a:C:p:r:u:v");
   int c;
   while((c=getopt()) != EOF)
   {
      switch(c)
      {
         case 'a':
            broadcast_IP = getopt.optarg;
            if( verbose )
               fprintf(stderr,"Using broadcast address %s for communications\n", broadcast_IP.c_str());
            break;

         case 'C':
            comms_debug_flags = (uint)atoi(getopt.optarg);
            if( verbose )
               fprintf(stderr,"Communications debug flags set to %d\n", comms_debug_flags);
            break;

         case 'p':
            broadcast_port = atoi(getopt.optarg);
            if( verbose )
               fprintf(stderr,"Using port %d for communications\n",broadcast_port);
            break;

         case 'r':
            max_robots_per_machine = atoi(getopt.optarg);
            if( verbose )
               fprintf(stderr,"Supporting %d robots per processor\n",max_robots_per_machine);
            break;

	 case 'u':
         {
            if( verbose )
            {
               fprintf(stderr,"Loading address file and sending multicast as unicast\n");
            }

            multicastIsUnicast = true;
            string filename = getopt.optarg;

            // Create an input stream from the file
            FILE *input = fopen( filename.c_str(), "r" );
            if( !input )
            {
               cerr << "Unable to open the address file " << filename << " for reading!" << endl;
               exit(1);
            }

            // Read it in.
            while( !feof(input) )
            {
               char buf[2048];
               fscanf(input,"%s", buf);
               addressTable.push_back( buf );

               if( verbose )
               {
                  fprintf(stderr," adding address '%s'\n", buf);
               }
            }
	    break;
         }

	 case 'v':
            verbose = true; 
	    break;

	 default:
	    usage(argv[0]);
	   exit(1);
      }
   }

   if( addressTable.size() == 0 )
   {
      string at_filename("addressTable.txt");

      int optIndex = getopt.optind;
      if( optIndex < argc )
      {
         at_filename = argv[optIndex];
      }

      cerr << "Loading address table from " << at_filename << endl;

      // Create an input stream from the file
      FILE *input = fopen( at_filename.c_str(), "r" );
      if( !input )
      {
         cerr << "Unable to open the address file " << at_filename << " for reading!" << endl;
         exit(1);
      }

      // Read it in.
      while( !feof(input) )
      {
         char buf[2048];
         int rtn = fscanf(input,"%s", buf);
         if( rtn > 0 )
         {
            addressTable.push_back( buf );
   
            if( verbose )
            {
               fprintf(stderr," adding address '%s'\n", buf);
            }
         }
      }
   }

   for(uint i=0; i<addressTable.size(); ++i)
   {
      teamAddrs.push_back( ComAddr( addressTable[i] ) );
   }

   // Start the communications subsystem.
   comobj = new objcomms();

   // if multicastIsUnicast, pass on the address table
   if( multicastIsUnicast )
   {
      if( !comobj->loadMemberAddresses(addressTable) )
      {
         FATAL_ERROR("Unable to set multicastIsUnicast");
      }
   }

   char robot_name[80] = "opcon";
   if( !comobj->open(broadcast_IP, broadcast_port, robot_name, max_robots_per_machine) )
   {
      FATAL_ERROR("Unable to inititialize communications system");
   }

   // set the debug level
   if( comms_debug_flags )
   {
      if( verbose )
         cerr << "communications debugging set to " << comms_debug_flags << endl;
      comobj->setDebug(comms_debug_flags);
   }

   // Set compression on or off
   if( useCompression )
   {
      if( verbose )
         cerr << "compression turned on" << endl;
      comobj->enableCompression();
   }
   else
   {
      if( verbose )
         cerr << "compression turned off" << endl;
      comobj->disableCompression();
   }

   // set encryption on or off
   if( useEncryption )
   {
      if( verbose )
         cerr << "encryption turned on with key '" << encryptionKey << "'" << endl;
      comobj->enableEncryption(encryptionKey);
   }
   else
   {
      if( verbose )
         cerr << "encryption turned off" << endl;
      comobj->disableEncryption();
   }

   // Register the packets we will use
   comobj->attachHandler(MsgTypes::CMDLCOMMAND, CMDLcommand::from_stream);
   comobj->attachHandler(MsgTypes::CMDLCOMMANDACK, CMDLcommandAck::from_stream);

   // provide a simple user interface
   bool done = false;
   char cmd[2048];
   char arg[2048];
   while( !done )
   {
      printf("Enter a command then press enter:\n");
      printf("  d filename.cmdl -  downloads a mission file\n");
      printf("  e                 -  execute the last mission downloaded\n");
      printf("  s                 -  stop execution\n");
//      printf("  p                 -  pause execution\n");
//      printf("  r                 -  resume execution\n");
      printf("  q                 -  quit this program\n");
      printf("> ");

      cmd[0] = '\0';
      arg[0] = '\0';
      int num = scanf("%s", cmd);
      if( num > 0 && (cmd[0] == 'd' || cmd[0] == 'D'))
      {
         num += scanf("%s", arg);
      }
//      printf("\nnum=%d  cmd='%s'  arg='%s'\n", num, cmd, arg);

      if( num < 1 )
	 continue;

      if( cmd[0] == 'q' || cmd[0] == 'Q' )
      {
         done = true;
      }
      else if( cmd[0] == 'd' || cmd[0] == 'D' )
      {
	 FILE *fh = fopen(arg, "r");
         if( !fh )
         {
            WARN_with_perror("Unable to open file '%s'\n", arg);
            continue;
         }

         stringstream data;
         int ch;
         while( (ch = fgetc(fh)) != EOF )
         {
            char c = (char)ch;
            data << c;
         }

         // build the message
         CMDLcommand cmd;
         cmd.opcon = comobj->getName();
         cmd.opconID = nextID++;
         cmd.action = CMDLcommand::CMD_DOWNLOAD;
         cmd.data = data.str();

/*
         // Register for acks 
         objcomms::QueueingHandle AckCommandHandle = NULL;
	 if( AckCommandHandle == NULL )
	 {
	    AckCommandHandle = comobj->beginQueueing(MsgTypes::CMDLCOMMANDACK);
	    if( AckCommandHandle == NULL )
	    {
	       ERROR("Internal Error: beginQueueing failed - Unable to register for CMDLcommandAck messages!");
               exit(1);
	    }
	 }


         TimeOfDay endTime;
         TimeOfDay remainingTime;
         TimeOfDay timeOfTaskStatus;

         // The current retry
         int retry = 0;
         bool failed = false;
         bool msgsDone = false;
         enum {S_SEND=0, S_WAITFORACKS} curstate = S_SEND;
         for(uint i=0; i<teamAddrs.size(); ++i)
         {
            responded[ teamAddrs[i] ] = false;
	 }
         int numFalse = teamAddrs.size();

         while( !msgsDone )
         {
            switch(curstate)
            {
               case S_SEND:
	          // send/re-send any solicitations that have not received bids
                  for(uint i=0; i<teamAddrs.size(); ++i)
                  {
                     if( !responded[teamAddrs[i]] )
                     {
                        if( !comobj->sendMsg(teamAddrs[i], cmd) )
	                {
	                   ERROR("Internal Error: beginQueueing failed - Unable to register for CMDLcommandAck messages!");
                           exit(1);
	                }
                     }
                  }

	          // init the timer
	          endTime = TimeOfDay::now() + AckMsg_Timeout;
	          remainingTime = AckMsg_Timeout;

	          curstate = S_WAITFORACKS;
                  break;

               case S_WAITFORACKS:
               {
                  commMsg *msg = comobj->waitFor(AckCommandHandle, remainingTime);
	          if( msg )
	          {
	             // Got one
                     CMDLcommandAck *ack = dynamic_cast<CMDLcommandAck *>(msg);
                     if( ack == NULL )
                     {
                        cerr << "Unable to upcast commMsg to CMDLcommandAck!" << endl;
			delete msg;
		     }
		     else if( ack->opcon != cmd.opcon || ack->opconID != cmd.opconID )
		     {
	                if( verbose )
	                {
                           cerr << "Ignoring extranous ack." << endl;
	                }
	                delete msg;
	             }
	             else
	             {
	                if( verbose )
	                {
                           cerr << "got an ack from " << ack->senderAddr.str() << endl;
	                }
         
                        // good one.  Figure out who it was from
                        responses::iterator it = responded.find( ack->senderAddr );
                        if( it == responded.end() )
	                {
                           cerr << "ignoring ack from " << ack->senderAddr.str() << " - not from one of our team members!" << endl;
	                }
                        else if( it->second == false )
                        {
                           it->second == true;
                           numFalse --;
                        }
                         
                        // If that is the last one, then we are done.
                        if( numFalse <= 0 ) 
                        {
                           msgsDone = true;
                        }
	             }
	          }

                  // If time has expired, and we don't have them all, then check if we have retries left.
	          remainingTime = endTime - TimeOfDay::now();
	          if( !remainingTime.isTimeLeft() )
	          {
                     if( ++retry < Max_MsgSendRetries )
                     {
                        fprintf(stderr,".");
                        curstate = S_SEND;
                     }
                     else
                     {
                        // nope, out of luck
                        failed = true;
                        msgsDone = true;
                     }
	          }
               }
               break;
            }
         }

         // Cleanup
         if( AckCommandHandle )
         {
            comobj->endQueueing(AckCommandHandle);
            AckCommandHandle = NULL;
         }
*/
         responses responded;
         bool ok = reliableSend(cmd, responded);

         if( !ok )
         {
            printf("\ndownload FAILED.  Unable to talk to robots:\n");
            responses::iterator it;
            for(it=responded.begin(); it!=responded.end(); ++it)
            {
               if( !it->second )
               {
                  cerr << "   " << it->first.str() << endl;
               }
            }
            printf("\n");
         }
         else
         {
            printf("\ndownload succeeded!\n");
         }
      }
      else if( cmd[0] == 'e' || cmd[0] == 'E' )
      {
         // build the message
         CMDLcommand cmd;
         cmd.opcon = comobj->getName();
         cmd.opconID = nextID++;
         cmd.action = CMDLcommand::CMD_START;
/*
         // Register for acks 
         objcomms::QueueingHandle AckCommandHandle = NULL;
	 if( AckCommandHandle == NULL )
	 {
	    AckCommandHandle = comobj->beginQueueing(MsgTypes::CMDLCOMMANDACK);
	    if( AckCommandHandle == NULL )
	    {
	       ERROR("Internal Error: beginQueueing failed - Unable to register for CMDLcommandAck messages!");
               exit(1);
	    }
	 }

         TimeOfDay endTime;
         TimeOfDay remainingTime;
         TimeOfDay timeOfTaskStatus;

         // The current retry
         int retry = 0;
         bool failed = false;
         bool msgsDone = false;
         enum {S_SEND=0, S_WAITFORACKS} curstate = S_SEND;
         typedef map< ComAddr, bool > responses;
         responses responded;
         for(uint i=0; i<teamAddrs.size(); ++i)
         {
            responded[ teamAddrs[i] ] = false;
	 }
         int numFalse = teamAddrs.size();

         while( !msgsDone )
         {
            switch(curstate)
            {
               case S_SEND:
	          // send/re-send the command to robots that have not acked yet
                  for(uint i=0; i<teamAddrs.size(); ++i)
                  {
                     if( !responded[teamAddrs[i]] )
                     {
                        if( !comobj->sendMsg(teamAddrs[i], cmd) )
	                {
	                   ERROR("Internal Error: beginQueueing failed - Unable to register for CMDLcommandAck messages!");
                           exit(1);
	                }
                     }
                  }

	          // init the timer
	          endTime = TimeOfDay::now() + AckMsg_Timeout;
	          remainingTime = AckMsg_Timeout;

	          curstate = S_WAITFORACKS;
                  break;

               case S_WAITFORACKS:
               {
                  commMsg *msg = comobj->waitFor(AckCommandHandle, remainingTime);
	          if( msg )
	          {
	             // Got one
                     CMDLcommandAck *ack = dynamic_cast<CMDLcommandAck *>(msg);
                     if( ack == NULL )
                     {
                        cerr << "Unable to upcast commMsg to CMDLcommandAck!" << endl;
			delete msg;
		     }
		     else if( ack->opcon != cmd.opcon || ack->opconID != cmd.opconID )
		     {
	                if( verbose )
	                {
                           cerr << "Ignoring extranous ack." << endl;
	                }
	                delete msg;
	             }
	             else
	             {
	                if( verbose )
	                {
                           cerr << "got an ack from " << ack->senderAddr.str() << endl;
	                }
         
                        // good one.  Figure out who it was from
                        responses::iterator it = responded.find( ack->senderAddr );
                        if( it == responded.end() )
	                {
                           cerr << "ignoring ack from " << ack->senderAddr.str() << " - not from one of our team members!" << endl;
	                }
                        else if( it->second == false )
                        {
                           responded[ ack->senderAddr ] = true;
                           numFalse --;
cerr << "Got one.  numFalse =" << numFalse << endl;
                        }
                         
                        // If that is the last one, then we are done.
                        if( numFalse <= 0 ) 
                        {
cerr << "all done" << endl;
                           msgsDone = true;
                           break;
                        }
	             }
	          }

                  // If time has expired, and we don't have them all, then check if we have retries left.
	          remainingTime = endTime - TimeOfDay::now();
	          if( !remainingTime.isTimeLeft() )
	          {
                     if( ++retry < Max_MsgSendRetries )
                     {
                        fprintf(stderr,".");
                        curstate = S_SEND;
                     }
                     else
                     {
                        // nope, out of luck
                        failed = true;
                        msgsDone = true;
                     }
	          }
               }
               break;
            }
         }

         // Cleanup
         if( AckCommandHandle )
         {
            comobj->endQueueing(AckCommandHandle);
            AckCommandHandle = NULL;
         }
*/
         responses responded;
         bool ok = reliableSend(cmd, responded);

         if( !ok )
         {
            printf("\nstart FAILED.  Unable to talk to robots:\n");
            responses::iterator it;
            for(it=responded.begin(); it!=responded.end(); ++it)
            {
               if( !it->second )
               {
                  cerr << "   " << it->first.str() << endl;
               }
            }
            printf("\n");
         }
         else
         {
            printf("\nstart succeeded!\n");
         }
      }
      else if( cmd[0] == 's' || cmd[0] == 'S' )
      {
         // build the message
         CMDLcommand cmd;
         cmd.opcon = comobj->getName();
         cmd.opconID = nextID++;
         cmd.action = CMDLcommand::CMD_STOP;
/*
         // Register for acks 
         objcomms::QueueingHandle AckCommandHandle = NULL;
	 if( AckCommandHandle == NULL )
	 {
	    AckCommandHandle = comobj->beginQueueing(MsgTypes::CMDLCOMMANDACK);
	    if( AckCommandHandle == NULL )
	    {
	       ERROR("Internal Error: beginQueueing failed - Unable to register for CMDLcommandAck messages!");
               exit(1);
	    }
	 }

         TimeOfDay endTime;
         TimeOfDay remainingTime;
         TimeOfDay timeOfTaskStatus;

         // The current retry
         int retry = 0;
         bool failed = false;
         bool msgsDone = false;
         enum {S_SEND=0, S_WAITFORACKS} curstate = S_SEND;
         typedef map< ComAddr, bool > responses;
         responses responded;
         for(uint i=0; i<teamAddrs.size(); ++i)
         {
            responded[ teamAddrs[i] ] = false;
	 }
         int numFalse = teamAddrs.size();

         while( !msgsDone )
         {
            switch(curstate)
            {
               case S_SEND:
	          // send/re-send any solicitations that have not received bids
                  for(uint i=0; i<teamAddrs.size(); ++i)
                  {
                     if( !responded[teamAddrs[i]] )
                     {
                        if( !comobj->sendMsg(teamAddrs[i], cmd) )
	                {
	                   ERROR("Internal Error: beginQueueing failed - Unable to register for CMDLcommandAck messages!");
                           exit(1);
	                }
                     }
                  }

	          // init the timer
	          endTime = TimeOfDay::now() + AckMsg_Timeout;
	          remainingTime = AckMsg_Timeout;

	          curstate = S_WAITFORACKS;
                  break;

               case S_WAITFORACKS:
               {
                  commMsg *msg = comobj->waitFor(AckCommandHandle, remainingTime);
	          if( msg )
	          {
	             // Got one
                     CMDLcommandAck *ack = dynamic_cast<CMDLcommandAck *>(msg);
                     if( ack == NULL )
                     {
                        cerr << "Unable to upcast commMsg to CMDLcommandAck!" << endl;
			delete msg;
		     }
		     else if( ack->opcon != cmd.opcon || ack->opconID != cmd.opconID )
		     {
	                if( verbose )
	                {
                           cerr << "Ignoring extranous ack." << endl;
	                }
	                delete msg;
	             }
	             else
	             {
	                if( verbose )
	                {
                           cerr << "got an ack from " << ack->senderAddr.str() << endl;
	                }
         
                        // good one.  Figure out who it was from
                        responses::iterator it = responded.find( ack->senderAddr );
                        if( it == responded.end() )
	                {
                           cerr << "ignoring ack from " << ack->senderAddr.str() << " - not from one of our team members!" << endl;
	                }
                        else if( it->second == false )
                        {
                           it->second == true;
                           numFalse --;
                        }
                         
                        // If that is the last one, then we are done.
                        if( numFalse <= 0 ) 
                        {
                           msgsDone = true;
                           break; 
                        }
	             }
	          }

                  // If time has expired, and we don't have them all, then check if we have retries left.
	          remainingTime = endTime - TimeOfDay::now();
	          if( !remainingTime.isTimeLeft() )
	          {
                     if( ++retry < Max_MsgSendRetries )
                     {
                        fprintf(stderr,".");
                        curstate = S_SEND;
                     }
                     else
                     {
                        // nope, out of luck
                        failed = true;
                        msgsDone = true;
                     }
	          }
               }
               break;
            }
         }

         // Cleanup
         if( AckCommandHandle )
         {
            comobj->endQueueing(AckCommandHandle);
            AckCommandHandle = NULL;
         }
*/
         responses responded;
         bool ok = reliableSend(cmd, responded);

         if( !ok )
         {
            printf("\nstop FAILED.  Unable to talk to robots:\n");
            responses::iterator it;
            for(it=responded.begin(); it!=responded.end(); ++it)
            {
               if( !it->second )
               {
                  cerr << "   " << it->first.str() << endl;
               }
            }
            printf("\n");
         }
         else
         {
            printf("\nstop succeeded!\n");
         }
      }

   }

   return 0;
}

// *********************************************************************
}
