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

nemsgPong::nemsgPong(ncSocket *sock) : nemsgCommon(sock)
{
}

nemsgPong::~nemsgPong()
{
}

int nemsgPong::send()
{
    int ret = 1;
    NEMSGPONG_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGPONG_HEADER));
    msgHeader.msgType = htonl(NE_MSG_PONG);

    if (m_sock->writeData((void *)&msgHeader,
                          sizeof(NEMSGPONG_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgPong::recv()
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long pongMsg = 0;

    if (m_sock->readData((void *)&pongMsg,
                         sizeof(unsigned long)) == NC_OK)
    {
        *((unsigned long *)ptr) = pongMsg;
        pongMsg = ntohl(pongMsg);

        if (pongMsg == NE_MSG_PONG)
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgPong::format()
{
    int ret = 1;
    char *ptr = m_buf;

    if (ntohl(*((unsigned long *)ptr)) == NE_MSG_PONG)
    {
        ret = 0;
    }
    return ret;
}
