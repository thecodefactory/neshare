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

nemsgPushRequest::nemsgPushRequest(ncSocket *sock) : nemsgCommon(sock)
{
}

nemsgPushRequest::~nemsgPushRequest()
{
}

int nemsgPushRequest::send(unsigned long ipAddr,
                           unsigned long ctrlPort,
                           unsigned long id,
                           char *filename)
{
    int ret = 1;
    int filenameLen =
        (filename ? MIN(strlen(filename),NE_MSG_MAX_DATA_LEN) : 0);

    if (filenameLen)
    {
        NEMSGPUSHREQUEST_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGPUSHREQUEST_HEADER));

        msgHeader.msgType = htonl(NE_MSG_PUSH_REQUEST);
#if (__BYTE_ORDER == __BIG_ENDIAN)
        msgHeader.msgIpAddr = htonl(ipAddr);
#else
        msgHeader.msgIpAddr = ipAddr;
#endif
        msgHeader.msgCtrlPort = htonl(ctrlPort);
        msgHeader.msgID = htonl(id);
        msgHeader.msgLength = htonl(filenameLen);

        if ((m_sock->writeData((void *)&msgHeader,
                               sizeof(NEMSGPUSHREQUEST_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)filename,filenameLen) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgPushRequest::recv()
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long filenameLen = 0;

    if (m_sock->readData((void *)ptr,4*sizeof(unsigned long)) == NC_OK)
    {
        ptr += 3*sizeof(unsigned long);
        filenameLen = MIN(NE_MSG_MAX_RAW_DATA_LEN-
                          ((4*sizeof(unsigned long))-1),
                          ntohl(*((unsigned long *)ptr)));

        ptr += sizeof(unsigned long);
        if (m_sock->readData((void *)ptr,filenameLen) == NC_OK)
        {
            m_buf[filenameLen+(4*sizeof(unsigned long))] = '\0';
            ret = 0;
        }
    }
    return ret;
}

int nemsgPushRequest::recv(unsigned long *ipAddr,
                           unsigned long *ctrlPort,
                           unsigned long *id,
                           char *filename,
                           unsigned long maxlen)
{
    int ret = 1;
    unsigned long filenameLen = MIN((unsigned long)maxlen,
                                    NE_MSG_MAX_DATA_LEN-1);
    NEMSGPUSHREQUEST_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGPUSHREQUEST_HEADER));

    if (ipAddr && ctrlPort && id && filename &&
        m_sock->readData((void *)&msgHeader,
                         sizeof(NEMSGPUSHREQUEST_HEADER)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        if (msgHeader.msgType == NE_MSG_PUSH_REQUEST)
        {
#if (__BYTE_ORDER == __BIG_ENDIAN)
            msgHeader.msgIpAddr = ntohl(msgHeader.msgIpAddr);
#endif
            *ipAddr = msgHeader.msgIpAddr;
            *ctrlPort = ntohl(msgHeader.msgCtrlPort);
            *id = ntohl(msgHeader.msgID);
            filenameLen = MIN(filenameLen, ntohl(msgHeader.msgLength));

            if (m_sock->readData((void *)filename,filenameLen) == NC_OK)
            {
                filename[filenameLen] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}

int nemsgPushRequest::format(unsigned long *ipAddr,
                             unsigned long *ctrlPort,
                             unsigned long *id,
                             char *filename,
                             unsigned long maxlen)
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long tokenType = ntohl(*((unsigned long *)ptr));
    unsigned long filenameLen = MIN((unsigned long)maxlen,
                                    NE_MSG_MAX_RAW_DATA_LEN-1);

    if (ipAddr && ctrlPort && id && filename &&
        (tokenType == NE_MSG_PUSH_REQUEST))
    {
        ptr += sizeof(unsigned long);
        *ipAddr = *((unsigned long *)ptr);

        ptr += sizeof(unsigned long);
        *ctrlPort = *((unsigned long *)ptr);

        ptr += sizeof(unsigned long);
        *id = *((unsigned long *)ptr);

        ptr += sizeof(unsigned long);
        filenameLen = MIN(filenameLen,ntohl(*((unsigned long *)ptr)));

        ptr += sizeof(unsigned long);
        memcpy(filename,ptr,filenameLen);
        filename[filenameLen] = '\0';

        ret = 0;
    }
    return ret;
}
