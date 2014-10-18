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

#ifndef __NECLIENTCONNECTION_H
#define __NECLIENTCONNECTION_H

/*
  This class is a high level abstraction layer that should be
  used directly by applications wishing to integrate peer to peer
  functionality.  This class wraps all of the lower level NEshare
  protocol into nice and easy to use object methods which accomplish
  all of the major NEshare tasks.
*/

/*
  The abstraction of the client-to-client handling code into a separate
  namespace is to keep the implementation totally independent of the 
  client-to-server networking implementation.  This class directly
  handles client-to-server communication, and dispatches client-to-client
  commnication to the neShareClientThreads namespace methods.  The entry
  point of that namespace is the startThreads method. -N.M.
*/

class neClientConnection
{
  public:
    /*
      This timeout value determines how long to wait for stalled
      communication between the client and the server.
      the timeout is in milliseconds.
    */
    neClientConnection(unsigned long timeout = 30000);
    ~neClientConnection();
   
    /* 
       this client connection *must* be initialized with a function
       pointer which will be responsible for handling network messages
       sent from the server.  Some messages are taken care of behind the
       scenes, but some *must* be handled by the client program using
       this API.  In general, the ACK messages are handled internally,
       but all others must be handled by the calling client program.
       if the incoming argument is NULL, or this method is never called,
       most messages from the server will be ignored, and this would
       be generally useless.
      
       the messages that are handled internally should not be handled by
       the passed in messageHandler routine.  For a complete list,
       see the neClientConnection::handleIncomingMessages method.

       the declaration for the function pointer should be:
       void *(*ncCallBack)(void *ptr) (under Linux only, see ncthread.h
       for the win32 variant).  Example (linux) prototype is:
       void *messageHandler(void *ptr);
       the argument ptr should be cast to a ncSocket ptr when called
       and it should not be saved for later use (i.e. between calls).

       generally, this message handler should peek for a message on the
       socket (using something like neUtils::peekMessage) and handle
       the message if there is one.

       the execution of the listener thread is blocked until the
       function handler returns.  Do not forget to return control
       back to this thread!

       if the callback handler is called with an incoming parameter
       of NULL, then it means that the default handler had an error
       trying to handle a message and the callback handler should
       take appropriate action (such as disconnecting properly
       from the server by using the neClientConnection object since
       the socket will be invalid).

       RETURN VALUES:
       This function reads a config file called "config" in the current
       directory or a file called .neshare/config in the user's home
       directory.  If neither is found, or there is an error writing or 
       reading the file, a value of NE_CONFIG_NOT_FOUND (3) is returned.
 
       In addition, this function allocates several required objects
       for normal operation.  If the allocation or initialization of
       these objects fail, a value of NE_RESOURCE_INIT_FAILED (2)
       is returned.

       Any other failure will be returned with NE_INIT_FAILED (1).

       If this function completes everything properly, a value of
       NE_INIT_OK (0) is returned.

       WARNING: Do not call any method in this class without first
       calling the initialize method!

       -N.M.

       01/18/2002 - N.M.
       Added a nasty hack which messily works around a larger issue.
       In short, the way the ne_common/nemsg* messages are implemented,
       it's possible for a msg header to be sent (e.g. the msgType) -
       for the thread to be pre-empted, a different message sent, and
       then resumed.  The server will be confused because the bytes
       it's reading back do not correspond to a single coherent tokenized
       message.  Thus, the hack was to wrap all msg writes in this class
       in a mutex to ensure that each message is sent atomically.
       A better way to do this is to use an iovec in the message classes 
       so that the data will be guaranteed to be sent all in one piece
       (i.e. the BSD system call writev should be used).  However this
       solution is not portable (i.e. will never work on win32), so 
       a different solution should be considered.
    */
    int initialize(ncCallBack messageHandler);

    /*
      returns 1 if we're initialized and connected to the server.
      returns 0 otherwise
    */
    int isConnected();

    /*
      in the case the calling app knows a reason why we're not
      connected and we don't, allow them to set us in the non
      connected state
    */
    void setDisconnected();

    /*
      a non-zero value specifies that we are connected and a
      zero value specifies that we are not connected.  Note that
      a non-zero value does not mean that isConnected will return
      a non-zero value (since it checks if we're connected *and*
      if the library is initialized).
    */
    void setConnected(int connected);

    /* 
       returns a configuration object ptr with the config
       settings from a file called "config" or "~/.neshare/config".
    */
    neConfig *getConfigObjectPtr();

    /*
      sets the timeout value - usually used if value was not
      passed into constructor.
    */
    void setTimeout(unsigned long msecs);

    /*
      Server communication thread:
      this function starts threads -- one of which polls the socket
      connection to the server (once connected) and it manages several
      internal member variables for signaling when certain ack messages
      are received.  When a peek returns a message that is not handled
      internally, the socket is handed to the function pointer for
      handling outside of this object (which was passed into the
      initialize method)

      Client communication threads:
      another thread monitors connections on the specified
      clientAcceptPort (gathered from the internal neConfig object)
      for communication from one client to another.

      another thread handles all file uploads to all connected peers.

      returns 0 on success; 1 on error
    */
    int startListenerThreads();

    /* attempts to stop the listener threads */
    void stopListenerThreads();

/*** NEshare Client/Server Network Protocol Related Methods ***/

    /* 
       logs into the NEshare server gathered from the neConfig object.
       This function also depends on the server accept port, the
       connection speed, the firewall status and the control port.
       The loginMessage is a buffer provided to retrieve the loginMessage
       from the server, given that the login is successful.  maxlen
       should be the maximum number of bytes that can be copied to
       loginMessage.  To ignore this message, pass a NULL value for
       the loginMessage and a maxlen value of 0.

       returns 0 on success; 1 on error.
       if an error is returned and loginMessage is non-NULL, the error
       message is placed in loginMessage up to maxlen bytes.
    */
    int login(char *loginMessage, int maxlen);

    /* 
       this function reprocesses the config file (including
       a rebuild of the file list). returns 0 on success; 1 on error
    */
    int reprocessConfigFile();

    /*
      this function builds up an internal file list based on the
      directories present in the internal neConfig object
    */
    void buildFileList();

    /*
      this function submits all of the files in each of the directories 
      stored in the global fileManager to the server.
      This function must be called after buildFileList() and after
      login(...).  returns 0 on success; 1 on error
    */
    int submitFileList();

    /*
      sends a query to the server with the keywords handed
      into this method.  The first argument is an array of char 
      pointers and the second is the number of elements in the array.
      returns 0 on success; 1 on failure
    */
    int sendSearchKeywords(char **keywords, int numKeywords);

    /*
      disconnects from the NEshare server if connected.
      to retrieve the server's disconnect message,
      up to maxlen bytes of the message will be placed in
      disconnectMessage.  To ignore the message, pass NULL
      as the disconnectMessage and 0 as maxlen.
      returns 0 on success; 1 on error
    */
    int disconnect(char *disconnectMessage, int maxlen);

/*** NEshare Client/Client Network Protocol Related Methods ***/

    /* 
       requests a file from a peer.  usually the information passed to
       this method is gathered from the results of a previous search.
       The filename must be a protocol encoded filename (as handed
       back from the server).  An example is:

       neshare://research/data/data-log.dat

       While this file format can be spoofed by a malicious user,
       it should be safe since the receiving client must match the
       exact filename in its filemanager (map lookup on filename)
       before the file can be retrieved.

       If the file exists already locally, this call will
       attempt to automatically resume the file download.

       returns 0 on success; 1 on failure
    */
    int requestPeerFile(unsigned long peerIpAddr,
                        unsigned long peerCtrlPort,
                        unsigned long firewallStatus,
                        unsigned long id,
                        char *filename);

    /*
      sends a cancel request to another peer who is currently sending
      the file which corresponds to the specified information
    */
    int cancelPeerDownload(unsigned long peerIpAddr, char *filename);

    /*
      cancels a file upload matching the fileID
      being uploaded to the specified peer
    */
    int cancelPeerUpload(nePeer *peer, unsigned long fileID);

/*** NEshare Misc. Methods ***/
    ncSocket *getSocket();

    /*
      this registers a callback which is called with progress status info
      for each upload in transmission at any given time.  This must not
      be called before startListenerThreads is called. 
      returns 1 on error; 0 on success.
    */
    int registerUploadCallback(neUploadCallback fnptr);

    /*
      this determines how often the registered
      upload callback is called.
    */
    void registerUploadCallbackDelay(unsigned long millisecs);

    /*
      this registers a callback which is called with progress status
      info for each download in transmission at any given time.
      This must not be called before startListenerThreads is called.
      returns 1 on error; 0 on success.
    */
    int registerDownloadCallback(neDownloadCallback fnptr);

    /*
      this determines how often the registered
      download callback is called.
    */
    void registerDownloadCallbackDelay(unsigned long millisecs);

    /*
      this registers a listener for outputted message from within the
      neshare library.  The levels of registration include
      NESYSLOG_LEVEL_DEBUG, NESYSLOG_LEVEL_INFO, or NESYSLOG_LEVEL_ERROR.
      You can OR these values together if you're interested in listening
      to more than one kind of outputted message.  The callback passed
      in is called with the message each time a message is internally
      emitted.
     
      The callback should be defined as follows:
      void callbackFunc(void *data, char *msg);
     
      for now, the data argument will always be NULL. 
      returns 1 on error; 0 on success
    */
    int registerSysLogListener(unsigned long levels,
                               actionFunc callback);

    /*
      unregisters a callback previously registered that matches
      both the registered callback and the associated data
    */
    void unregisterSysLogListener(unsigned long levels,
                                  actionFunc callback);


/*** NEshare undocumented public functions - do not call ***/
    void handleIncomingMessages();
  private:

    unsigned long syncWithServer(unsigned long type, 
                                 unsigned long timeout);
    nePeer *loginToPeer(unsigned long peerIpAddr,
                        unsigned long peerCtrlPort);
    int sendFileRequest(nePeer *peer, char *filename);
    int handleFirewalledRequest(unsigned long peerIpAddr,
                                unsigned long peerCtrlPort,
                                unsigned long firewallStatus,
                                unsigned long id,
                                char *filename);

    int handleNonFirewalledRequest(unsigned long peerIpAddr,
                                   unsigned long peerCtrlPort,
                                   unsigned long firewallStatus,
                                   char *filename);

    int m_initialized;
    int m_connected;

    unsigned long m_timeout;
    ncSocket *m_sock;
    ncThread m_incomingMessageThread;
    unsigned long m_incomingMessageThreadReady;
    ncCallBack m_messageHandler;
    neUploadCallback m_uploadCallback;
    neDownloadCallback m_downloadCallback;
    
    /* internal message handling sync variables */
    unsigned long m_serverLoginSync;
    unsigned long m_serverDisconnectSync;

    /* internal objects that are required for operation */
    neConfig *m_config;
    neFileManager *m_fileManager;
    nePeerManager *m_peerClientManager;
    nePeerManager *m_peerServentManager;
    nePeerDownloadManager *m_peerDownloadManager;
    nePeerUploadManager *m_peerUploadManager;

    /*
      required for synchronization between calls
      into the class methods (where necessary)
    */
    ncMutex m_callMutex;

    /* synchronizes socket writes on server communications */
    ncMutex m_ioWriteMutex;
};

/*
  this is used internally only and should be
  treated as a private method only
*/
void *startIncomingMessageHandler(void *ptr);

#endif /* __NECLIENTCONNECTION_H */
