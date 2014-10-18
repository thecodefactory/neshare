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

nemsgStatus::nemsgStatus(ncSocket *sock) : nemsgCommon(sock)
{
}

nemsgStatus::~nemsgStatus()
{
}

int nemsgStatus::send(unsigned long numUsers,
                      unsigned long numFiles,
                      unsigned long numMB)
{
    int ret = 1;
    NEMSGSTATUS_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGSTATUS_HEADER));
    msgHeader.msgType = htonl(NE_MSG_STATUS);
    msgHeader.msgNumUsers = htonl(numUsers);
    msgHeader.msgNumFiles = htonl(numFiles);
    msgHeader.msgNumMB = htonl(numMB);
   
    if (m_sock->writeData((void *)&msgHeader,
                          sizeof(NEMSGSTATUS_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgStatus::recv()
{
    int ret = 1;
   
    if (m_sock->readData((void *)m_buf,
                         sizeof(NEMSGSTATUS_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgStatus::recv(unsigned long *numUsers, 
                      unsigned long *numFiles,
                      unsigned long *numMB)
{
    int ret = 1;
    NEMSGSTATUS_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGSTATUS_HEADER));

    if (numUsers && numFiles && numMB &&
        m_sock->readData((void *)&msgHeader,sizeof(msgHeader)) == NC_OK)
    {
        *numUsers = ntohl(msgHeader.msgNumUsers);
        *numFiles = ntohl(msgHeader.msgNumFiles);
        *numMB = ntohl(msgHeader.msgNumMB);

        ret = 0;
    }
    return ret;
}

int nemsgStatus::format(unsigned long *numUsers, 
                        unsigned long *numFiles,
                        unsigned long *numMB)
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long tokenType = ntohl(*((unsigned long *)ptr));

    assert(numUsers);
    assert(numFiles);
    assert(numMB);

    if (numUsers && numFiles && numMB && (tokenType == NE_MSG_STATUS))
    {
        ptr += sizeof(unsigned long);
        *numUsers = ntohl(*((unsigned long *)ptr));

        ptr += sizeof(unsigned long);
        *numFiles = ntohl(*((unsigned long *)ptr));

        ptr += sizeof(unsigned long);
        *numMB = ntohl(*((unsigned long *)ptr));

        ret = 0;
    }
    return ret;
}
