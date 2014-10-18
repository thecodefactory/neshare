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

nemsgPeerLoginFailed::nemsgPeerLoginFailed(ncSocket *sock)
{
    m_sock = sock;
}

nemsgPeerLoginFailed::~nemsgPeerLoginFailed()
{
    m_sock = (ncSocket *)0;
}

int nemsgPeerLoginFailed::send(char *peerloginfailedmsg)
{
    int ret = 1;
    int peerloginfailedMsgLen =
        (peerloginfailedmsg ? MIN(strlen(peerloginfailedmsg),
                                  NE_MSG_MAX_DATA_LEN-1) : 0);

    if (peerloginfailedMsgLen)
    {
        NEMSGPEERLOGINFAILED_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGPEERLOGINFAILED_HEADER));
        msgHeader.msgType = htonl(NE_MSG_PEER_LOGIN_FAILED);
        msgHeader.msgLength = htonl(peerloginfailedMsgLen);

        if ((m_sock->writeData((void *)&msgHeader,
                               sizeof(NEMSGPEERLOGINFAILED_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)peerloginfailedmsg,
                               peerloginfailedMsgLen) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgPeerLoginFailed::recv(char *peerloginfailedmsg, int maxlen)
{
    int ret = 1;
    unsigned long peerloginfailedMsgLen = MIN((unsigned long)maxlen,
                                              NE_MSG_MAX_DATA_LEN-1);

    NEMSGPEERLOGINFAILED_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGPEERLOGINFAILED_HEADER));

    if (peerloginfailedmsg &&
        m_sock->readData((void *)&msgHeader,sizeof(msgHeader)) == NC_OK)
    {
        if (ntohl(msgHeader.msgType) == NE_MSG_PEER_LOGIN_FAILED)
        {
            peerloginfailedMsgLen = MIN(peerloginfailedMsgLen,
                                        ntohl(msgHeader.msgLength));

            if (m_sock->readData((void *)peerloginfailedmsg,
                                 peerloginfailedMsgLen) == NC_OK)
            {
                peerloginfailedmsg[peerloginfailedMsgLen] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}
