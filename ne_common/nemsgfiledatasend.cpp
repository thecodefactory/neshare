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

nemsgFileDataSend::nemsgFileDataSend(ncSocket *sock)
{
    m_sock = sock;
}

nemsgFileDataSend::~nemsgFileDataSend()
{
    m_sock = (ncSocket *)0;
}

int nemsgFileDataSend::send(unsigned long fileID, 
                            unsigned long dataSize, 
                            void *data)
{
    int ret = 1;

    if (data && dataSize)
    {
        NEMSGFILEDATASEND_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGFILEDATASEND_HEADER));
        msgHeader.msgType = htonl(NE_MSG_FILE_DATA_SEND);
        msgHeader.msgFileID = htonl(fileID);
        msgHeader.msgLength = htonl(dataSize);

        if ((m_sock->writeData((void *)&msgHeader,
                               sizeof(NEMSGFILEDATASEND_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)data,dataSize) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgFileDataSend::recv(unsigned long *fileID, 
                            unsigned long *dataSize,
                            void *data, int maxlen)
{
    int ret = 1;
    NEMSGFILEDATASEND_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGFILEDATASEND_HEADER));

    if (fileID && dataSize && data &&
        m_sock->readData((void *)&msgHeader,
                         sizeof(NEMSGFILEDATASEND_HEADER)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        if (msgHeader.msgType != NE_MSG_FILE_DATA_SEND)
        {
            return ret;
        }
        msgHeader.msgFileID = ntohl(msgHeader.msgFileID);
        msgHeader.msgLength = ntohl(msgHeader.msgLength);

        unsigned long readLen = MIN(msgHeader.msgLength,(unsigned long)maxlen);
        if (m_sock->readData((void *)data,readLen) == NC_OK)
        {
            *fileID = msgHeader.msgFileID;
            *dataSize = msgHeader.msgLength;
            ret = 0;
        }
    }
    return ret;
}

void nemsgFileDataSend::setSocket(ncSocket *sock)
{
    m_sock = sock;
}

unsigned long nemsgFileDataSend::getMessageType()
{
    unsigned long msgType = 0;
    m_sock->readData((void *)&msgType,sizeof(unsigned long));
    m_msgType = ntohl(msgType);
    return m_msgType;
}

unsigned long nemsgFileDataSend::getFileID()
{
    unsigned long fileID = 0;
    m_sock->readData((void *)&fileID,sizeof(unsigned long));
    m_fileID = ntohl(fileID);
    return m_fileID;
}

unsigned long nemsgFileDataSend::getDataLength()
{
    unsigned long dataLength = 0;
    m_sock->readData((void *)&dataLength,sizeof(unsigned long));
    m_dataLength = ntohl(dataLength);
    return m_dataLength;
}

int nemsgFileDataSend::readData(void *buffer, unsigned long maxlen)
{
    int ret = 1;
    unsigned long readLen = MIN(m_dataLength,maxlen);

    assert(buffer);

    if (m_sock->readData(buffer,readLen) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}
