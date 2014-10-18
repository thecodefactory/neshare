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

/*
  a global sys logging object through which all messages should
  be output.  This class allows registration as a listener to
  external applications.
*/
neSysLog g_syslog;

neClientConnection::neClientConnection(unsigned long timeout)
{
    m_timeout = timeout;
    m_sock = (ncSocket *)0;
    m_messageHandler = (ncCallBack)0;
    m_incomingMessageThreadReady = 0;
    m_config = (neConfig *)0;
    m_fileManager = (neFileManager *)0;
    m_peerClientManager = (nePeerManager *)0;
    m_peerServentManager = (nePeerManager *)0;
    m_peerDownloadManager = (nePeerDownloadManager *)0;
    m_peerUploadManager = (nePeerUploadManager *)0;
    m_initialized = 0;
    m_connected = 0;
    m_serverLoginSync = 0;
    m_serverDisconnectSync = 0;
}

neClientConnection::~neClientConnection()
{
    delete m_sock;
    delete m_config;
    delete m_fileManager;
    delete m_peerClientManager;
    delete m_peerServentManager;
    delete m_peerDownloadManager;
    delete m_peerUploadManager;
}

int neClientConnection::initialize(ncCallBack messageHandler)
{
    int ret = NE_RESOURCE_INIT_FAILED;

    /* lock the callMutex -- absolutely required */
    if (m_callMutex.trylock() == NC_OK)
    {
        m_messageHandler = messageHandler;

        m_config = new neConfig();
        if (!m_config)
        {
            m_callMutex.unlock();
            return NE_RESOURCE_INIT_FAILED;
        }

        /* first try to read a config file in the current directory */
        if (m_config->readConfigFile("config"))
        {
            /* if that fails, try to open $HOME/.neshare/config */
            char *homedir = (char *)getenv("HOME");
            std::string homeConfigFile = std::string(homedir) +
                "/.neshare/config";
            if (!homedir || 
                (m_config->readConfigFile(
                    (char *)homeConfigFile.c_str())))
            {
                /*
                  if no file is found, create one in the current
                  directory with all of the constructor defaults
                  and read it back in.
                */
                if ((m_config->writeConfigFile("config",
                                               NECFG_CLIENT_FIELDS)) ||
                    (m_config->readConfigFile("config")))
                {
                    eprintf("Cannot open config file.\nMake sure "
                            "that one exists and is either in ./config "
                            "or %s/.neshare/config\n",homedir);
                    delete m_config;
                    m_callMutex.unlock();
                    return NE_CONFIG_NOT_FOUND;
                }
                else
                {
                    iprintf("Wrote default configuration file.\n");
                    iprintf("Using ./config\n");
                }
            }
            else
            {
                iprintf("Found configuration file.\n");
                iprintf("Using %s\n",(char *)homeConfigFile.c_str());
            }
        }
        else
        {
            iprintf("Found configuration file.\n");
            iprintf("Using ./config\n");
        }

        /* make sure all allocations succeed - or else terminate */
        m_fileManager = new neFileManager();
        m_peerClientManager =
            new nePeerManager(m_config->getMaxNumPeers());
        m_peerServentManager =
            new nePeerManager(m_config->getMaxNumPeers());
        m_peerDownloadManager =
            new nePeerDownloadManager(); 
        m_peerUploadManager =
            new nePeerUploadManager(m_config,m_fileManager); 
   
        if (!m_fileManager ||
            !m_peerClientManager || m_peerClientManager->initialize() ||
            !m_peerServentManager || m_peerServentManager->initialize() ||
            !m_peerDownloadManager ||
            !m_peerUploadManager)
        {
            eprintf("Error: Cannot allocate required managers.\n");
            delete m_config;
            delete m_fileManager;
            delete m_peerClientManager;
            delete m_peerServentManager;
            delete m_peerDownloadManager;
            delete m_peerUploadManager;
            m_callMutex.unlock();
            return NE_RESOURCE_INIT_FAILED;
        }

        m_initialized = 1;
        ret = NE_INIT_OK;
    }
    m_callMutex.unlock();
    return ret;
}

int neClientConnection::reprocessConfigFile()
{
    int ret = 1;

    /* lock the callMutex -- absolutely required */
    if (m_callMutex.trylock() == NC_OK)
    {
        /* stop all running threads */
        stopListenerThreads();

        /* delete all previously allocated objects */
        delete m_sock;
        delete m_config;
        delete m_fileManager;
        delete m_peerClientManager;
        delete m_peerServentManager;
        delete m_peerDownloadManager;
        delete m_peerUploadManager;

        /* re-initialize the entire library */
        m_sock = (ncSocket *)0;
        m_incomingMessageThreadReady = 0;
        m_config = (neConfig *)0;
        m_fileManager = (neFileManager *)0;
        m_peerClientManager = (nePeerManager *)0;
        m_peerServentManager = (nePeerManager *)0;
        m_peerDownloadManager = (nePeerDownloadManager *)0;
        m_peerUploadManager = (nePeerUploadManager *)0;
        m_initialized = 0;
        m_connected = 0;
        m_serverLoginSync = 0;
        m_serverDisconnectSync = 0;

        if (m_ioWriteMutex.isLocked())
        {
            m_ioWriteMutex.unlock();
        }

        m_callMutex.unlock();
        ret = this->initialize(m_messageHandler);
        m_callMutex.lock();

        if (ret == 0)
        {
            /* rebuild the file list */
            m_callMutex.unlock();
            this->buildFileList();
            m_callMutex.lock();

            /* and restart all required threads */
            if (startListenerThreads() == 0)
            {
                /* finally, re-install the user callbacks */
                registerUploadCallback(m_uploadCallback);
                registerDownloadCallback(m_downloadCallback);
                ret = 0;
            }
        }
        m_callMutex.unlock();
    }
    return ret;
}

int neClientConnection::isConnected()
{
    return (m_initialized && m_connected);
}

void neClientConnection::setDisconnected()
{
    m_connected = 0;
}

void neClientConnection::setConnected(int connected)
{
    m_connected = connected;
}

neConfig *neClientConnection::getConfigObjectPtr()
{
    return m_config;
}

void neClientConnection::setTimeout(unsigned long msec)
{
    m_timeout = msec;
}

int neClientConnection::startListenerThreads()
{
    int ret = 1;

    /*
      start the server socket listener thread - which handles
      incoming messages from the server, or dispatches them to
      the handler provided in the call to initialize. 
    */
    if (m_incomingMessageThread.start
        (startIncomingMessageHandler,(void *)this) == NC_OK)
    {
        /* now detach it so it can do its thing independently */
        if (m_incomingMessageThread.detach() == NC_OK)
        {
            /* second, start the client-to-client communication threads */
            neShareClientThreads::startThreads(m_config,
                                               m_peerClientManager,
                                               m_peerServentManager,
                                               m_peerDownloadManager,
                                               m_peerUploadManager);
            ret = 0;
        }
    }
    return ret;
}

void neClientConnection::stopListenerThreads()
{
    /* first shut down our client-server comm thread */
    m_incomingMessageThreadReady = 0;
    ncSleep(100);
    m_incomingMessageThread.stop(0);

    /* then shut down the client-client comm threads */
    neShareClientThreads::stopThreads();
}

int neClientConnection::login(char *loginMessage, int maxlen)
{
    int ret = 1;

    iprintf("neClientConnection::login | Trying server %s\n",
            m_config->getServerAddress());

    delete m_sock;
    m_sock = new ncSocket(m_config->getServerAddress(),
                          m_config->getServerAcceptPort(),
                          SOCKTYPE_TCPIP);
    if (m_sock && (m_sock->connect() == NC_OK))
    {
        m_sock->setTimeout(m_timeout);

        /* send the login message */
        nemsgLogin login(m_sock);
        m_ioWriteMutex.lock();
        if (login.send(m_config->getClientConnectionSpeed(),
                       m_config->getClientFirewallStatus(),
                       m_config->getClientControlPort()) == 0)
        {
            m_ioWriteMutex.unlock();

            /* signal to the recv thread that it should start receiving */
            m_incomingMessageThreadReady = 1;
            ncSleep(100);

            /*
              wait for the server sync variable to be set
              by handleIncomingMessages
            */
            if (syncWithServer(NE_SYNC_LOGIN,m_timeout*2) ==
                NE_SYNC_LOGIN)
            {
                /*
                  fill in the message if it's desired
                  (for success or failure case)
                */
                nemsgLoginAck loginAck(m_sock);
                nemsgLoginFailed loginFailed(m_sock);
                if (neUtils::peekMessage(m_sock) == NE_MSG_LOGIN_ACK)
                {
                    if (loginAck.recv(loginMessage,maxlen) == 0)
                    {
                        m_connected = 1;
                        ret = 0;
                    }
                }
                else if (neUtils::peekMessage(m_sock) ==
                         NE_MSG_LOGIN_FAILED)
                {
                    loginFailed.recv(loginMessage,maxlen);
                    m_connected = 0;
                    ret = 1;
                }
                else
                {
                    m_connected = 0;
                    ret = 1;
                    eprintf("Received unexpected message type %x\n",
                            neUtils::peekMessage(m_sock));
                }
            }
        }
        else
        {
            m_ioWriteMutex.unlock();
        }
    }
    return ret;
}

int neClientConnection::sendSearchKeywords(char **keywords,
                                           int numKeywords)
{
    int ret = 1;

    if (isConnected() && keywords && numKeywords)
    {
        /* send the search query to the server */
        nemsgSearchQuery searchQueryMsg(m_sock);
        unsigned long *typeFlags = (unsigned long *)
            malloc(numKeywords*sizeof(unsigned long));
        if (typeFlags)
        {
            /* for now we ignore typeflags... */
            for(int i = 0; i < numKeywords; i++)
            {
                typeFlags[i] = 0x00000000;
            }
            m_ioWriteMutex.lock();
            ret = searchQueryMsg.send(numKeywords,typeFlags,keywords);
            m_ioWriteMutex.unlock();
            free(typeFlags);
        }
    }
    return ret;
}

void neClientConnection::buildFileList()
{
    m_callMutex.lock();
    std::vector<std::string> *directories =
        m_config->getDirectories();
    if (directories)
    {
        m_fileManager->reset();

        std::vector<std::string>::iterator iter;
        for(iter = directories->begin(); iter != directories->end();
            iter++)
        {
            if (m_fileManager->addDirectory((char *)(*iter).c_str(),1))
            {
                eprintf("neClientConnection::buildFileList "
                        "failed on %s. Skipping\n",(*iter).c_str());
            }
        }
    }
    m_callMutex.unlock();
    iprintf("neClientConnection::buildFileList | Built File List "
            "of %d Entries.\n",m_fileManager->getNumFiles());
}
   
int neClientConnection::submitFileList()
{
    int ret = 1;
    int actualEntriesSent = 0;
    int retryErrorCount = 0;

    nemsgEntry entry(m_sock);
    nemsgEntrySetStart entrySetStart(m_sock);
    nemsgEntrySetEnd entrySetEnd(m_sock);
    NESHARE_FILE_OBJ *fileObj = (NESHARE_FILE_OBJ *)0;

    iprintf("Submitting file list.\n");
    if (isConnected() && m_fileManager)
    {
        m_ioWriteMutex.lock();
        if (entrySetStart.send(m_fileManager->getNumFiles()) == 0)
        {
            m_ioWriteMutex.unlock();

            m_fileManager->resetEncodedFileObjPtr();
            while((fileObj = m_fileManager->getNextEncodedFileObj()))
            {
                /* make sure the encoded filename is encoded properly */
                if (memcmp(fileObj->encodedFilename,"neshare://",10) != 0)
                {
                    eprintf("neClientConnection::submitFileList | Invalid "
                            "file encoding: %s.  Removing.\n",
                            fileObj->encodedFilename);
                    m_fileManager->removeEncodedFile(fileObj);
                    continue;
                }

                /*
                  try a few times to send the msg
                  and give up if we can't
                */
                retryErrorCount = 0;
                m_ioWriteMutex.lock();
                while (entry.send(fileObj->encodedFilename,
                                  fileObj->fileSize))
                {
                    if (++retryErrorCount == /* config option? */5)
                    {
                        eprintf("neClientConnection::submitFileList | "
                                "Failed to send: %s (attempt %d)\n",
                                fileObj->encodedFilename,retryErrorCount);
                        m_ioWriteMutex.unlock();
                        return ret;
                    }
                }
                m_ioWriteMutex.unlock();
                ++actualEntriesSent;
            }
            m_ioWriteMutex.lock();
            ret = entrySetEnd.send(actualEntriesSent);
            m_ioWriteMutex.unlock();
        }
        else
        {
            m_ioWriteMutex.unlock();
        }
    }
    return ret;
}

int neClientConnection::disconnect(char *disconnectMessage, int maxlen)
{
    int ret = 1;

    if (isConnected())
    {
        /* send the disconnect message to the server */
        nemsgDisconnect disconnect(m_sock);
        m_ioWriteMutex.lock();
        if (disconnect.send() == 0)
        {
            m_ioWriteMutex.unlock();

            /*
              wait for the server sync variable to be
              set by handleIncomingMessages
            */
            if (syncWithServer(NE_SYNC_DISCONNECT,m_timeout*2) ==
                NE_SYNC_DISCONNECT)
            {
                /*
                  fill in the message if it's desired
                  (for success or failure case)
                */
                nemsgDisconnectAck disconnectAck(m_sock);
                if (neUtils::peekMessage(m_sock) == NE_MSG_DISCONNECT_ACK)
                {
                    if (disconnectAck.recv(disconnectMessage,maxlen) == 0)
                    {
                        m_connected = 0;
                        ret = 0;
                    }
                }
            }
        }
        else
        {
            m_ioWriteMutex.unlock();
        }
    }
    return ret;
}

ncSocket *neClientConnection::getSocket()
{
    /*
      note that this socket is the one connecting this
      client to the server only.
    */
    return m_sock;
}

void neClientConnection::handleIncomingMessages()
{
    /* wait until we get the signal that we're good to go */
    while(!m_incomingMessageThreadReady)
    {
        ncSleep(10);
    }

    /* declare some message objects that will be needed later */
    nemsgPing pingMsg((ncSocket *)0);
    nemsgPong pongMsg((ncSocket *)0);

    unsigned long msgType = 0;
    unsigned long ipAddr = 0;
    unsigned long ctrlPort = 0;
    unsigned long id = 0;
    nePeer *peer = (nePeer *)0;
    char pushRequestFilename[NE_MSG_MAX_DATA_LEN];
    nemsgPushRequest *pushRequestMsg = (nemsgPushRequest *)0;
    nemsgPushRequestAck *pushRequestAckMsg = (nemsgPushRequestAck *)0;

    /* take control of the connection socket for server communications */
    while(m_incomingMessageThreadReady)
    {
        while((msgType = neUtils::peekMessage(m_sock)))
        {
            switch(msgType)
            {
                case NE_MSG_DISCONNECT_ACK:
                    m_serverDisconnectSync = 1;
                    /* give the sync thread some time to handle this */
                    ncSleep(10);
                    break;
                case NE_MSG_LOGIN_ACK:
                    m_serverLoginSync = 1;
                    /* give the sync thread some time to handle this */
                    ncSleep(10);
                    break;
                case NE_MSG_LOGIN_FAILED:
                    m_serverLoginSync = 1;
                    /* give the sync thread some time to handle this */
                    ncSleep(10);
                    break;
                case NE_MSG_PUSH_REQUEST:
                    /*
                      at this point, we know that we were indirectly
                      instructed to request the specified file from
                      a non-firewalled user
                    */
                    iprintf("neClientConnection::handleIncomingMessages |"
                            " Push message received.\n");
                    pushRequestMsg = new nemsgPushRequest(m_sock);
                    if (pushRequestMsg->recv(&ipAddr,&ctrlPort,&id,
                                             pushRequestFilename,
                                             NE_MSG_MAX_DATA_LEN) == 0)
                    {
                        iprintf("Logging into %s\n",
                                neUtils::ipToStr(ipAddr));
                        peer = loginToPeer(ipAddr,ctrlPort);
                        if (peer)
                        {
                            /* send a push request ack message */
                            pushRequestAckMsg =
                                new nemsgPushRequestAck(peer->getSocket());
                            m_ioWriteMutex.lock();
                            if (pushRequestAckMsg->send(pushRequestFilename))
                            {
                                eprintf("neClientConnection::"
                                        "handleIncomingMessages | Failed "
                                        "to send push requst ack "
                                        "message\n");
                            }
                            m_ioWriteMutex.unlock();
                            delete pushRequestAckMsg;
                        }
                    }
                    delete pushRequestMsg;
                    break;
                case NE_MSG_PING:
                    pingMsg.setSocket(m_sock);
                    if (pingMsg.recv())
                    {
                        eprintf("neClientConnection::handleIncomingMessages "
                                "| Ping recv failed.\n");
                        (*m_messageHandler)((ncSocket *)0);
                        break;
                    }
                    pongMsg.setSocket(m_sock);
                    m_ioWriteMutex.lock();
                    if (pongMsg.send())
                    {
                        eprintf("neClientConnection::handleIncomingMessages "
                                "| Pong send failed.\n");
                        (*m_messageHandler)((ncSocket *)0);
                    }
                    m_ioWriteMutex.unlock();
                    break;
                default:
                    /*
                      pass any messages we don't handle here
                      to the client-registered message handler
                    */
                    if (m_messageHandler)
                    {
                        (*m_messageHandler)(m_sock);
                    }

            }
            ncThread::testCancel();
        }
        ncSleep(10);
    }
}

int neClientConnection::requestPeerFile(unsigned long peerIpAddr,
                                        unsigned long peerCtrlPort, 
                                        unsigned long firewallStatus,
                                        unsigned long id,
                                        char *filename)
{
    int ret = 1;

    if (isConnected())
    {
        if (firewallStatus)
        {
            /* firewalled requests require use of the id argument */
            ret = handleFirewalledRequest(peerIpAddr,peerCtrlPort,
                                          firewallStatus,id,filename);
        }
        else
        {
            ret = handleNonFirewalledRequest(peerIpAddr,peerCtrlPort,
                                             firewallStatus,filename);
        }
    }
    /*
      we return ok here because neShareClientThreads::processClientPeers
      will continue to initiate the file download if an ack to this
      file request message comes back and is valid
    */
    return ret;
}

void *startIncomingMessageHandler(void *ptr)
{
    neClientConnection *clientConnection = (neClientConnection *)ptr;
    if (clientConnection)
    {
        clientConnection->handleIncomingMessages();
    }
    return (void *)0;
}

int neClientConnection::registerUploadCallback(neUploadCallback fnptr)
{
    int ret = 1;
    if (m_peerUploadManager && fnptr)
    {
        m_peerUploadManager->registerCallback(fnptr);
        m_uploadCallback = fnptr;
        ret = 0;
    }
    return ret;
}

int neClientConnection::registerDownloadCallback(neDownloadCallback fnptr)
{
    int ret = 1;
    if (m_peerDownloadManager && fnptr)
    {
        m_peerDownloadManager->registerCallback(fnptr);
        m_downloadCallback = fnptr;
        ret = 0;
    }
    return ret;
}

void neClientConnection::registerUploadCallbackDelay(
    unsigned long millisecs)
{
    if (m_peerUploadManager)
    {
        m_peerUploadManager->setCallbackDelay(millisecs);
    }
}

void neClientConnection::registerDownloadCallbackDelay(
    unsigned long millisecs)
{
    if (m_peerDownloadManager)
    {
        m_peerDownloadManager->setCallbackDelay(millisecs);
    }
}

int neClientConnection::registerSysLogListener(unsigned long levels,
                                               actionFunc callback)
{
    return (g_syslog.registerListener(levels,callback,0x00000000));
}

void neClientConnection::unregisterSysLogListener(unsigned long levels,
                                                  actionFunc callback)
{
    g_syslog.unregisterListener(levels,callback,0x00000000);
}


unsigned long neClientConnection::syncWithServer(unsigned long type,
                                                 unsigned long timeout)
{
    unsigned long wait = 0;
    /* 
       increment timeout by one round to make up for a
       possible situation where a check is failed at the
       same time the variable is changed.  By incrementing
       by one round, we'll get it properly the next time around.
    */
    timeout += 250;
    do
    {
        if (type == NE_SYNC_LOGIN)
        {
            if (m_serverLoginSync)
            {
                m_serverLoginSync = 0;
                break;
            }
        }
        else if (type == NE_SYNC_DISCONNECT)
        {
            if (m_serverDisconnectSync)
            {
                m_serverDisconnectSync = 0;
                break;
            }
        }
        ncSleep(250);
        wait += 250;
    } while(wait < timeout);
    return ((wait >= timeout) ? NE_SYNC_TIMEOUT : type);
}

int neClientConnection::cancelPeerDownload(unsigned long ipaddr,
                                           char *filename)
{
    int ret = 1;
    if (filename)
    {
        ret = m_peerDownloadManager->cancelPeerDownload(ipaddr,
                                                        filename,1);
    }
    return ret;
}

int neClientConnection::cancelPeerUpload(nePeer *peer, 
                                         unsigned long fileID)
{
    int ret = 1;
    if (peer)
    {
        ret = m_peerUploadManager->cancelPeerUpload(peer,fileID,1);
    }
    return ret;
}

int neClientConnection::handleFirewalledRequest(unsigned long peerIpAddr,
                                                unsigned long peerCtrlPort,
                                                unsigned long firewallStatus,
                                                unsigned long id,
                                                char *filename)
{
    int ret = 1;

    if (isConnected())
    {
        /*
          send the server a message which instructs it to send a message
          to the user we specify here to connect to us and upload the
          specified file our way.  (this is a simple push request)
        */
        nemsgPushRequest pushRequestMsg(m_sock);
        m_ioWriteMutex.lock();
        ret = pushRequestMsg.send(peerIpAddr,peerCtrlPort,id,filename);
        m_ioWriteMutex.unlock();
        iprintf("neClientConnection::handleFirewalledRequest "
                "| Push message sent.\n");
    }
    return ret;
}

int neClientConnection::handleNonFirewalledRequest(
    unsigned long peerIpAddr,
    unsigned long peerCtrlPort,
    unsigned long firewallStatus,
    char *filename)
{
    int ret = 1;

    nePeer *peer = loginToPeer(peerIpAddr,peerCtrlPort);
    if (peer)
    {
        m_ioWriteMutex.lock();
        ret = neShareClientThreads::neClientUtils::sendFileRequest(
            peer,m_peerDownloadManager,filename);
        m_ioWriteMutex.unlock();
    }
    else
    {
        eprintf("neClientConnection::requstPeerFile | "
                "cannot connect to peer\n");
    }
    return ret;
}

nePeer *neClientConnection::loginToPeer(unsigned long peerIpAddr,
                                        unsigned long peerCtrlPort)
{
    /* check if we're already connected to the peer */
    nePeer *peer = m_peerClientManager->findPeer(peerIpAddr);
    if (!peer)
    {
        /*
          if not, create a new peer object, attempt to connect to
          the peer, login, and update internal book keeping.
          NOTE: Make the timeout configurable using
          g_config->something()
        */
        peer = neShareClientThreads::neClientUtils::getNewConnectedPeer
            (peerIpAddr,peerCtrlPort,5000);
        if (!peer)
        {
            eprintf("neClientConnection::loginToPeer | "
                    "Connection to peer cannot be made.\n");
            return (nePeer *)0;
        }
        else if (neShareClientThreads::neClientUtils::handleConnectedPeerLogin
                 (peer,m_timeout))
        {
            eprintf("neClientConnection::loginToPeer | "
                    "Peer login attempt failed.\n");
            delete peer;
            return (nePeer *)0;
        }
        else if (m_peerClientManager->addPeer(peer))
        {
            eprintf("neClientConnection::loginToPeer | "
                    "Peer failed to be added to peer client manager.\n");
            delete peer;
            return (nePeer *)0;
        }
    }
    return peer;
}
