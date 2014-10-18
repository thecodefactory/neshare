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

nemsgEntry::nemsgEntry(ncSocket *sock) : nemsgCommon(sock)
{
}

nemsgEntry::~nemsgEntry()
{
}

int nemsgEntry::send(char *filename, unsigned long filesize)
{
    int ret = 1;
    int entryMsgLen =
        (filename ? MIN(strlen(filename),NE_MSG_MAX_DATA_LEN-1) : 0);

    if (entryMsgLen)
    {
        NEMSGENTRY_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGENTRY_HEADER));
        msgHeader.msgType = htonl(NE_MSG_ENTRY);
        msgHeader.msgLength = htonl(entryMsgLen);
        msgHeader.filesize = htonl(filesize);

        if ((m_sock->writeData((void *)&msgHeader,
                               sizeof(NEMSGENTRY_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)filename,entryMsgLen) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgEntry::recv()
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long msgLength = 0;

    if (m_sock->readData((void *)ptr,3*sizeof(unsigned long)) == NC_OK)
    {
        ptr += sizeof(unsigned long);
        msgLength = MIN(NE_MSG_MAX_RAW_DATA_LEN-
                        ((3*sizeof(unsigned long))-1),
                        ntohl(*((unsigned long *)ptr)));

        ptr += 2*sizeof(unsigned long);
        if (m_sock->readData((void *)ptr,msgLength) == NC_OK)
        {
            m_buf[msgLength+(3*sizeof(unsigned long))] = '\0';
            ret = 0;
        }
    }
    return ret;
}

int nemsgEntry::recv(char *filename, int maxlen, unsigned long *filesize)
{
    int ret = 1;
    unsigned long entryLen = MIN((unsigned long)maxlen,
                                 NE_MSG_MAX_DATA_LEN-1);

    NEMSGENTRY_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGENTRY_HEADER));

    if (filename && filesize &&
        m_sock->readData((void *)&msgHeader,
                         sizeof(NEMSGENTRY_HEADER)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        msgHeader.msgLength = MIN(entryLen,ntohl(msgHeader.msgLength));

        if (msgHeader.msgType == NE_MSG_ENTRY)
        {
            if (m_sock->readData((void *)filename,
                                 msgHeader.msgLength) == NC_OK)
            {
                *filesize = ntohl(msgHeader.filesize);
                filename[msgHeader.msgLength] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}

int nemsgEntry::format(char *filename,
                       int maxlen,
                       unsigned long *filesize)
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long tokenType = ntohl(*((unsigned long *)ptr));
    unsigned long msgLength = MIN((unsigned long)maxlen,
                                  NE_MSG_MAX_RAW_DATA_LEN-1);

    if (filename && filesize && (tokenType == NE_MSG_ENTRY))
    {
        ptr += sizeof(unsigned long);
        msgLength = MIN(msgLength,ntohl(*((unsigned long *)ptr)));

        ptr += sizeof(unsigned long);
        *filesize = ntohl(*((unsigned long *)ptr));

        ptr += sizeof(unsigned long);
        memcpy(filename,ptr,msgLength);
        filename[msgLength] = '\0';

        ret = 0;
    }
    return ret;
}
