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

nemsgDisconnectAck::nemsgDisconnectAck(ncSocket *sock) : nemsgCommon(sock)
{
}

nemsgDisconnectAck::~nemsgDisconnectAck()
{
}

int nemsgDisconnectAck::send(char *disconnectackmsg)
{
    int ret = 1;
    int disconnectackMsgLen =
        (disconnectackmsg ?
         MIN(strlen(disconnectackmsg),NE_MSG_MAX_DATA_LEN-1) : 0);

    if (disconnectackMsgLen)
    {
        NEMSGDISCONNECTACK_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGDISCONNECTACK_HEADER));
        msgHeader.msgType = htonl(NE_MSG_DISCONNECT_ACK);
        msgHeader.msgLength = htonl(disconnectackMsgLen);

        if ((m_sock->writeData((void *)&msgHeader,
                               sizeof(NEMSGDISCONNECTACK_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)disconnectackmsg,
                               disconnectackMsgLen) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgDisconnectAck::recv()
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long msgLength = 0;

    if (m_sock->readData((void *)ptr,2*sizeof(unsigned long)) == NC_OK)
    {
        ptr += sizeof(unsigned long);
        msgLength = MIN(NE_MSG_MAX_RAW_DATA_LEN-
                        ((2*sizeof(unsigned long))-1),
                        ntohl(*((unsigned long *)ptr)));

        ptr += sizeof(unsigned long);
        if (m_sock->readData((void *)ptr,msgLength) == NC_OK)
        {
            m_buf[msgLength+(2*sizeof(unsigned long))] = '\0';
            ret = 0;
        }
    }
    return ret;
}

int nemsgDisconnectAck::recv(char *disconnectackmsg, int maxlen)
{
    int ret = 1;
    unsigned long disconnectackMsgLen = MIN((unsigned long)maxlen,
                                            NE_MSG_MAX_DATA_LEN-1);
    NEMSGDISCONNECTACK_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGDISCONNECTACK_HEADER));

    if (disconnectackmsg &&
        m_sock->readData((void *)&msgHeader,sizeof(msgHeader)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        msgHeader.msgLength = MIN(disconnectackMsgLen,
                                  ntohl(msgHeader.msgLength));

        if (msgHeader.msgType == NE_MSG_DISCONNECT_ACK)
        {
            if (m_sock->readData((void *)disconnectackmsg,
                                 msgHeader.msgLength) == NC_OK)
            {
                disconnectackmsg[msgHeader.msgLength] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}

int nemsgDisconnectAck::format(char *disconnectackmsg, int maxlen)
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long tokenType = ntohl(*((unsigned long *)ptr));
    unsigned long msgLength = MIN((unsigned long)maxlen,
                                  NE_MSG_MAX_RAW_DATA_LEN-1);

    if (disconnectackmsg && (tokenType == NE_MSG_DISCONNECT_ACK))
    {
        ptr += sizeof(unsigned long);
        msgLength = MIN(msgLength,ntohl(*((unsigned long *)ptr)));

        ptr += sizeof(unsigned long);
        memcpy(disconnectackmsg,ptr,msgLength);
        disconnectackmsg[msgLength] = '\0';

        ret = 0;
    }
    return ret;
}
