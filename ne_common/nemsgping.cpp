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

nemsgPing::nemsgPing(ncSocket *sock) : nemsgCommon(sock)
{
}

nemsgPing::~nemsgPing()
{
}

int nemsgPing::send()
{
    int ret = 1;
    NEMSGPING_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGPING_HEADER));
    msgHeader.msgType = htonl(NE_MSG_PING);

    if (m_sock->writeData((void *)&msgHeader,
                          sizeof(NEMSGPING_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgPing::recv()
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long pingMsg = 0;

    if (m_sock->readData((void *)&pingMsg,
                         sizeof(unsigned long)) == NC_OK)
    {
        *((unsigned long *)ptr) = pingMsg;
        pingMsg = ntohl(pingMsg);

        if (pingMsg == NE_MSG_PING)
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgPing::format()
{
    int ret = 1;
    char *ptr = m_buf;

    if (ntohl(*((unsigned long *)ptr)) == NE_MSG_PING)
    {
        ret = 0;
    }
    return ret;
}
