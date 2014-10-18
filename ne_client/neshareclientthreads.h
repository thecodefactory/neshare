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

#ifndef __NESHARECLIENTTHREADS_H
#define __NESHARECLIENTTHREADS_H

#include "neclientheaders.h"

/* 
   a namespace which stores all of the thread functions for the NEshare client.
   These functions are started on client initialization and unless noted are
   persistent throughout the life of the running client. 
   The utility namespace is just for grouping misc. client utilities.
*/
namespace neShareClientThreads
{
   namespace neClientUtils
   {
      nePeer *getNewConnectedPeer(unsigned long peerIpAddr, 
                                  unsigned long peerCtrlPort,
                                  unsigned long timeout);

      int rejectNewServentPeer(nePeer *peer);
      int handlePeerDisconnect(nePeer *peer,
                               nePeerManager *peerManager,
                               nePeerUploadManager *peerUploadManager,
                               nePeerDownloadManager *peerDownloadManager);
      int handleFileRequestAck(nePeer *peer, 
                               nePeerDownloadManager *peerDownloadManager);
      int handleFileResumeAck(nePeer *peer, 
                              nePeerDownloadManager *peerDownloadManager);
      int handleFileDataSend(nePeer *peer,
                             nePeerDownloadManager *peerDownloadManager);
      int handleFileDataSendEnd(nePeer *peer,
                                nePeerDownloadManager *peerDownloadManager);
      int handleFileDataCancel(nePeer *peer,
                               nePeerUploadManager *peerUploadManager);
      int handleFileRequest(nePeer *peer,
                            nePeerUploadManager *peerUploadManager);
      int handleFileResume(nePeer *peer,
                           nePeerUploadManager *peerUploadManager);
      int handlePushRequestAck(nePeer *peer,
                               nePeerDownloadManager *peerDownloadManager);
      int handleReadyPeers(nePeerManager *peerManager,
                           nePeerDownloadManager *peerDownloadManager,
                           nePeerUploadManager *peerUploadManager);

      int handleConnectedPeerLogin(nePeer *peer, unsigned long timeout);
      int sendFileRequest(nePeer *peer,
                          nePeerDownloadManager *peerDownloadManager,
                          char *filename);
  }

/* a function which is called each time a new peer client tries to connect to
   this client.  The incoming parameter is an ncSocket pointer.
   this function either discards the client if an error occurs, or adds it
   to the active client list if all is ok. */
   void *processLoginMessage(void *ptr);

/* thread function which binds to a port and listens for incoming connections
   from peers who wish to communicate with this client directly */
   void *listenForClients(void *ptr);

/* thread functions which listen to all peer sockets and dispatches
   based on what activities are ready.  start these threads before 
   listening for connections */
   void *processClientPeers(void *ptr);
   void *processServentPeers(void *ptr);

/* thread function which handles all file reads and network sends for
   any uploaded file from this peer to another.  this thread must be
   started before any data can be sent, but is not critical for
   normal client operation */
   void *processUploads(void *ptr);

/* this is the function which starts all of the threads, calling the
   above thread functions initially, and initializes necessary objects.
   All arguments should be passed in from the neClientConnection object */
   void startThreads(neConfig *config, 
                     nePeerManager *peerClientManager,
                     nePeerManager *peerServentManager,
                     nePeerDownloadManager *peerDownloadManager,
                     nePeerUploadManager *peerUploadManager);
                    
/* this stops the neShare server threads and does clean up of objects */
   void stopThreads();
}

#endif /* __NESHARECLIENTTHREADS_H */
