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

nemsgFileDataCancel::nemsgFileDataCancel(ncSocket *sock)
    : nemsgCommon(sock)
{
}

nemsgFileDataCancel::~nemsgFileDataCancel()
{
}

int nemsgFileDataCancel::send(unsigned long fileID)
{
    int ret = 1;
    NEMSGFILEDATACANCEL_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGFILEDATACANCEL_HEADER));
    msgHeader.msgType = htonl(NE_MSG_FILE_DATA_CANCEL);
    msgHeader.msgFileID = htonl(fileID);

    if (m_sock->writeData((void *)&msgHeader,
                          sizeof(NEMSGFILEDATACANCEL_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgFileDataCancel::recv()
{
    int ret = 1;
    char *ptr = m_buf;

    if (m_sock->readData((void *)ptr,2*sizeof(unsigned long)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgFileDataCancel::recv(unsigned long *fileID)
{
    int ret = 1;
    NEMSGFILEDATACANCEL_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGFILEDATACANCEL_HEADER));

    if (fileID &&
        m_sock->readData((void *)&msgHeader,
                         sizeof(NEMSGFILEDATACANCEL_HEADER)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        if (msgHeader.msgType == NE_MSG_FILE_DATA_CANCEL)
        {
            msgHeader.msgFileID = ntohl(msgHeader.msgFileID);
            *fileID = msgHeader.msgFileID;
            ret = 0;
        }
    }
    return ret;
}

int nemsgFileDataCancel::format(unsigned long *fileID)
{
    int ret = 1;
    char *ptr = m_buf;

    if (fileID &&
        ntohl(*((unsigned long *)ptr)) == NE_MSG_FILE_DATA_CANCEL)
    {
        ptr += sizeof(unsigned long);

        *fileID = *((unsigned long *)ptr);
        ret = 0;
    }
    return ret;
}
