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

nemsgPushRequestAck::nemsgPushRequestAck(ncSocket *sock)
{
    m_sock = sock;
}

nemsgPushRequestAck::~nemsgPushRequestAck()
{
    m_sock = (ncSocket *)0;
}

int nemsgPushRequestAck::send(char *filename)
{
    int ret = 1;
    int filenameLen =
        (filename ? MIN(strlen(filename),
                        NE_MSG_MAX_DATA_LEN) : 0);

    if (filenameLen)
    {
        NEMSGPUSHREQUESTACK_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGPUSHREQUESTACK_HEADER));
        msgHeader.msgType = htonl(NE_MSG_PUSH_REQUEST_ACK);
        msgHeader.msgLength = htonl(filenameLen);
   
        if ((m_sock->writeData(
                 (void *)&msgHeader,
                 sizeof(NEMSGPUSHREQUESTACK_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)filename,filenameLen) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgPushRequestAck::recv(char *filename,
                              unsigned long maxlen)
{
    int ret = 1;
    unsigned long filenameLen = MIN((unsigned long)maxlen,
                                    NE_MSG_MAX_DATA_LEN-1);

    NEMSGPUSHREQUESTACK_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGPUSHREQUESTACK_HEADER));

    if (filename &&
        m_sock->readData((void *)&msgHeader,sizeof(msgHeader)) == NC_OK)
    {
        if (ntohl(msgHeader.msgType) == NE_MSG_PUSH_REQUEST_ACK)
        {
            filenameLen = MIN(filenameLen,ntohl(msgHeader.msgLength));

            if (m_sock->readData((void *)filename,filenameLen) == NC_OK)
            {
                filename[filenameLen] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}
