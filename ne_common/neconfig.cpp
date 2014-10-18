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

#include "necommonheaders.h"

neConfig::neConfig()
{
    m_goodbyeMessageFile = strdup(NE_CONFIG_GOODBYE_MESSAGE);
    m_greetingMessageFile = strdup(NE_CONFIG_GREETING_MESSAGE);
    m_loginFailedMessageFile = strdup(NE_CONFIG_LOGIN_FAILED_MESSAGE);
    m_invalidMessageTypeFile = strdup(NE_CONFIG_INVALID_MESSAGE_TYPE);

    assert(m_goodbyeMessageFile);
    assert(m_greetingMessageFile);
    assert(m_loginFailedMessageFile);
    assert(m_invalidMessageTypeFile);

    reset();
}

neConfig::~neConfig()
{
    reset();

    if (m_goodbyeMessageFile)
    {
        free(m_goodbyeMessageFile);
        m_goodbyeMessageFile = (char *)0;
    }
    if (m_greetingMessageFile)
    {
        free(m_greetingMessageFile);
        m_greetingMessageFile = (char *)0;
    }
    if (m_loginFailedMessageFile)
    {
        free(m_loginFailedMessageFile);
        m_loginFailedMessageFile = (char *)0;
    }
    if (m_invalidMessageTypeFile)
    {
        free(m_invalidMessageTypeFile);
        m_invalidMessageTypeFile = (char *)0;
    }
}

void neConfig::reset()
{
    m_maxNumUsers = 128;
    m_userTTL = 180;
    m_userPingDelay = 180;
    m_userStatusDelay = 30;
    m_maxResults = 1000;
    m_serverAcceptPort = 12414;
    m_clientControlPort = 12415;
    m_clientForwardingPort = 0;
    m_clientFirewallStatus = 0;
    m_clientConnectionSpeed = 0;
    m_clientMaxNumSends = 4;
    m_clientMaxNumRecvs = 4;
    m_maxNumPeers = 5;
    m_showFilePath = 0;
    m_minUserSharedFileLimit = 0;
    m_numThreadedHandlers = 4;
    m_numUserManagers = 4;
    m_maxMessagesToHandle = 1;
    m_userMsgThrottle = 10000;
    m_numReservedUserFiles = 100000;
    memset(m_serverAddress,0,NESERVER_MAX_NAME_LEN*sizeof(char));
    memcpy(m_serverAddress,"localhost",strlen("localhost"));
    memset(m_savePath,0,NE_MSG_MAX_DATA_LEN*sizeof(char));
#ifdef WIN32
    strcpy(m_savePath,"C:\\temp");
#else
    strcpy(m_savePath,"/tmp");
#endif
    memset(m_serverLog,0,NE_MSG_MAX_DATA_LEN*sizeof(char));
    strcpy(m_serverLog,"neshare.log");
    memset(m_userFilePageFile,0,NE_MSG_MAX_DATA_LEN*sizeof(char));
#ifdef WIN32
    strcpy(m_userFilePageFile,"C:\\temp\\ufpage");
#else
    strcpy(m_userFilePageFile,"/tmp/ufpage");
#endif

    emptyDirectories();
}

int neConfig::readConfigFile(char *configFile)
{
    int  ret = 0;

    FILE *fd = fopen(configFile,"r");
    if (fd)
    {
        char line[GENERIC_BUF_LEN] = {0};
        char option[GENERIC_BUF_LEN] = {0};
        char value[GENERIC_BUF_LEN] = {0};
        std::string str;

        while(fgets(line,GENERIC_BUF_LEN,fd))
        {
            if (CAN_IGNORE_LINE(line))
            {
                continue;
            }

            if (memcmp(line,"DIRECTORY",9*sizeof(char)) == 0)
            {
                if (sscanf(line,"DIRECTORY = %s",value))
                {
                    m_directories.push_back(std::string(value));
                    ret++;
                }
                else
                {
                    eprintf("DIRECTORY LINE FAILED: %s | %s\n",line,value);
                }
            }
            else if (memcmp(line,"SERVER ",7*sizeof(char)) == 0)
            {
                if (sscanf(line,"SERVER = %s",value))
                {
                    strncpy(m_serverAddress,value,NESERVER_MAX_NAME_LEN);
                    ret++;
                }
                else
                {
                    eprintf("SERVER LINE FAILED: %s | %s\n",line,value);
                }
            }
            else if (memcmp(line,"SAVEPATH",8*sizeof(char)) == 0)
            {
                if (sscanf(line,"SAVEPATH = %s",value))
                {
                    strncpy(m_savePath,value,NE_MSG_MAX_DATA_LEN);
                    ret++;
                }
                else
                {
                    eprintf("SAVEPATH LINE FAILED: %s | %s\n",line,value);
                }
            }
            else if (memcmp(line, "SERVERLOG", 9*sizeof(char)) == 0)
            {
                if (sscanf(line, "SERVERLOG = %s", value))
                {
                    strncpy(m_serverLog,value,NE_MSG_MAX_DATA_LEN);
                    ret++;
                }
                else
                {
                    eprintf("SERVERLOG LINE FAILED: %s | %s\n", line, value);
                }
            }
            else if (memcmp(line,"SERVERACCEPTPORT",16*sizeof(char)) == 0)
            {
                if (sscanf(line,"SERVERACCEPTPORT = %s",value))
                {
                    m_serverAcceptPort = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("SERVERACCEPTPORT LINE FAILED: %s | %s\n",
                            line,value);
                }
            }
            else if (memcmp(line,"CLIENTCONTROLPORT",17*sizeof(char)) == 0)
            {
                if (sscanf(line,"CLIENTCONTROLPORT = %s",value))
                {
                    m_clientControlPort = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("CLIENTCONTROLPORT LINE FAILED: %s | %s\n",
                            line,value);
                }
            }
            else if (memcmp(line,"MAXNUMUSERS",11*sizeof(char)) == 0)
            {
                if (sscanf(line,"MAXNUMUSERS = %s",value))
                {
                    m_maxNumUsers = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("MAXNUMUSERS LINE FAILED: %s | %s\n",line,value);
                }
            }
            else if (memcmp(line,"MAXNUMPEERS",11*sizeof(char)) == 0)
            {
                if (sscanf(line,"MAXNUMPEERS = %s",value))
                {
                    m_maxNumPeers = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("MAXNUMPEERS LINE FAILED: %s | %s\n",line,value);
                }
            }
            else if (memcmp(line,"USERTTL",7*sizeof(char)) == 0)
            {
                if (sscanf(line,"USERTTL = %s",value))
                {
                    m_userTTL = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("USERTTL LINE FAILED: %s | %s\n",line,value);
                }
            }
            else if (memcmp(line,"CLIENTCONNECTIONSPEED",
                            21*sizeof(char)) == 0)
            {
                if (sscanf(line,"CLIENTCONNECTIONSPEED = %s",value))
                {
                    m_clientConnectionSpeed = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("CLIENTCONNECTIONSPEED LINE FAILED: %s | "
                            "%s\n",line,value);
                }
            }
            else if (memcmp(line,"USERPINGDELAY",13*sizeof(char)) == 0)
            {
                if (sscanf(line,"USERPINGDELAY = %s",value))
                {
                    m_userPingDelay = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("USERPINGDELAY LINE FAILED: %s | %s\n",
                            line,value);
                }
            }
            else if (memcmp(line,"USERSTATUSDELAY",15*sizeof(char)) == 0)
            {
                if (sscanf(line,"USERSTATUSDELAY = %s",value))
                {
                    m_userStatusDelay = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("USERSTATUSDELAY LINE FAILED: %s | %s\n",
                            line,value);
                }
            }
            else if (memcmp(line,"MAXRESULTS",10*sizeof(char)) == 0)
            {
                if (sscanf(line,"MAXRESULTS = %s",value))
                {
                    m_maxResults = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("MAXRESULTS LINE FAILED: %s | %s\n",line,value);
                }
            }
            else if (memcmp(line,"SHOWFILEPATH",12*sizeof(char)) == 0)
            {
                if (sscanf(line,"SHOWFILEPATH = %s",value))
                {
                    m_showFilePath = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("SHOWFILEPATH LINE FAILED: %s | %s\n",
                            line,value);
                }
            }
            else if (memcmp(line,"CLIENTFIREWALLSTATUS",
                            20*sizeof(char)) == 0)
            {
                if (sscanf(line,"CLIENTFIREWALLSTATUS = %s",value))
                {
                    m_clientFirewallStatus = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("CLIENTFIREWALLSTATUS LINE FAILED: %s | "
                            "%s\n",line,value);
                }
            }
            else if (memcmp(line,"MINUSERSHAREDFILELIMIT",
                            22*sizeof(char)) == 0)
            {
                if (sscanf(line,"MINUSERSHAREDFILELIMIT = %s",value))
                {
                    m_minUserSharedFileLimit = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("MINUSERSHAREDFILELIMIT LINE FAILED: %s "
                            "| %s\n",line,value);
                }
            }
            else if (memcmp(line,"GOODBYEMESSAGEFILE",18*sizeof(char)) == 0)
            {
                if (sscanf(line,"GOODBYEMESSAGEFILE = %s",value))
                {
                    if (value)
                    {
                        str = neUtils::fileToStr(value);
                        if (str.length())
                        {
                            setGoodbyeMessageFile((char *)str.c_str());
                        }
                    }
                    ret++;
                }
                else
                {
                    eprintf("GOODBYEMESSAGEFILE LINE FAILED: %s | %s\n",
                            line,value);
                }
            }
            else if (memcmp(line,"GREETINGMESSAGEFILE",
                            19*sizeof(char)) == 0)
            {
                if (sscanf(line,"GREETINGMESSAGEFILE = %s",value))
                {
                    if (value)
                    {
                        str = neUtils::fileToStr(value);
                        if (str.length())
                        {
                            setGreetingMessageFile((char *)str.c_str());
                        }
                    }
                    ret++;
                }
                else
                {
                    eprintf("GREETINGMESSAGEFILE LINE FAILED: %s | %s\n",
                            line,value);
                }
            }
            else if (memcmp(line,"LOGINFAILEDMESSAGEFILE",
                            22*sizeof(char)) == 0)
            {
                if (sscanf(line,"LOGINFAILEDMESSAGEFILE = %s",value))
                {
                    if (value)
                    {
                        str = neUtils::fileToStr(value);
                        if (str.length())
                        {
                            setLoginFailedMessageFile((char *)str.c_str());
                        }
                    }
                    ret++;
                }
                else
                {
                    eprintf("LOGINFAILEDMESSAGEFILE LINE FAILED: "
                            "%s | %s\n",line,value);
                }
            }
            else if (memcmp(line,"INVALIDMESSAGETYPEFILE",
                            22*sizeof(char)) == 0)
            {
                if (sscanf(line,"INVALIDMESSAGETYPEFILE = %s",value))
                {
                    if (value)
                    {
                        str = neUtils::fileToStr(value);
                        if (str.length())
                        {
                            setInvalidMessageTypeFile(
                                (char *)str.c_str());
                        }
                    }
                    ret++;
                }
                else
                {
                    eprintf("INVALIDMESSAGETYPEFILE LINE FAILED: "
                            "%s | %s\n",line,value);
                }
            }
            else if (memcmp(line,"NUMTHREADEDHANDLERS",19*sizeof(char)) == 0)
            {
                if (sscanf(line,"NUMTHREADEDHANDLERS = %s",value))
                {
                    m_numThreadedHandlers = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("NUMTHREADEDHANDLERS LINE FAILED: %s | %s\n",
                            line,value);
                }
            }
            else if (memcmp(line,"NUMUSERMANAGERS",15*sizeof(char)) == 0)
            {
                if (sscanf(line,"NUMUSERMANAGERS = %s",value))
                {
                    m_numUserManagers = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("NUMUSERMANAGERS LINE FAILED: %s | %s\n",
                            line,value);
                }
            }
            else if (memcmp(line,"MAXNUMMESSAGESTOHANDLE",
                            22*sizeof(char)) == 0)
            {
                if (sscanf(line,"MAXNUMMESSAGESTOHANDLE = %s",value))
                {
                    m_maxMessagesToHandle = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("MAXNUMMESSAGESTOHANDLE LINE FAILED: %s "
                            "| %s\n",line,value);
                }
            }
            else if (memcmp(line,"USERFILEPAGEFILE",
                            16*sizeof(char)) == 0)
            {
                if (sscanf(line,"USERFILEPAGEFILE = %s",value))
                {
                    strcpy(m_userFilePageFile,value);
                    ret++;
                }
                else
                {
                    eprintf("USERFILEPAGEFILE LINE FAILED: %s "
                            "| %s\n",line,value);
                }
            }
            else if (memcmp(line,"NUMRESERVEDUSERFILES",
                            20*sizeof(char)) == 0)
            {
                if (sscanf(line,"NUMRESERVEDUSERFILES = %s",value))
                {
                    m_numReservedUserFiles = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("NUMRESERVEDUSERFILES LINE FAILED: %s "
                            "| %s\n",line,value);
                }
            }
            else if (memcmp(line,"USERMSGTHROTTLE",15*sizeof(char)) == 0)
            {
                if (sscanf(line,"USERMSGTHROTTLE = %s",value))
                {
                    m_userMsgThrottle = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("USERMSGTHROTTLE LINE FAILED: %s "
                            "| %s\n",line,value);
                }
            }
            else if (memcmp(line,"MAXNUMSENDS",11*sizeof(char)) == 0)
            {
                if (sscanf(line,"MAXNUMSENDS = %s",value))
                {
                    m_clientMaxNumSends = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("MAXNUMSENDS LINE FAILED: %s "
                            "| %s\n",line,value);
                }
            }
            else if (memcmp(line,"MAXNUMRECVS",11*sizeof(char)) == 0)
            {
                if (sscanf(line,"MAXNUMRECVS = %s",value))
                {
                    m_clientMaxNumRecvs = atoi(value);
                    ret++;
                }
                else
                {
                    eprintf("MAXNUMRECVS LINE FAILED: %s "
                            "| %s\n",line,value);
                }
            }

            /* reset buffers */
            memset(line,0,GENERIC_BUF_LEN*sizeof(char));
            memset(option,0,GENERIC_BUF_LEN*sizeof(char));
            memset(value,0,GENERIC_BUF_LEN*sizeof(char));
        }
        fclose(fd);
    }
    return ((ret > 1) ? 0 : 1);
}

int neConfig::getMaxNumUsers()
{
    return m_maxNumUsers;
}

int neConfig::getUserTTL()
{
    return m_userTTL;
}

int neConfig::getUserPingDelay()
{
    return m_userPingDelay;
}

int neConfig::getUserStatusDelay()
{
    return m_userStatusDelay;
}

int neConfig::getMaxResults()
{
    return m_maxResults;
}

int neConfig::getServerAcceptPort()
{
    return m_serverAcceptPort;
}

int neConfig::getClientControlPort()
{
    return m_clientControlPort;
}

int neConfig::getClientForwardingPort()
{
    return m_clientForwardingPort;
}

int neConfig::getClientFirewallStatus()
{
    return m_clientFirewallStatus;
}

int neConfig::getClientConnectionSpeed()
{
    return m_clientConnectionSpeed;
}

int neConfig::getMaxNumPeers()
{
    return m_maxNumPeers;
}

int neConfig::getShowFilePath()
{
    return m_showFilePath;
}

int neConfig::getClientMaxNumSends()
{
    return m_clientMaxNumSends;
}

int neConfig::getClientMaxNumRecvs()
{
    return m_clientMaxNumRecvs;
}

char *neConfig::getServerAddress()
{
    return m_serverAddress;
}

char *neConfig::getSavePath()
{
    return m_savePath;
}

char *neConfig::getServerLog()
{
    return m_serverLog;
}

int neConfig::getMinUserSharedFileLimit()
{
    return m_minUserSharedFileLimit;
}

int neConfig::getNumThreadedHandlers()
{
    return m_numThreadedHandlers;
}

int neConfig::getNumUserManagers()
{
    return m_numUserManagers;
}

int neConfig::getMaxMessagesToHandle()
{
    return m_maxMessagesToHandle;
}

int neConfig::getNumReservedUserFiles()
{
    return m_numReservedUserFiles;
}

char *neConfig::getGoodbyeMessageFile()
{
    return m_goodbyeMessageFile;
}

char *neConfig::getGreetingMessageFile()
{
    return m_greetingMessageFile;
}

char *neConfig::getLoginFailedMessageFile()
{
    return m_loginFailedMessageFile;
}

char *neConfig::getInvalidMessageTypeFile()
{
    return m_invalidMessageTypeFile;
}

char *neConfig::getUserFilePageFile()
{
    return m_userFilePageFile;
}

std::vector<std::string> *neConfig::getDirectories()
{
    return &m_directories;
}

void neConfig::emptyDirectories()
{
    m_directories.clear();
    assert(m_directories.empty());
}

int neConfig::writeConfigFile(char *configFile, int fieldType)
{
    int ret = 1;

    FILE *fd = 0;
    char line[GENERIC_BUF_LEN] = {0};
    static char *header =
        "# Generated by neConfig (a part of NEshare)\r\n"
        "# (C) 2001,2002 Neill Miller\r\n"
        "# Contact Author: neillm@thecodefactory.org\r\n#\r\n"
        "# Write out all user configurable settings.\r\n";

    if (!configFile || (fd = fopen(configFile,"w")) == 0)
    {
        return ret;
    }
    if (fwrite(header,sizeof(char),strlen(header),fd) == 0)
    {
        fclose(fd);
        return ret;
    }

    if (fieldType & NECFG_SERVER_FIELDS)
    {
        /* write out max num users */
        snprintf(line,GENERIC_BUF_LEN,
                 "MAXNUMUSERS = %d\r\n",m_maxNumUsers);
        fwrite(line,sizeof(char),strlen(line),fd);
      
        /* write out user TTL */
        snprintf(line,GENERIC_BUF_LEN,
                 "USERTTL = %d\r\n",m_userTTL);
        fwrite(line,sizeof(char),strlen(line),fd);
      
        /* write out user ping delay */
        snprintf(line,GENERIC_BUF_LEN,
                 "USERPINGDELAY = %d\r\n",m_userPingDelay);
        fwrite(line,sizeof(char),strlen(line),fd);
      
        /* write out user status delay */
        snprintf(line,GENERIC_BUF_LEN,
                 "USERSTATUSDELAY = %d\r\n",m_userStatusDelay);
        fwrite(line,sizeof(char),strlen(line),fd);
      
        /* write out max results */
        snprintf(line,GENERIC_BUF_LEN,
                 "MAXRESULTS = %d\r\n",m_maxResults);
        fwrite(line,sizeof(char),strlen(line),fd);
      
        /* write out min user shared file limit */
        snprintf(line,GENERIC_BUF_LEN,
                 "MINUSERSHAREDFILELIMIT = %d\r\n",m_showFilePath);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out server accept port */
        snprintf(line,GENERIC_BUF_LEN,
                 "SERVERACCEPTPORT = %d\r\n",m_serverAcceptPort);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out the number of threaded handlers to start */
        snprintf(line,GENERIC_BUF_LEN,
                 "NUMTHREADEDHANDLERS = %d\r\n",m_numThreadedHandlers);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out the number of user managers to start */
        snprintf(line,GENERIC_BUF_LEN,
                 "NUMUSERMANAGERS = %d\r\n",m_numUserManagers);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out the max number of messages to handle */
        snprintf(line,GENERIC_BUF_LEN,
                 "MAXNUMMESSAGESTOHANDLE = %d\r\n",
                 m_maxMessagesToHandle);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out the serverlog file field if non-null */
        if (m_serverLog)
        {
            snprintf(line,GENERIC_BUF_LEN,
                     "SERVERLOG = %s\r\n",m_serverLog);
            fwrite(line,sizeof(char),strlen(line),fd);
        }

        /* write out the goodbye message file */
        if (m_goodbyeMessageFile)
        {
            snprintf(line,GENERIC_BUF_LEN,
                     "# GOODBYEMESSAGEFILE = goodbye.txt\r\n");
            fwrite(line,sizeof(char),strlen(line),fd);
        }

        /* write out the greeting message file */
        if (m_greetingMessageFile)
        {
            snprintf(line,GENERIC_BUF_LEN,
                     "# GREETINGMESSAGEFILE = greeting.txt\r\n");
            fwrite(line,sizeof(char),strlen(line),fd);
        }

        /* write out the login failed message file */
        if (m_loginFailedMessageFile)
        {
            snprintf(line,GENERIC_BUF_LEN,
                     "# LOGINFAILEDMESSAGEFILE = loginfailed.txt\r\n");
            fwrite(line,sizeof(char),strlen(line),fd);
        }

        /* write out the invalid message type file */
        if (m_invalidMessageTypeFile)
        {
            snprintf(line,GENERIC_BUF_LEN,
                     "# INVALIDMESSAGETYPEFILE = invalidmessage.txt\r\n");
            fwrite(line,sizeof(char),strlen(line),fd);
        }

        /* write out the user file page file */
        if (m_userFilePageFile)
        {
            snprintf(line,GENERIC_BUF_LEN,
                     "USERFILEPAGEFILE = %s\r\n",m_userFilePageFile);
            fwrite(line,sizeof(char),strlen(line),fd);
        }

        /* write out the number of user file reservations */
        snprintf(line,GENERIC_BUF_LEN,
                 "NUMRESERVEDUSERFILES = %d\r\n",m_numReservedUserFiles);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out the user msg throttle value */
        snprintf(line,GENERIC_BUF_LEN,
                 "USERMSGTHROTTLE = %d\r\n",m_userMsgThrottle);
        fwrite(line,sizeof(char),strlen(line),fd);
    }

    if (fieldType & NECFG_CLIENT_FIELDS)
    {
        /* write out the server address */
        snprintf(line,GENERIC_BUF_LEN,"SERVER = %s\r\n",m_serverAddress);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out server accept port */
        snprintf(line,GENERIC_BUF_LEN,"SERVERACCEPTPORT = %d\r\n",
                 m_serverAcceptPort);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out max num peers */
        snprintf(line,GENERIC_BUF_LEN,"MAXNUMPEERS = %d\r\n",
                 m_maxNumPeers);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out client control port */
        snprintf(line,GENERIC_BUF_LEN,"CLIENTCONTROLPORT = %d\r\n",
                 m_clientControlPort);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out client forwarding port */
        snprintf(line,GENERIC_BUF_LEN,"CLIENTFORWARDINGPORT = %d\r\n",
                 m_clientForwardingPort);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out client firewall status */
        snprintf(line,GENERIC_BUF_LEN,"CLIENTFIREWALLSTATUS = %d\r\n",
                 m_clientFirewallStatus);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out client connection speed */
        snprintf(line,GENERIC_BUF_LEN,"CLIENTCONNECTIONSPEED = %d\r\n",
                 m_clientConnectionSpeed);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out client max num peers */
        snprintf(line,GENERIC_BUF_LEN,"MAXNUMPEERS = %d\r\n",
                 m_maxNumPeers);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out client show file path option */
        snprintf(line,GENERIC_BUF_LEN,"SHOWFILEPATH = %d\r\n",
                 m_showFilePath);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out all search directory entries */
        int numDirectories = m_directories.size();
        for(int i = 0; i < numDirectories; i++)
        {
            snprintf(line,GENERIC_BUF_LEN,"DIRECTORY = %s\r\n",
                     (m_directories[i]).c_str());
            fwrite(line,sizeof(char),strlen(line),fd);
        }

        /* write out the save path */
        snprintf(line,GENERIC_BUF_LEN,"SAVEPATH = %s\r\n",m_savePath);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out the max num sends value */
        snprintf(line,GENERIC_BUF_LEN,"MAXNUMSENDS = %d\r\n",
                 m_clientMaxNumSends);
        fwrite(line,sizeof(char),strlen(line),fd);

        /* write out the max num recvs value */
        snprintf(line,GENERIC_BUF_LEN,"MAXNUMRECVS = %d\r\n",
                 m_clientMaxNumRecvs);
        fwrite(line,sizeof(char),strlen(line),fd);
    }
    fclose(fd);

    ret = 0;
    return ret;
}

void neConfig::setMaxNumUsers(int maxNumUsers)
{
    m_maxNumUsers = maxNumUsers;
}

void neConfig::setUserTTL(int userTTL)
{
    m_userTTL = userTTL;
}

void neConfig::setUserPingDelay(int userPingDelay)
{
    m_userPingDelay = userPingDelay;
}

void neConfig::setUserStatusDelay(int userStatusDelay)
{
    m_userStatusDelay = userStatusDelay;
}

void neConfig::setMaxResults(int maxResults)
{
    m_maxResults = maxResults;
}

void neConfig::setServerAcceptPort(int serverAcceptPort)
{
    m_serverAcceptPort = serverAcceptPort;
}

void neConfig::setClientControlPort(int clientControlPort)
{
    m_clientControlPort = clientControlPort;
}

void neConfig::setClientForwardingPort(int clientForwardingPort)
{
    m_clientForwardingPort = clientForwardingPort;
}

void neConfig::setClientFirewallStatus(int clientFirewallStatus)
{
    m_clientFirewallStatus = clientFirewallStatus;
}

void neConfig::setServerAddress(char *serverAddr)
{
    if (serverAddr)
    {
        strncpy(m_serverAddress,serverAddr,NESERVER_MAX_NAME_LEN);
    }
}

void neConfig::setSavePath(char *savePath)
{
    if (savePath)
    {
        int len = MIN(strlen(savePath),(int)NE_MSG_MAX_DATA_LEN-1);

        if (len)
        {
            strncpy(m_savePath,savePath,len);
         
            /* make sure the path specified ends with a slash */
            if ((savePath[len-1] != '\\') &&
                (savePath[len-1] != '/'))
            {
#ifdef WIN32
                m_savePath[len] = '\\';
#else
                m_savePath[len] = '/';
#endif
                m_savePath[len+1] = '\0';
            }
        }
    }
}

void neConfig::setClientMaxNumSends(int maxNumSends)
{
    m_clientMaxNumSends = maxNumSends;
}

void neConfig::setClientMaxNumRecvs(int maxNumRecvs)
{
    m_clientMaxNumRecvs = maxNumRecvs;
}

void neConfig::setServerLog(char *serverlogfile)
{
    if (serverlogfile)
    {
        strncpy(m_serverLog,serverlogfile,NE_MSG_MAX_DATA_LEN);
    }
}

void neConfig::setClientConnectionSpeed(int connection)
{
    m_clientConnectionSpeed = connection;
}

void neConfig::setMaxNumPeers(int maxNumPeers)
{
    m_maxNumPeers = maxNumPeers;
}

void neConfig::setShowFilePath(int showFilePath)
{
    m_showFilePath = showFilePath;
}

void neConfig::setMinUserSharedFileLimit(int minUserSharedFileLimit)
{
    m_minUserSharedFileLimit = minUserSharedFileLimit;
}

void neConfig::setNumThreadedHandlers(int numThreadedHandlers)
{
    m_numThreadedHandlers = numThreadedHandlers;
}

void neConfig::setNumUserManagers(int numUserManagers)
{
    m_numUserManagers = numUserManagers;
}

void neConfig::setMaxMessagesToHandle(int maxMessagesToHandle)
{
    m_maxMessagesToHandle = maxMessagesToHandle;
}

void neConfig::setGoodbyeMessageFile(char *msg)
{
    if (msg)
    {
        if (m_goodbyeMessageFile)
        {
            free(m_goodbyeMessageFile);
            m_goodbyeMessageFile = (char *)0;
        }
        m_goodbyeMessageFile = strdup(msg);
    }
}

void neConfig::setGreetingMessageFile(char *msg)
{
    if (msg)
    {
        if (m_greetingMessageFile)
        {
            free(m_greetingMessageFile);
            m_greetingMessageFile = (char *)0;
        }
        m_greetingMessageFile = strdup(msg);
    }
}

void neConfig::setLoginFailedMessageFile(char *msg)
{
    if (msg)
    {
        if (m_loginFailedMessageFile)
        {
            free(m_loginFailedMessageFile);
            m_loginFailedMessageFile = (char *)0;
        }
        m_loginFailedMessageFile = strdup(msg);
    }
}

void neConfig::setInvalidMessageTypeFile(char *msg)
{
    if (msg)
    {
        if (m_invalidMessageTypeFile)
        {
            free(m_invalidMessageTypeFile);
            m_invalidMessageTypeFile = (char *)0;
        }
        m_invalidMessageTypeFile = strdup(msg);
    }
}

void neConfig::setUserFilePageFile(char *pagefile)
{
    if (pagefile)
    {
        memset(m_userFilePageFile,0,NE_MSG_MAX_DATA_LEN);
        strcpy(m_userFilePageFile,pagefile);
    }
}

void neConfig::setNumReservedUserFiles(int num)
{
    m_numReservedUserFiles = num;
}

void neConfig::setUserMsgThrottle(int num)
{
    m_userMsgThrottle = num;
}

void neConfig::addDirectory(char *directory)
{
    if (directory)
    {
        m_directories.push_back(std::string(directory));
    }
}
