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

nemsgError::nemsgError(ncSocket *sock) : nemsgCommon(sock)
{
}

nemsgError::~nemsgError()
{
}

int nemsgError::send(char *errormsg)
{
    int ret = 1;
    int errorMsgLen =
        (errormsg ? MIN(strlen(errormsg),NE_MSG_MAX_DATA_LEN-1) : 0);

    if (errorMsgLen)
    {
        NEMSGERROR_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGERROR_HEADER));
        msgHeader.msgType = htonl(NE_MSG_ERROR);
        msgHeader.msgLength = htonl(errorMsgLen);
   
        if ((m_sock->writeData((void *)&msgHeader,
                               sizeof(NEMSGERROR_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)errormsg,errorMsgLen) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgError::recv()
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

int nemsgError::recv(char *errormsg, int maxlen)
{
    int ret = 1;
    unsigned long errorMsgLen = MIN((unsigned long)maxlen,
                                    NE_MSG_MAX_DATA_LEN-1);
    NEMSGERROR_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGERROR_HEADER));

    if (errormsg &&
        m_sock->readData((void *)&msgHeader,sizeof(msgHeader)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        msgHeader.msgLength = MIN(errorMsgLen,ntohl(msgHeader.msgLength));

        if (msgHeader.msgType == NE_MSG_ERROR)
        {
            if (m_sock->readData((void *)errormsg,
                                 msgHeader.msgLength) == NC_OK)
            {
                errormsg[msgHeader.msgLength] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}

int nemsgError::format(char *errormsg, int maxlen)
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long tokenType = ntohl(*((unsigned long *)ptr));
    unsigned long msgLength = MIN((unsigned long)maxlen,
                                  NE_MSG_MAX_RAW_DATA_LEN-1);

    if (errormsg && (tokenType == NE_MSG_ERROR))
    {
        ptr += sizeof(unsigned long);
        msgLength = MIN(msgLength,ntohl(*((unsigned long *)ptr)));

        ptr += sizeof(unsigned long);
        memcpy(errormsg,ptr,msgLength);
        errormsg[msgLength] = '\0';

        ret = 0;
    }
    return ret;
}
