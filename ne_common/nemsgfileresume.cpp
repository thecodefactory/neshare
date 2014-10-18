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

nemsgFileResume::nemsgFileResume(ncSocket *sock)
{
    m_sock = sock;
}

nemsgFileResume::~nemsgFileResume()
{
    m_sock = (ncSocket *)0;
}

int nemsgFileResume::send(char *filename, unsigned long position)
{
    int ret = 1;
    int filenameLen =
        (filename ? MIN(strlen(filename),NE_MSG_MAX_DATA_LEN) : 0);

    if (filenameLen)
    {
        NEMSGFILERESUME_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGFILERESUME_HEADER));
        msgHeader.msgType = htonl(NE_MSG_FILE_RESUME);
        msgHeader.msgPosition = htonl(position);
        msgHeader.msgLength = htonl(filenameLen);
   
        if ((m_sock->writeData((void *)&msgHeader,
                               sizeof(NEMSGFILERESUME_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)filename,filenameLen) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgFileResume::recv(unsigned long *position,
                          char *filename,
                          unsigned long maxlen)
{
    int ret = 1;
    unsigned long filenameLen = MIN((unsigned long)maxlen,
                                    NE_MSG_MAX_DATA_LEN-1);
    NEMSGFILERESUME_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGFILERESUME_HEADER));

    if (position && filename &&
        m_sock->readData((void *)&msgHeader,sizeof(msgHeader)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        msgHeader.msgPosition = ntohl(msgHeader.msgPosition);
        msgHeader.msgLength = ntohl(msgHeader.msgLength);

        if (msgHeader.msgType == NE_MSG_FILE_RESUME)
        {
            filenameLen = MIN(msgHeader.msgLength, filenameLen);
         
            if (m_sock->readData((void *)filename,filenameLen) == NC_OK)
            {
                *position = msgHeader.msgPosition;
                filename[filenameLen] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}
