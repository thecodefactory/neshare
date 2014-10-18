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

#ifndef __NECONFIG_H
#define __NECONFIG_H

/*
  this is a generic configuration object used for reading and
  writing configuration files for the client and server.
*/

#define CAN_IGNORE_LINE(line) \
  ((line[0] == '#')  || (line[0] == ' ') || \
   (line[0] == '\r') || (line[0] == '\n'))

#define GENERIC_BUF_LEN            255
#define GENERIC_OPTION_LEN          32
#define GENERIC_VALUE_LEN           32

#define NECFG_CLIENT_FIELDS 0x00000001
#define NECFG_SERVER_FIELDS 0x00000002


class neConfig
{
  public:
    neConfig();
    ~neConfig();

    /* resets the state of this object back to defaults */
    void reset();

    /* reads a config file.  returns 0 on success;
       1 on error */
    int readConfigFile(char *configFile);

    /* return the respective fields */

    /* server fields */
    int getMaxNumUsers();
    int getUserTTL();
    int getUserPingDelay();
    int getUserStatusDelay();
    int getMaxResults();
    int getMinUserSharedFileLimit();
    int getNumThreadedHandlers();
    int getNumUserManagers();
    int getMaxMessagesToHandle();
    int getNumReservedUserFiles();
    int getUserMsgThrottle();
    char *getGoodbyeMessageFile();
    char *getGreetingMessageFile();
    char *getLoginFailedMessageFile();
    char *getInvalidMessageTypeFile();
    char *getUserFilePageFile();
    char *getServerLog();

    /* shared fields */
    int getServerAcceptPort();

    /* client fields */
    int getClientControlPort();
    int getClientForwardingPort();
    int getClientFirewallStatus();
    int getClientConnectionSpeed();
    int getMaxNumPeers();
    int getShowFilePath();
    int getClientMaxNumSends();
    int getClientMaxNumRecvs();
    char *getServerAddress();
    char *getSavePath();
    std::vector<std::string> *getDirectories();

    /* 
       writes a config file. returns 0 on success; 1 on error.
       if fieldType == NECFG_CLIENT_FIELDS, client fields are written.
       if fieldType == NECFG_SERVER_FIELDS, server fields are written.
       if fieldType == NECFG_CLIENT_FIELDS | NECFG_SERVER_FIELDS, all
       fields are written.  Shared fields are written in both.
    */
    int writeConfigFile(char *configFile, int fieldType);

    /* set the respective fields */

    /* server fields */
    void setMaxNumUsers(int maxNumUsers);
    void setUserTTL(int userTTL);
    void setUserPingDelay(int userPingDelay);
    void setUserStatusDelay(int userStatusDelay);
    void setMaxResults(int maxResults);
    void setMinUserSharedFileLimit(int minUserSharedFileLimit);
    void setNumThreadedHandlers(int numThreadedHandlers);
    void setNumUserManagers(int numUserManagers);
    void setMaxMessagesToHandle(int maxMessageToHandle);
    void setGoodbyeMessageFile(char *msg);
    void setGreetingMessageFile(char *msg);
    void setLoginFailedMessageFile(char *msg);
    void setInvalidMessageTypeFile(char *msg);
    void setServerLog(char *serverlogfile);
    void setUserFilePageFile(char *pagefile);
    void setNumReservedUserFiles(int num);
    void setUserMsgThrottle(int num);

    /* shared fields */
    void setServerAcceptPort(int serverAcceptPort);

    /* client fields */
    void setClientControlPort(int clientControlPort);
    void setClientForwardingPort(int clientForwardingPort);
    void setClientFirewallStatus(int firewallStatus);
    void setClientConnectionSpeed(int connection);
    void setMaxNumPeers(int maxNumPeers);
    void setShowFilePath(int showFilePath);
    void setServerAddress(char *serverAddr);
    void addDirectory(char *directory);
    void setSavePath(char *savePath);
    void setClientMaxNumSends(int maxNumSends);
    void setClientMaxNumRecvs(int maxNumRecvs);

  private:
    void emptyDirectories();

    /* server members */
    int m_maxNumUsers;
    int m_userTTL;
    int m_userPingDelay;
    int m_userStatusDelay;
    int m_maxResults;
    int m_minUserSharedFileLimit;
    int m_numThreadedHandlers;
    int m_numUserManagers;
    int m_maxMessagesToHandle;
    int m_numReservedUserFiles;
    int m_userMsgThrottle;
    int m_clientMaxNumSends;
    int m_clientMaxNumRecvs;
    char *m_goodbyeMessageFile;
    char *m_greetingMessageFile;
    char *m_loginFailedMessageFile;
    char *m_invalidMessageTypeFile;

    /* shared members */
    int m_serverAcceptPort;

    /* client members */
    int m_clientControlPort;
    int m_clientForwardingPort;
    int m_clientFirewallStatus;
    int m_clientConnectionSpeed;
    int m_maxNumPeers;
    int m_showFilePath;

    char m_serverAddress[NESERVER_MAX_NAME_LEN];
    char m_savePath[NE_MSG_MAX_DATA_LEN];
    char m_serverLog[NE_MSG_MAX_DATA_LEN];
    char m_userFilePageFile[NE_MSG_MAX_DATA_LEN];
    std::vector<std::string> m_directories;
};

#endif /* __NECONFIG_H */
