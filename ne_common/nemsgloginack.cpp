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

nemsgLoginAck::nemsgLoginAck(ncSocket *sock) : nemsgCommon(sock)
{
}

nemsgLoginAck::~nemsgLoginAck()
{
}

int nemsgLoginAck::send(char *loginackmsg)
{
    int ret = 1;
    int loginackMsgLen =
        (loginackmsg ? MIN(strlen(loginackmsg),
                           NE_MSG_MAX_GREETING_LEN-1) : 0);

    if (loginackMsgLen)
    {
        NEMSGLOGINACK_HEADER msgHeader;
        memset(&msgHeader,0,sizeof(NEMSGLOGINACK_HEADER));
        msgHeader.msgType = htonl(NE_MSG_LOGIN_ACK);
        msgHeader.msgLength = htonl(loginackMsgLen);

        if ((m_sock->writeData((void *)&msgHeader,
                               sizeof(NEMSGLOGINACK_HEADER)) == NC_OK) &&
            (m_sock->writeData((void *)loginackmsg,loginackMsgLen) == NC_OK))
        {
            ret = 0;
        }
    }
    return ret;
}

int nemsgLoginAck::recv()
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long msgLength = 0;

    if (m_sock->readData((void *)ptr,2*sizeof(unsigned long)) == NC_OK)
    {
        ptr += sizeof(unsigned long);

        /* if this recv method is used, the length must be restricted */
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

int nemsgLoginAck::recv(char *loginackmsg, int maxlen)
{
    int ret = 1;
    unsigned long loginackMsgLen = MIN((unsigned long)maxlen,
                                       NE_MSG_MAX_GREETING_LEN-1);
    NEMSGLOGINACK_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGLOGINACK_HEADER));

    if (loginackmsg &&
        m_sock->readData((void *)&msgHeader,sizeof(msgHeader)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        msgHeader.msgLength = MIN(loginackMsgLen,
                                  ntohl(msgHeader.msgLength));

        if (msgHeader.msgType == NE_MSG_LOGIN_ACK)
        {
            if (m_sock->readData((void *)loginackmsg,
                                 msgHeader.msgLength) == NC_OK)
            {
                loginackmsg[msgHeader.msgLength] = '\0';
                ret = 0;
            }
        }
    }
    return ret;
}

int nemsgLoginAck::format(char *loginackmsg, int maxlen)
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long tokenType = ntohl(*((unsigned long *)ptr));
    unsigned long msgLength = MIN((unsigned long)maxlen,
                                  NE_MSG_MAX_RAW_DATA_LEN-1);

    if (loginackmsg && (tokenType == NE_MSG_ERROR))
    {
        ptr += sizeof(unsigned long);
        msgLength = MIN(msgLength,ntohl(*((unsigned long *)ptr)));

        ptr += sizeof(unsigned long);
        memcpy(loginackmsg,ptr,msgLength);
        loginackmsg[msgLength] = '\0';
      
        ret = 0;
    }
    return ret;
}
