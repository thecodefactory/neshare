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

namespace neShareClientThreads
{
    static ncThread s_processClientPeersThread;
    static ncThread s_processServentPeersThread;
    static ncThread s_clientListenerThread;
    static ncThread s_processUploadsThread;

    static int s_processClientPeersThreadStop = 0;
    static int s_processServentPeersThreadStop = 0;
    static int s_processUploadsThreadStop = 0;

    static ncSocketListener *g_nsl = (ncSocketListener *)0;

    /*
      the following are defined only to store pointers to the 
      internal objects stored in the neClientConnection object 
      to use internally in this namespace
    */
    static neConfig *g_config = (neConfig *)0;
    static nePeerManager *g_peerClientManager = (nePeerManager *)0;
    static nePeerManager *g_peerServentManager = (nePeerManager *)0;
    static nePeerDownloadManager *g_peerDownloadManager =
    (nePeerDownloadManager *)0;
    static nePeerUploadManager *g_peerUploadManager =
    (nePeerUploadManager *)0;

    void *processLoginMessage(void *ptr)
    {
        ncSocket *newSock = (ncSocket *)ptr;
        if (newSock)
        {
            /*
              read the login message and return
              an appropriate response
            */
            nemsgPeerLogin peerLoginMsg(newSock);
            if (peerLoginMsg.recv() == 0)
            {
                iprintf("neShareClientThreads::processLoginMessage | "
                        "Login Message Received.\n");
                /*
                  FIXME: default TTL of connected peer is 300 seconds...
                  this should be configurable
                */
                nePeer *newPeer = new nePeer(newSock,300);
                if (newPeer)
                {
                    if (g_peerServentManager->addPeer(newPeer))
                    {
                        eprintf("neShareClientThreads::processLoginMessa"
                                "ge | addPeer failed.\n");
                        neClientUtils::rejectNewServentPeer(newPeer);
                        newSock->flush();
                        delete newSock;
                        return (void *)0;
                    }
                    /* send a peer login ack */
                    nemsgPeerLoginAck peerLoginAckMsg(newSock);
                    if (peerLoginAckMsg.send() == 0)
                    {
                        iprintf("New Peer Added (addr = %x) - Total Count"
                                " is %d.\n",newPeer,
                                g_peerServentManager->getNumPeers());
                        newSock->flush();
                    }
                }
                else
                {
                    eprintf("neShareClientThreads::processLoginMessage | "
                            "Cannot allocate new peer.\n");
                }
            }
            else
            {
                /* drop the connection */
                eprintf("neShareClientThreads::processLoginMessage | "
                        "Login Message not received.\n");
                delete newSock;
            }
        }
        return (void *)0;
    }

    void *listenForClients(void *ptr)
    {
        unsigned long clientControlPort = 0;

        assert(g_config);
        clientControlPort = g_config->getClientControlPort();

        /* 
           create a ncSocketListener and register a callback that will
           add a new user to the client peerManager (similar to the
           userManager in the server).
        */
        if (g_nsl)
        {
            eprintf("FIXME: neShareClientThreads::listenForClients "
                    "called with an already initialized socket "
                    "listener object -- terminating\n");
            assert(0);
        }
        g_nsl = new ncSocketListener(clientControlPort,SOCKTYPE_TCPIP);
        if (g_nsl &&
            g_nsl->startListening(processLoginMessage, NC_NONTHREADED,
                                  NC_REUSEADDR) != NC_OK)
        {
            eprintf("ERROR!!! NEshare client listener has mysteriously "
                    "stopped running.\nNo more incoming client "
                    "connections are allowed.\nClient listener "
                    "terminating.\n");
        }
        return (void *)0;
    }


    void *processClientPeers(void *ptr)
    {
        int numReady = 0;
        std::vector<nePeer *> markedPeers;
        std::vector<nePeer *>::iterator iter;

        s_processClientPeersThreadStop = 1;
        while(s_processClientPeersThreadStop)
        {
            assert(markedPeers.empty());

            numReady = g_peerClientManager->pollPeerSockets(&markedPeers);

            /* remove marked peers if any */
            for(iter = markedPeers.begin(); iter != markedPeers.end();
                iter++)
            {
                g_peerClientManager->removePeer((*iter),
                                                g_peerUploadManager,
                                                g_peerDownloadManager);
            }
            markedPeers.clear();

            if (numReady == 0)
            {
                /*
                  if there are no peer sockets ready,
                  sleep and then try again.
                */
                ncSleep(250);
                continue;
            }
            else if (numReady == -1)
            {
                /* if an error occurred, report the error and continue */
                eprintf("neShareClientThreads::processClientPeers | "
                        "peerManager::pollPeerSockets failed.\n");
                continue;
            }

            /* handle ready peers, if any */
            if (neClientUtils::handleReadyPeers(g_peerClientManager,
                                                g_peerDownloadManager,
                                                g_peerUploadManager))
            {
                eprintf("neShareClientThreads::processClientPeers | a"
                        " non-fatal peer error occured.\n");
            }

            /* check if a cancel request was issued */
            ncThread::testCancel();
        }
        s_processClientPeersThreadStop = 1;
        return (void *)0;
    }

    void *processServentPeers(void *ptr)
    {
        int numReady = 0;
        std::vector<nePeer *> markedPeers;
        std::vector<nePeer *>::iterator iter;

        s_processServentPeersThreadStop = 1;
        while(s_processServentPeersThreadStop)
        {
            assert(markedPeers.empty());

            numReady = g_peerServentManager->pollPeerSockets(
                &markedPeers);

            /* remove marked peers if any */
            for(iter = markedPeers.begin(); iter != markedPeers.end();
                iter++)
            {
                g_peerServentManager->removePeer((*iter),
                                                 g_peerUploadManager,
                                                 g_peerDownloadManager);
            }
            markedPeers.clear();

            if (numReady == 0)
            {
                /*
                  if there are no peer sockets ready,
                  sleep and then try again.
                */
                ncSleep(250);
                continue;
            }
            else if (numReady == -1)
            {
                /* if an error occurred, report the error and continue */
                eprintf("neShareClientThreads::processServentPeers | "
                        "peerManager::pollPeerSockets failed.\n");
                continue;
            }

            /* handle ready peers, if any */
            if (neClientUtils::handleReadyPeers(g_peerServentManager,
                                                g_peerDownloadManager,
                                                g_peerUploadManager))
            {
                eprintf("neShareClientThreads::processServentPeers "
                        "| a non-fatal peer error occured.\n");
            }

            /* check if a cancel request was issued */
            ncThread::testCancel();
        }
        s_processServentPeersThreadStop = 1;
        return (void *)0;
    }

    void *processUploads(void *ptr)
    {
        s_processUploadsThreadStop = 1;
        while(s_processUploadsThreadStop)
        {
            /* check if there are any current uploads */
            if (g_peerUploadManager->getNumUploads() == 0)
            {
                /* if not, sleep for a while */
                ncSleep(500);
            }
            else
            {
                /*
                  send another chunk to each peer
                  with an active download
                */
                g_peerUploadManager->sendPeerData();
            }
            /* check if a cancel request was issued */
            ncThread::testCancel();
        }
        s_processUploadsThreadStop = 1;
        return (void *)0;
    }

    void startThreads(neConfig *config, 
                      nePeerManager *peerClientManager,
                      nePeerManager *peerServentManager,
                      nePeerDownloadManager *peerDownloadManager,
                      nePeerUploadManager *peerUploadManager)
    {
        /* stash all incoming arguments for later use */
        g_config = config;
        g_peerClientManager = peerClientManager;
        g_peerServentManager = peerServentManager;
        g_peerDownloadManager = peerDownloadManager;
        g_peerUploadManager = peerUploadManager;

        /* set the config object on the download manager */
        g_peerDownloadManager->setConfig(g_config);

        /* start up client-to-client related threads */
        if (s_processClientPeersThread.start(
                processClientPeers,(void *)0) == NC_FAILED)
        {
            eprintf("Fatal error: Cannot start "
                    "processClientPeersThread.\n");
            exit(1);
        }
        if (s_processServentPeersThread.start(
                processServentPeers,(void *)0) == NC_FAILED)
        {
            eprintf("Fatal error: Cannot start "
                    "processServentPeersThread.\n");
            exit(1);
        }
        if (s_clientListenerThread.start(
                listenForClients,(void *)0) == NC_FAILED)
        {
            eprintf("Fatal error: Cannot start "
                    "clientListenerThread.\n");
            exit(1);
        }
        if (s_processUploadsThread.start(
                processUploads,(void *)0) == NC_FAILED)
        {
            eprintf("Error: Cannot start upload processing thread.  "
                    "Skipping.\n");
        }
        /* detach threads (to spin off in background) */
        if (s_processClientPeersThread.detach() == NC_FAILED)
        {
            eprintf("Fatal error: Cannot detach "
                    "processClientPeersThread.\n");
            exit(1);
        }
        if (s_processServentPeersThread.detach() == NC_FAILED)
        {
            eprintf("Fatal error: Cannot detach "
                    "processServentPeersThread.\n");
            exit(1);
        }
        if (s_clientListenerThread.detach() == NC_FAILED)
        {
            eprintf("Fatal error: Cannot detach clientListenerThread.\n");
            stopThreads();
            exit(1);
        }
        if (s_processUploadsThread.detach() == NC_FAILED)
        {
            eprintf("Error: Cannot detach processUploadsThread.  "
                    "Skipping.\n");
        }
    }

    void stopThreads()
    {
        /* stop all running client threads */
        if (g_nsl)
        {
            g_nsl->stopListening();
        }
        s_processClientPeersThreadStop = 0;
        s_processServentPeersThreadStop = 0;
        s_processUploadsThreadStop = 0;

        /*
          sleep for half a second to allow
          for proper thread cancellation
        */
        ncSleep(500);

        /* now cancel the threads, if they haven't stopped already */
        if (!s_processClientPeersThreadStop)
        {
            s_processClientPeersThread.stop(0);
        }
        if (!s_processServentPeersThreadStop)
        {
            s_processServentPeersThread.stop(0);
        }
        if (!s_processUploadsThreadStop)
        {
            s_processUploadsThread.stop(0);
        }
        s_clientListenerThread.stop(0);

        /* uninitialize our pointers to the objects we know about */
        g_config = (neConfig *)0;
        g_peerClientManager = (nePeerManager *)0;
        g_peerServentManager = (nePeerManager *)0;
        g_peerDownloadManager = (nePeerDownloadManager *)0;
        g_peerUploadManager = (nePeerUploadManager *)0;

        delete g_nsl;
        g_nsl = (ncSocketListener *)0;
    }
}
