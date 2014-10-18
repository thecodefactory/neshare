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

nemsgFileDataSendEnd::nemsgFileDataSendEnd(ncSocket *sock)
{
    m_sock = sock;
}

nemsgFileDataSendEnd::~nemsgFileDataSendEnd()
{
    m_sock = (ncSocket *)0;
}

int nemsgFileDataSendEnd::send(unsigned long fileID)
{
    int ret = 1;
    NEMSGFILEDATASENDEND_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGFILEDATASENDEND_HEADER));
    msgHeader.msgType = htonl(NE_MSG_FILE_DATA_SEND_END);
    msgHeader.msgFileID = htonl(fileID);

    if (m_sock->writeData((void *)&msgHeader,
                          sizeof(NEMSGFILEDATASENDEND_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgFileDataSendEnd::recv(unsigned long *fileID)
{
    int ret = 1;
    NEMSGFILEDATASENDEND_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGFILEDATASENDEND_HEADER));   

    if (fileID &&
        m_sock->readData((void *)&msgHeader,
                         sizeof(NEMSGFILEDATASENDEND_HEADER)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        if (msgHeader.msgType == NE_MSG_FILE_DATA_SEND_END)
        {
            msgHeader.msgFileID = ntohl(msgHeader.msgFileID);
            *fileID = msgHeader.msgFileID;
            ret = 0;
        }
    }
    return ret;
}
