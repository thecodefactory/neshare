/*
  NEshare is a peer-to-peer file sharing toolkit.
  Copyright (C) 2001, 2002 Neill Miller
  This file is part of NEshare.
  
  NEshare is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  NEshare is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with NEshare; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "neclientheaders.h"

#define MAX_LOGIN_MSG_LEN       0x00002000
#define MAX_DISCONNECT_MSG_LEN  0x00000100

/* a global pointer to a valid client connection object */
neClientConnection *g_clientConnection = (neClientConnection *)0;

/* 
   This function handles all incoming message from the server.

   returns 0 if no messages are waiting;
   returns 1 if a message was handled.
   returns 2 if login ack was retrieved.
   returns 3 if disconnect ack is retrieved.
*/
void *handleIncomingMessage(void *socket)
{
   unsigned long msgType = 0;
   unsigned long numUsers = 0;
   unsigned long numFiles = 0;
   unsigned long numMB = 0;
   unsigned long numResults = 0;
   unsigned long ipAddr = 0;
   unsigned long connection = 0;
   unsigned long firewallStatus = 0;
   unsigned long filesize = 0;
   unsigned long controlPort = 0;
   unsigned long id = 0;
   static unsigned long connected = 1;

   struct in_addr inAddr;
   memset(&inAddr,0,sizeof(inAddr));

   char buf[NE_MSG_MAX_DATA_LEN];
   memset(buf,0,NE_MSG_MAX_DATA_LEN);
   
   /* declare some message pointers that may be needed later */
   nemsgError *errorMsg = (nemsgError *)0;
   nemsgDisconnect *disconnectMsg = (nemsgDisconnect *)0;
   nemsgForcedDisconnect *forcedDisconnectMsg = (nemsgForcedDisconnect *)0;
   nemsgStatus *statusMsg = (nemsgStatus *)0;
   nemsgSearchResults *searchResultsMsg = (nemsgSearchResults *)0;
   nemsgQueryResult *queryResultMsg = (nemsgQueryResult *)0;

   ncSocket *sock = (ncSocket *)socket;
   if (socket)
   {
      if (connected && (msgType = neUtils::peekMessage(sock)))
      {
         memset(buf,0,NE_MSG_MAX_DATA_LEN*sizeof(char));

         printf("Client message handler called and got msgType %lu\n",msgType);
         switch(msgType)
         {
         case NE_MSG_ERROR:
            errorMsg = new nemsgError(sock);
            if (errorMsg->recv(buf,NE_MSG_MAX_DATA_LEN) == NC_OK)
            {
                printf("Error retrieved from server: %s\n",buf);
            }
            else
            {
                goto recv_failed;
            }
            delete errorMsg;
         case NE_MSG_STATUS:
            statusMsg = new nemsgStatus(sock);
            if (statusMsg->recv(&numUsers,&numFiles,&numMB) == NC_OK)
            {
                printf("Status Message Received: %lu connected user(s) | "
                       "%lu shared file(s) | %luMB\n",numUsers,numFiles,
                       numMB);
            }
            else
            {
                goto recv_failed;
            }
            delete statusMsg;
            break;
         case NE_MSG_FORCED_DISCONNECT:
            printf("Server forcefully disconnected us for the following reason:\n");
            forcedDisconnectMsg = new nemsgForcedDisconnect(sock);
            if (forcedDisconnectMsg->recv(buf,NE_MSG_MAX_DATA_LEN) == NC_OK)
            {
                connected = 0;
                printf("%s\n",buf);
                printf("Client terminating.\n");
            }
            else
            {
                goto recv_failed;
            }
            delete forcedDisconnectMsg;
            exit(1);
            break;
         case NE_MSG_SEARCH_RESULTS:
            printf("Received NE_MSG_SEARCH_RESULTS ... ");
            searchResultsMsg = new nemsgSearchResults(sock);
            if (searchResultsMsg->recv(&numResults) == NC_OK)
            {
                printf("%lu results reported.\n",numResults);
            }
            else
            {
                goto recv_failed;
            }
            delete searchResultsMsg;
            break;
         case NE_MSG_QUERY_RESULT:
            printf("Received NE_MSG_QUERY_RESULT:\n");
            queryResultMsg = new nemsgQueryResult(sock);
            if (queryResultMsg->recv(&ipAddr,&connection,&firewallStatus,
                                     &filesize,&controlPort,&id,buf,
                                     NE_MSG_MAX_DATA_LEN) == NC_OK)
            {
                inAddr.s_addr = ipAddr;
                printf("IP: %s | Connection: %lu | File: %s | Firewall: "
                       "%lu | Filesize: %lu | CPort: %lu | ID = %lu\n",
                       inet_ntoa(inAddr),connection,buf,firewallStatus,
                       filesize,controlPort,id);
            }
            else
            {
                goto recv_failed;
            }
            delete queryResultMsg;
            break;
           recv_failed:
            printf("Failed to retrieve message type %d\n",msgType);
            goto disconnect_user;
         default:
            printf("Client received an unknown message of type %lu\n",msgType);
           disconnect_user:
            printf("Disconnecting from server...");
            disconnectMsg = new nemsgDisconnect(sock);
            disconnectMsg->send();
            sock->flush();
            delete disconnectMsg;
            printf("done.\n");
            connected = 0;
            printf("Application terminating.\n");
            exit(1);
         }
      }
   }
   else
   {
      char disconnectMsg[MAX_DISCONNECT_MSG_LEN];
      memset(disconnectMsg,0,MAX_DISCONNECT_MSG_LEN);
      if (g_clientConnection->disconnect(disconnectMsg,
                                         MAX_DISCONNECT_MSG_LEN))
      {
          printf("Error: Disconnect ack was not received. "
                 "Client Terminating.\n");
          g_clientConnection->stopListenerThreads();
          exit(1);
      }
      else
      {
          connected = 0;
          disconnectMsg[MAX_DISCONNECT_MSG_LEN-1] = '\0';
          printf("Communication with NEshare Server has ended "
                 "gracefully.\n");
      }
   }
   return (void *)0;
}


int main(int argc, char **argv)
{
   /* parse command line options */
   if (argc < 2)
   {
      printf("Usage: neshare_client <query1> <query2> ...\n");
      exit(1);
   }

   /*
     create a client connection object for
     communicating with an NEshare Server
   */
   neClientConnection clientConnection(30000);
   clientConnection.initialize(handleIncomingMessage);

   /* set the global pointer to point to this clientConnection object */
   g_clientConnection = &clientConnection;

   printf("NEshare Client v%s has started (written by "
          "Neill Miller).\n",NE_VERSION);

   /* build up client file list */
   clientConnection.buildFileList();

   /*
     start the listener thread for receiving messages from both the
     server and any clients that wish to communicate with us
   */
   if (clientConnection.startListenerThreads())
   {
      printf("Failed to start listener threads. Program terminating.\n");
      exit(1);
   }

   /* attempt to login to the NEshare server */
   char loginMessage[MAX_LOGIN_MSG_LEN];
   memset(loginMessage,0,MAX_LOGIN_MSG_LEN);
   if (clientConnection.login(loginMessage,MAX_LOGIN_MSG_LEN))
   {
      printf("Connection to NEshare Server failed. Client terminating.\n");
      clientConnection.stopListenerThreads();
      exit(1);
   }
   loginMessage[MAX_LOGIN_MSG_LEN-1] = '\0';
   printf("Login successful.\nServer says: %s\n\n",loginMessage);

   /* submit the file list we've built up */
   if (clientConnection.submitFileList())
   {
      printf("Failed to submit file list to NEshare Server. "
             "Client terminating.\n");
      clientConnection.stopListenerThreads();
      exit(1);
   }

   /* delay for a bit */
   sleep(2);

    if (clientConnection.sendSearchKeywords(argv+1,argc-1))
    {
       printf("Error: sendSearchQuery failed.  Program terminating.\n");
       clientConnection.stopListenerThreads();
       exit(1);
    }

   /* delay for a bit */
   sleep(10);
   
   /* and now disconnect */
   char disconnectMsg[256];
   memset(disconnectMsg,0,256);
   if (clientConnection.disconnect(disconnectMsg,255))
   {
      printf("Error: Disconnect ack was not received. Client Terminating.\n");
   }
   else
   {
      printf("Communication with NEshare Server has ended gracefully.\n");
   }

   /* delay for a bit to receive disconnect ack */
   sleep(5);

   /* finally, stop the listening threads */
   clientConnection.stopListenerThreads();

   return 0;
}
