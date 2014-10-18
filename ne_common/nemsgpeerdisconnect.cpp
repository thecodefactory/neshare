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

nemsgPeerDisconnect::nemsgPeerDisconnect(ncSocket *sock)
{
    m_sock = sock;
}

nemsgPeerDisconnect::~nemsgPeerDisconnect()
{
    m_sock = (ncSocket *)0;
}

int nemsgPeerDisconnect::send()
{
    int ret = 1;
    NEMSGPEERDISCONNECT_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGPEERDISCONNECT_HEADER));
    msgHeader.msgType = htonl(NE_MSG_PEER_DISCONNECT);

    if (m_sock->writeData((void *)&msgHeader,
                          sizeof(NEMSGPEERDISCONNECT_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgPeerDisconnect::recv()
{
    int ret = 1;
    unsigned long peerdisconnectMsg = 0;

    if (m_sock->readData((void *)&peerdisconnectMsg,
                         sizeof(unsigned long)) == NC_OK)
    {
        peerdisconnectMsg = ntohl(peerdisconnectMsg);
        if (peerdisconnectMsg == NE_MSG_PEER_DISCONNECT)
        {
            ret = 0;
        }
    }
    return ret;
}
