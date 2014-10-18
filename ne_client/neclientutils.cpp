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
    namespace neClientUtils
    {
        nePeer *getNewConnectedPeer(unsigned long peerIpAddr, 
                                    unsigned long peerCtrlPort,
                                    unsigned long timeout)
        {
            nePeer *ret = (nePeer *)0;
            iprintf("neClientUtils::getNewConnectedPeer | Trying "
                    "peer %s (%lu)\n",
                    neUtils::ipToStr(peerIpAddr),peerIpAddr);
            ncSocket *sock = new ncSocket(neUtils::ipToStr(peerIpAddr),
                                          peerCtrlPort, SOCKTYPE_TCPIP);
            if (sock && (sock->connect() == NC_OK))
            {
                sock->setTimeout(timeout);
            }

            /* FIXME: default ttl is 300 seconds; should be configurable */
            ret = new nePeer(sock,300);
            if (!ret)
            {
                delete sock;
            }
            return ret;
        }

        int rejectNewServentPeer(nePeer *peer)
        {
            int ret = 1;

            /* send the peer login failed message and delete the peer */
            nemsgPeerLoginFailed peerLoginFailedMsg(peer->getSocket());
            ret = peerLoginFailedMsg.send(/*FIXME: config.*/
                "Connection denied by remote Servent.");
            peer->getSocket()->flush();
            delete peer;
            return ret;
        }

        int handlePeerDisconnect(nePeer *peer,
                                 nePeerManager *peerManager,
                                 nePeerUploadManager *peerUploadManager,
                                 nePeerDownloadManager *peerDownloadManager)
        {
            int ret = 0;

            /* read disconnect msg from peer */
            nemsgPeerDisconnect disconnectMsg(peer->getSocket());
            ret = disconnectMsg.recv();
            ret += peerManager->removePeer(
                peer,peerUploadManager,peerDownloadManager);
            iprintf("neClientUtils::handlePeerDisconnect | %d peers "
                    "still connected.\n",peerManager->getNumPeers());
            return ret;
        }

        int handleFileRequest(nePeer *peer, 
                              nePeerUploadManager *peerUploadManager)
        {
            int ret = 1;
            char filename[NE_MSG_MAX_DATA_LEN];
            memset(filename,0,NE_MSG_MAX_DATA_LEN*sizeof(char));

            /* read in the filename, etc. requested */
            nemsgFileRequest fileRequestMsg(peer->getSocket());
            ret = fileRequestMsg.recv(filename,NE_MSG_MAX_DATA_LEN);

            if (ret == 0)
            {
                iprintf("neClientUtils::handleFileRequest | got a "
                        "request for %s\n",filename);

                /*
                  if the addition is OK, a proper
                  ack will be sent here
                */
                ret = peerUploadManager->addPeerUpload(peer,filename);
                peer->getSocket()->flush();
            }
            return ret;
        }

        int handleFileResume(nePeer *peer, 
                             nePeerUploadManager *peerUploadManager)
        {
            int ret = 1;
            char filename[NE_MSG_MAX_DATA_LEN];
            unsigned long position = 0;
            memset(filename,0,NE_MSG_MAX_DATA_LEN*sizeof(char));

            /* read in the filename, etc. to be resumed */
            nemsgFileResume fileResumeMsg(peer->getSocket());
            ret = fileResumeMsg.recv(&position,filename,
                                     NE_MSG_MAX_DATA_LEN);
            if (ret == 0)
            {
                iprintf("neClientUtils::handleFileResume | got a resume "
                        "for %s\n",filename);

                /*
                  if the addition is OK, a proper
                  ack will be sent here
                */
                ret = peerUploadManager->addPeerUpload(peer,filename,
                                                       position);
                peer->getSocket()->flush();
            }
            return ret;
        }

        int handleFileRequestAck(nePeer *peer,
                                 nePeerDownloadManager *peerDownloadManager)
        {
            int ret = 1;
            unsigned long fileSendID = 0;
            unsigned long maxBlockSize = 0;
            unsigned long filesize = 0;
            unsigned char md5checksum[16];
            char filename[NE_MSG_MAX_DATA_LEN];
            memset(filename,0,NE_MSG_MAX_DATA_LEN*sizeof(char));

            nemsgFileRequestAck fileRequestAckMsg(peer->getSocket());
            if (fileRequestAckMsg.recv(&fileSendID,
                                       &maxBlockSize,
                                       &filesize,
                                       md5checksum,
                                       filename,NE_MSG_MAX_DATA_LEN))
            {
                eprintf("neClientUtils::handleFileRequestAck | "
                        "File Request Ack recv error. Skipping.\n");
            }
            else if (fileSendID == INVALID_FILE_REQUEST)
            {
                /* handle the error case of an invalid request */
                eprintf("neClientUtils::handleFileRequestAck | "
                        "File Request denied by peer. Skipping.\n");
            }
            else
            {
                ret = peerDownloadManager->addPeerDownload(
                    fileSendID,maxBlockSize,filesize,
                    md5checksum,peer,filename);
                if (ret)
                {
                    eprintf("neClientUtils::handleFileRequestAck | "
                            "Cancelling peer download "
                            "(FILE ID = %lu | ret = %d)\n",
                            fileSendID,ret);
                }
                else
                {
                    iprintf("neClientUtils::handleFileRequestAck | "
                            "Added peer download "
                            "(FILE ID = %lu | ret = %d)\n",
                            fileSendID,ret);
                }
            }
            return ret;
        }

        int handleFileResumeAck(nePeer *peer,
                                nePeerDownloadManager *peerDownloadManager)
        {
            int ret = 1;
            unsigned long fileSendID = 0;
            unsigned long maxBlockSize = 0;
            unsigned long filesize = 0;
            unsigned char md5checksum[16];
            char filename[NE_MSG_MAX_DATA_LEN];
            memset(filename,0,NE_MSG_MAX_DATA_LEN*sizeof(char));

            nemsgFileResumeAck fileResumeAckMsg(peer->getSocket());
            if (fileResumeAckMsg.recv(&fileSendID,
                                      &maxBlockSize,
                                      &filesize,
                                      md5checksum,
                                      filename,NE_MSG_MAX_DATA_LEN))
            {
                eprintf("neClientUtils::handleFileResumeAck | File "
                        "Resume Ack recv error. Skipping.\n");
            }
            else if (fileSendID == INVALID_FILE_REQUEST)
            {
                /* handle the error case of an invalid resume */
                eprintf("neClientUtils::handleFileResumeAck | File "
                        "Resume denied by peer. Skipping.\n");
            }
            else
            {
                ret = peerDownloadManager->addPeerDownload(
                    fileSendID,maxBlockSize,filesize,
                    md5checksum,peer,filename);
                if (ret)
                {
                    eprintf("neClientUtils::handleFileResumeAck | "
                            "Cancelling peer resume "
                            "(FILE ID = %lu | ret = %d)\n",
                            fileSendID,ret);
                    nemsgFileDataCancel fileDataCancelMsg(
                        peer->getSocket());
                    ret = fileDataCancelMsg.send(fileSendID);
                }
                else
                {
                    iprintf("neClientUtils::handleFileResumeAck | "
                            "Added peer resume "
                            "(FILE ID = %lu | ret = %d)\n",
                            fileSendID,ret);
                }
            }
            return ret;
        }

        int handleFileDataSend(nePeer *peer,
                               nePeerDownloadManager *peerDownloadManager)
        {
            /* just pass this off to the peer download manager */
            return (peerDownloadManager->getFileDataSend(peer));
        }

        int handleFileDataSendEnd(nePeer *peer,
                                  nePeerDownloadManager *peerDownloadManager)
        {
            /* just pass this off to the peer download manager */
            return (peerDownloadManager->endFileDataSend(peer));
        }

        int handleFileDataCancel(nePeer *peer,
                                 nePeerUploadManager *peerUploadManager,
                                 nePeerDownloadManager *peerDownloadManager)
        {
            int ret = 0;
            unsigned long fileID = 0;
            nemsgFileDataCancel fileDataCancelMsg(peer->getSocket());
            if ((ret = fileDataCancelMsg.recv(&fileID)) == 0)
            {
                iprintf("neClientUtils::handleFileDataCancel | "
                        "Cancelling transfer ID %lu\n",fileID);
                /*
                  first check if the file is being uploaded,
                  and if so cancel it
                */
                if (peerUploadManager->cancelPeerUpload(peer,fileID,0))
                {
                    /* if not, it must be a download, so cancel it */
                    if (peerDownloadManager->cancelPeerDownload(
                            peer->getSocket()->getIpAddr(),fileID,0))
                    {
                        eprintf("neClientUtils::handleFileDataCancel | "
                                "FIXME: File Cancel failed.\n");
                        ret = 1;
                    }
                }
            }
            return ret;
        }

        int handlePushRequestAck(nePeer *peer,
                                 nePeerDownloadManager *peerDownloadManager)
        {
            int ret = 0;
            char buf[NE_MSG_MAX_DATA_LEN];

            nemsgPushRequestAck pushRequestAckMsg(peer->getSocket());
            if (pushRequestAckMsg.recv(buf,NE_MSG_MAX_DATA_LEN) ||
                sendFileRequest(peer,peerDownloadManager,buf))
            {
                eprintf("neClientConnection::handleIncomingMessages | "
                        "Failed to handle push request ack message.\n");
                ret = 1;
            }
            return ret;
        }

        int handleReadyPeers(nePeerManager *peerManager,
                             nePeerDownloadManager *peerDownloadManager,
                             nePeerUploadManager *peerUploadManager)
        {
            int ret = 0;
            nePeer *peer = (nePeer *)0;
            unsigned long msgType = 0;

            assert(peerManager);

            /*
              iterate over each ready socket and do a peek read.  
              Based on the message type, handle it appropriately.
            */
            peerManager->resetPeerSocketPosition();
            while(peerManager &&
                  (peer = peerManager->getNextPeerSocketReady()))
            {
                /* reset TTL since we have activity */
                peer->resetTTL();

                /* retrieve (by peeking at) the message type */
                msgType = neUtils::peekMessage(peer->getSocket());

                /* handle messages based on type */
                switch(msgType)
                {
                    case NE_MSG_PEER_LOGIN:
                        /* 
                           this should never happen.  if it does, the
                           peer should be disconnected for a protocol
                           violation since the login sequence should
                           be over at this point (the login msg is
                           expected only in the 
                           neShareClientThreads::processLoginMessage
                           method (which is a callback registered
                           with netclass))
                        */
                        eprintf("neClientUtils::handleReadyPeers | Got "
                                "a peer login ack.\nProtocol violation: "
                                "Sending a peer disconnect.\n");
                        ret = handlePeerDisconnect(
                            peer,peerManager,peerUploadManager,
                            peerDownloadManager);
                        break;
                    case NE_MSG_PEER_LOGIN_ACK:
                        /* 
                           this should never happen.  if it does, the
                           peer should be disconnected for a protocol
                           violation since the login sequence should
                           be over at this point (the login ack is
                           expected only in the
                           neClientConnection::login method)
                        */
                        eprintf("neClientUtils::handleReadyPeers | Got "
                                "a peer login ack.\nProtocol violation: "
                                "Sending a peer disconnect.\n");
                        ret = handlePeerDisconnect(
                            peer,peerManager,peerUploadManager,
                            peerDownloadManager);
                        break;
                    case NE_MSG_PEER_LOGIN_FAILED:
                        /*
                          this should never happen.  if it does, the
                          peer should be disconnected for a protocol
                          violation since the login sequence should
                          be over at this point 
                        */
                        eprintf("neClientUtils::handleReadyPeers | Got "
                                "a peer login failed.\nProtocol "
                                "violation:Sending a peer disconnect.\n");
                        ret = handlePeerDisconnect(
                            peer,peerManager,peerUploadManager,
                            peerDownloadManager);
                        break;
                    case NE_MSG_PEER_DISCONNECT:
                        dprintf("neClientUtils::handleReadyPeers | Got "
                                "a peer disconnect\n");
                        ret = handlePeerDisconnect(
                            peer,peerManager,peerUploadManager,
                            peerDownloadManager);
                        break;
                    case NE_MSG_FILE_REQUEST:
                        dprintf("neClientUtils::handleReadyPeers | Got "
                                "a file request\n");
                        ret = handleFileRequest(peer,peerUploadManager);
                        break;
                    case NE_MSG_FILE_REQUEST_ACK:
                        dprintf("neClientUtils::handleReadyPeers | Got "
                                "a file request ack\n");
                        ret = handleFileRequestAck(peer,peerDownloadManager);
                        break;
                    case NE_MSG_FILE_RESUME:
                        dprintf("neClientUtils::handleReadyPeers | Got "
                                "a file resume\n");
                        ret = handleFileResume(peer,peerUploadManager);
                        break;
                    case NE_MSG_FILE_RESUME_ACK:
                        dprintf("neClientUtils::handleReadyPeers | Got "
                                "a file resume ack\n");
                        ret = handleFileResumeAck(peer,peerDownloadManager);
                        break;
                    case NE_MSG_FILE_DATA_SEND:
                        ret = handleFileDataSend(peer,peerDownloadManager);
                        break;
                    case NE_MSG_FILE_DATA_SEND_END:
                        dprintf("neClientUtils::handleReadyPeers | Got "
                                "a file data send end!\n");
                        ret = handleFileDataSendEnd(peer,peerDownloadManager);
                        break;
                    case NE_MSG_FILE_DATA_CANCEL:
                        dprintf("neClientUtils::handleReadyPeers | Got "
                                "a file data cancel!\n");
                        ret = handleFileDataCancel(
                            peer,peerUploadManager,peerDownloadManager);
                        break;
                    case NE_MSG_PUSH_REQUEST_ACK:
                        dprintf("neClientUtils::handleReadyPeers | Got "
                                "a push request ack!\n");
                        ret = handlePushRequestAck(peer,peerDownloadManager);
                        break;
                    default:
                        /* 
                           if we get an invalid message, disconnect user
                           forcefully - otherwise we'll spin forever
                           peeking at a message that may never be handled.
                        */
                        eprintf("neClientUtils::handleReadyPeers | "
                                "Message Type is INVALID (%x).\n"
                                "Protocol violation: Disconnecting "
                                "Peer.\n",(unsigned int)msgType);

                        /*
                          just return if we've already removed this
                          peer, yet it's still in book keeping
                          temporarily.
                        */
                        if ((msgType == 0) && peer->wasRemoved())
                        {
                            return ret;
                        }
                        ret = peerManager->removePeer(
                            peer,peerUploadManager,peerDownloadManager);

                        /* see FIXME and NOTE below */
                        peer->setRemoved();

                        iprintf("neClientUtils::handleReadyPeers | %d "
                                "peers still connected.\n",
                                peerManager->getNumPeers());
                        peer = (nePeer *)0;
                        break;
/*
  FIXME:
  Here we can suffer from a degenerate case where the current peer is
  properly disconnected, however the next peer returned from
  peerManager->getNextPeerSocketReady() on the next iteration is
  the peer that we just disconnected.  We could stash recently
  removed peers and continue processing while ignoring those in
  particular, or we could take the easy way out and just return
  -- which is what we're doing here for now ;-)

  NOTE: this is now solved by the use of the hackish m_removed
  flag in the nePeer object.  it's set after peerManager->removePeer
  is called so that we can return before trying to remove the
  peer again if we see invalid messages while the user is in the
  process of leaving book keeping.
*/
                }
                /* check if a cancel request was issued */
                ncThread::testCancel();
            }

/*
  FIXME: Here we need to check if a single peer repeatedly fails
  for a configurable number of times.  If so, remove the peer
  forcefully because it's likely we're either getting flooded or
  something is very broken in our communication with them.  This
  would best be done with an error count field in the nePeer
  class similar to how it used in the neUser class in the server code.

  04/12/2003
  Sort of fixed now; needs tweaking.  Limit should be configurable too.
  -N.M.
*/
            if (ret && peer)
            {
                peer->incrementErrorCount();
                if (peer->getErrorCount() > CONFIG_MAX_PEER_ERROR_COUNT)
                {
                    eprintf("neClientUtils::handleReadyPeers | Too many "
                            "peer errors.  Forcing disconnect.\n");
                    ret = peerManager->removePeer(
                        peer,peerUploadManager,peerDownloadManager);
                    iprintf("neClientUtils::handleReadyPeers | %d "
                            "peers still connected.\n",
                            peerManager->getNumPeers());
                }
            }
            return ret;
        }

        int handleConnectedPeerLogin(nePeer *peer, unsigned long timeout)
        {
            int ret = 1;
            char buf[NE_MSG_MAX_DATA_LEN];
            if (peer)
            {
                memset(buf,0,NE_MSG_MAX_DATA_LEN);
                nemsgPeerLogin peerLoginMsg(peer->getSocket());
                nemsgPeerLoginAck peerLoginAckMsg(peer->getSocket());
                nemsgPeerLoginFailed peerLoginFailedMsg(peer->getSocket());
                if (peerLoginMsg.send())
                {
                    eprintf("neClientUtils::handleConnectedPeerLogin | "
                            "Peer Login msg send failed.\n");
                }
                else
                {
                    /* wait for the next incoming messages */
                    while(neUtils::peekMessage(peer->getSocket()) == 0)
                    {
                        ncSleep(10);
                    }
                    if (neUtils::peekMessage(peer->getSocket()) ==
                        NE_MSG_PEER_LOGIN_FAILED)
                    {
                        if (peerLoginFailedMsg.recv(
                                buf,NE_MSG_MAX_DATA_LEN))
                        {
                            eprintf("neClientUtils::handleConnectedPeerLogin "
                                    "| peer login failed msg recv error.\n");
                        }
                    }
                    else if (peerLoginAckMsg.recv() == 0)
                    {
                        iprintf("neClientUtils::handleConnectedPeerLogin"
                                " | recv'd login ack message.\n");
                        ret = 0;
                    }
                    else
                    {
                        eprintf("neClientUtils::handleConnectedPeerLogin"
                                " | peer login error or timeout.\n"
                                "Unexpected message received = %x.\n",
                                (unsigned int)
                                neUtils::peekMessage(peer->getSocket()));
                    }
                }
            }
            return ret;
        }

        int sendFileRequest(nePeer *peer,
                            nePeerDownloadManager *peerDownloadManager,
                            char *filename)
        {
            int ret = 1;
         
            assert(peer);
            assert(peerDownloadManager);
            assert(filename);
         
            unsigned long existingFileSize = 
                peerDownloadManager->fileExistsLocally(filename);
            if (existingFileSize)
            {
                nemsgFileResume fileResumeMsg(peer->getSocket());
                if (fileResumeMsg.send(filename,existingFileSize))
                {
                    eprintf("neClientConnection::requestPeerFile | "
                            "file resume message send failed.\n");
                }
                else
                {
                    ret = 0;
                }
            }
            else
            {
                /*
                  if we now have a properly managed peer object,
                  send a file request
                */
                nemsgFileRequest fileRequestMsg(peer->getSocket());
                if (fileRequestMsg.send(filename))
                {
                    eprintf("neClientConnection::requestPeerFile | "
                            "file request message send failed.\n");
                }
                else
                {
                    ret = 0;
                }
            }
            return ret;
        }
    }
}

