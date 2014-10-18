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

nemsgLogin::nemsgLogin(ncSocket *sock) : nemsgCommon(sock)
{
}

nemsgLogin::~nemsgLogin()
{
}

int nemsgLogin::send(unsigned long connection, 
                     unsigned long firewallStatus,
                     unsigned long controlPort)
{
    int ret = 1;
    NEMSGLOGIN_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGLOGIN_HEADER));
    msgHeader.msgType = htonl(NE_MSG_LOGIN);
    msgHeader.msgConnection = htonl(connection);
    msgHeader.msgFirewallStatus = htonl(firewallStatus);
    msgHeader.msgControlPort = htonl(controlPort);

    if (m_sock->writeData((void *)&msgHeader,
                          sizeof(NEMSGLOGIN_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgLogin::recv()
{
    int ret = 1;

    if (m_sock->readData((void *)m_buf,sizeof(NEMSGLOGIN_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgLogin::recv(unsigned long *connection, 
                     unsigned long *firewallStatus,
                     unsigned long *controlPort)
{
    int ret = 1;
    NEMSGLOGIN_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGLOGIN_HEADER));

    if (connection && firewallStatus && controlPort &&
        m_sock->readData((void *)&msgHeader,sizeof(msgHeader)) == NC_OK)
    {
        if (ntohl(msgHeader.msgType) == NE_MSG_LOGIN)
        {
            *connection = ntohl(msgHeader.msgConnection);
            *firewallStatus = ntohl(msgHeader.msgFirewallStatus);
            *controlPort = ntohl(msgHeader.msgControlPort);
            ret = 0;
        }
    }
    return ret;
}

int nemsgLogin::format(unsigned long *connection, 
                       unsigned long *firewallStatus,
                       unsigned long *controlPort)
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long tokenType = ntohl(*((unsigned long *)ptr));

    if (connection && firewallStatus && controlPort &&
        (tokenType == NE_MSG_LOGIN))
    {
        ptr += sizeof(unsigned long);
        *connection = ntohl(*((unsigned long *)ptr));

        ptr += sizeof(unsigned long);
        *firewallStatus = ntohl(*((unsigned long *)ptr));

        ptr += sizeof(unsigned long);
        *controlPort = ntohl(*((unsigned long *)ptr));

        ret = 0;
    }
    return ret;
}
