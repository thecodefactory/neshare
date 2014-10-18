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

nemsgFileRequest::nemsgFileRequest(ncSocket *sock)
{
    m_sock = sock;
}

nemsgFileRequest::~nemsgFileRequest()
{
    m_sock = (ncSocket *)0;
}

int nemsgFileRequest::send(char *filerequest)
{
    int ret = 1;
    int fileRequestLen =
        (filerequest ? MIN(strlen(filerequest),NE_MSG_MAX_DATA_LEN) : 0);

    if (fileRequestLen)
    {
        NEMSGFILEREQUEST_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGFILEREQUEST_HEADER));
        msgHeader.msgType = htonl(NE_MSG_FILE_REQUEST);
        msgHeader.msgLength = htonl(fileRequestLen);
   
        if ((m_sock->writeData((void *)&msgHeader,
                               sizeof(NEMSGFILEREQUEST_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)filerequest,fileRequestLen) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgFileRequest::recv(char *filerequest, unsigned long maxlen)
{
    int ret = 1;
    unsigned long filerequestLen = MIN((unsigned long)maxlen,
                                       NE_MSG_MAX_DATA_LEN-1);
    NEMSGFILEREQUEST_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGFILEREQUEST_HEADER));

    if (filerequest &&
        m_sock->readData((void *)&msgHeader,sizeof(msgHeader)) == NC_OK)
    {
        if (ntohl(msgHeader.msgType) == NE_MSG_FILE_REQUEST)
        {
            filerequestLen = MIN(filerequestLen,
                                 ntohl(msgHeader.msgLength));

            if (m_sock->readData((void *)filerequest,
                                 filerequestLen) == NC_OK)
            {
                filerequest[filerequestLen] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}
