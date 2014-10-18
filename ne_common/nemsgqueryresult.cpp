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

nemsgQueryResult::nemsgQueryResult(ncSocket *sock)
{
    m_sock = sock;
}

nemsgQueryResult::~nemsgQueryResult()
{
    m_sock = (ncSocket *)0;
}

int nemsgQueryResult::send(unsigned long ipAddr, 
                           unsigned long connection, 
                           unsigned long firewallStatus,
                           unsigned long filesize,
                           unsigned long controlPort,
                           unsigned long id,
                           char *result)
{
    int ret = 1;
    int queryresultLen =
        (result ? MIN(strlen(result),NE_MSG_MAX_DATA_LEN-1) : 0);

    if (queryresultLen)
    {
        NEMSGQUERYRESULT_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGQUERYRESULT_HEADER));

        msgHeader.msgType = htonl(NE_MSG_QUERY_RESULT);
#if (__BYTE_ORDER == __BIG_ENDIAN)
        msgHeader.msgIpAddr = htonl(ipAddr);
#else
        msgHeader.msgIpAddr = ipAddr;
#endif
        msgHeader.msgConnection = htonl(connection);
        msgHeader.msgFirewallStatus = htonl(firewallStatus);
        msgHeader.msgFilesize = htonl(filesize);
        msgHeader.msgControlPort = htonl(controlPort);
        msgHeader.msgID = htonl(id);
        msgHeader.msgResultLen = htonl(queryresultLen);

        if ((m_sock->writeData((void *)&msgHeader,
                               sizeof(NEMSGQUERYRESULT_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)result,queryresultLen) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}


int nemsgQueryResult::recv(unsigned long *ipAddr,
                           unsigned long *connection,
                           unsigned long *firewallStatus,
                           unsigned long *filesize,
                           unsigned long *controlPort,
                           unsigned long *id,
                           char *result, 
                           int maxresultlen)
{
    int ret = 1;
    unsigned long queryresultLen = MIN((unsigned long)maxresultlen,
                                       NE_MSG_MAX_DATA_LEN-1);

    NEMSGQUERYRESULT_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGQUERYRESULT_HEADER));

    /* read the entire header in at once and make sure it's sane */
    if (ipAddr && connection && firewallStatus &&
        filesize && controlPort && id &&
        m_sock->readData((void *)&msgHeader,
                         sizeof(NEMSGQUERYRESULT_HEADER)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
#if (__BYTE_ORDER == __BIG_ENDIAN)
        msgHeader.msgIpAddr = ntohl(msgHeader.msgIpAddr);
#else
        msgHeader.msgIpAddr = msgHeader.msgIpAddr;
#endif
        msgHeader.msgConnection = ntohl(msgHeader.msgConnection);
        msgHeader.msgFirewallStatus = ntohl(msgHeader.msgFirewallStatus);
        msgHeader.msgFilesize = ntohl(msgHeader.msgFilesize);
        msgHeader.msgControlPort = ntohl(msgHeader.msgControlPort);
        msgHeader.msgID = ntohl(msgHeader.msgID);
        msgHeader.msgResultLen = ntohl(msgHeader.msgResultLen);

        if (msgHeader.msgType == NE_MSG_QUERY_RESULT)
        {
            *ipAddr = msgHeader.msgIpAddr;
            *connection = msgHeader.msgConnection;
            *firewallStatus = msgHeader.msgFirewallStatus;
            *filesize = msgHeader.msgFilesize;
            *controlPort = msgHeader.msgControlPort;
            *id = msgHeader.msgID;
            queryresultLen = MIN(msgHeader.msgResultLen,queryresultLen);

            if (m_sock->readData((void *)result,queryresultLen) == NC_OK)
            {
                result[queryresultLen] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}
