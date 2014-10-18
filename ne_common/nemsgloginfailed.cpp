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

nemsgLoginFailed::nemsgLoginFailed(ncSocket *sock) : nemsgCommon(sock)
{
}

nemsgLoginFailed::~nemsgLoginFailed()
{
}

int nemsgLoginFailed::send(char *loginfailedmsg)
{
    int ret = 1;
    int loginfailedMsgLen =
        (loginfailedmsg ? MIN(strlen(loginfailedmsg),
                              NE_MSG_MAX_DATA_LEN-1) : 0);

    if (loginfailedMsgLen)
    {
        NEMSGLOGINFAILED_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGLOGINFAILED_HEADER));
        msgHeader.msgType = htonl(NE_MSG_LOGIN_FAILED);
        msgHeader.msgLength = htonl(loginfailedMsgLen);

        if ((m_sock->writeData((void *)&msgHeader,
                               sizeof(NEMSGLOGINFAILED_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)loginfailedmsg,
                               loginfailedMsgLen) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgLoginFailed::recv()
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

int nemsgLoginFailed::recv(char *loginfailedmsg, int maxlen)
{
    int ret = 1;
    unsigned long loginfailedMsgLen = MIN((unsigned long)maxlen,
                                          NE_MSG_MAX_DATA_LEN-1);
    NEMSGLOGINFAILED_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGLOGINFAILED_HEADER));

    if (loginfailedmsg &&
        m_sock->readData((void *)&msgHeader,sizeof(msgHeader)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        msgHeader.msgLength = MIN(loginfailedMsgLen,
                                  ntohl(msgHeader.msgLength));

        if (msgHeader.msgType == NE_MSG_ERROR)
        {
            if (m_sock->readData((void *)loginfailedmsg,
                                 msgHeader.msgLength) == NC_OK)
            {
                loginfailedmsg[msgHeader.msgLength] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}

int nemsgLoginFailed::format(char *loginfailedmsg, int maxlen)
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long tokenType = ntohl(*((unsigned long *)ptr));
    unsigned long msgLength = MIN((unsigned long)maxlen,
                                  NE_MSG_MAX_RAW_DATA_LEN-1);

    if (loginfailedmsg &&
        (tokenType == NE_MSG_LOGIN_FAILED))
    {
        ptr += sizeof(unsigned long);
        msgLength = MIN(msgLength,ntohl(*((unsigned long *)ptr)));

        ptr += sizeof(unsigned long);
        memcpy(loginfailedmsg,ptr,msgLength);
        loginfailedmsg[msgLength] = '\0';

        ret = 0;
    }
    return ret;
}
