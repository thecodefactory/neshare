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

nemsgFileResumeAck::nemsgFileResumeAck(ncSocket *sock)
{
    m_sock = sock;
}

nemsgFileResumeAck::~nemsgFileResumeAck()
{
    m_sock = (ncSocket *)0;
}

int nemsgFileResumeAck::send(unsigned long fileSendID, 
                             unsigned long maxBlockSize,
                             unsigned long filesize,
                             unsigned char md5checksum[16],
                             char *filename)
{
    int ret = 1;
    int filenameLen =
        (filename ? MIN(strlen(filename),NE_MSG_MAX_DATA_LEN) : 0);

    if (filenameLen)
    {
        NEMSGFILERESUMEACK_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGFILERESUMEACK_HEADER));
        msgHeader.msgType = htonl(NE_MSG_FILE_RESUME_ACK);
        msgHeader.fileSendID = htonl(fileSendID);
        msgHeader.maxBlockSize = htonl(maxBlockSize);
        msgHeader.filesize = htonl(filesize);
        memcpy(msgHeader.md5checksum,md5checksum,16*sizeof(unsigned char));
        msgHeader.msgFilenameLen = htonl((unsigned long)filenameLen);
      
        if (m_sock->writeData((void *)&msgHeader,
                              sizeof(NEMSGFILERESUMEACK_HEADER)) == NC_OK)
         
        {
            if (m_sock->writeData((void *)filename,filenameLen) == NC_OK)
            {
                ret = 0;
            }
        }
    }
    return ret;
}

int nemsgFileResumeAck::recv(unsigned long *fileSendID, 
                             unsigned long *maxBlockSize,
                             unsigned long *filesize,
                             unsigned char md5checksum[16],
                             char *filename,
                             unsigned long maxlen)
{
    int ret = 1;
    unsigned long filenameLen = MIN(maxlen,NE_MSG_MAX_DATA_LEN-1);
    NEMSGFILERESUMEACK_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGFILERESUMEACK_HEADER));

    if (fileSendID && maxBlockSize && filesize && filename &&
        m_sock->readData((void *)&msgHeader,
                         sizeof(NEMSGFILERESUMEACK_HEADER)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        msgHeader.fileSendID = ntohl(msgHeader.fileSendID);
        msgHeader.maxBlockSize = ntohl(msgHeader.maxBlockSize);
        msgHeader.filesize = ntohl(msgHeader.filesize);
        msgHeader.msgFilenameLen = ntohl(msgHeader.msgFilenameLen);

        if (msgHeader.msgType == NE_MSG_FILE_RESUME_ACK)
        {
            filenameLen = MIN(filenameLen,msgHeader.msgFilenameLen);
            if (m_sock->readData((void *)filename,filenameLen) == NC_OK)
            {
                *fileSendID = msgHeader.fileSendID;
                *maxBlockSize = msgHeader.maxBlockSize;
                *filesize = msgHeader.filesize;
                memcpy(md5checksum,msgHeader.md5checksum,
                       16*sizeof(unsigned char));
                filename[filenameLen] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}
