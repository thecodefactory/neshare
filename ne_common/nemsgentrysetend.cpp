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

nemsgEntrySetEnd::nemsgEntrySetEnd(ncSocket *sock)
    : nemsgCommon(sock)
{
}

nemsgEntrySetEnd::~nemsgEntrySetEnd()
{
}

int nemsgEntrySetEnd::send(unsigned long numEntries)
{
    int ret = 1;
    NEMSGENTRYSETEND_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGENTRYSETEND_HEADER));
    msgHeader.msgType = htonl(NE_MSG_ENTRY_SET_END);
    msgHeader.numEntries = htonl(numEntries);

    if (m_sock->writeData((void *)&msgHeader,
                          sizeof(NEMSGENTRYSETEND_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgEntrySetEnd::recv()
{
    int ret = 1;

    if (m_sock->readData((void *)m_buf,
                         sizeof(NEMSGENTRYSETEND_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgEntrySetEnd::recv(unsigned long *numEntries)
{
    int ret = 1;
    NEMSGENTRYSETEND_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGENTRYSETEND_HEADER));

    if (numEntries &&
        m_sock->readData((void *)&msgHeader,
                         sizeof(msgHeader)) == NC_OK)
    {
        if (ntohl(msgHeader.msgType) == NE_MSG_ENTRY_SET_END)
        {
            *numEntries = ntohl(msgHeader.numEntries);
            ret = 0;
        }
    }
    return ret;
}

int nemsgEntrySetEnd::format(unsigned long *numEntries)
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long tokenType = ntohl(*((unsigned long *)ptr));

    if (numEntries && (tokenType == NE_MSG_ENTRY_SET_END))
    {
        ptr += sizeof(unsigned long);
        *numEntries = ntohl(*((unsigned long *)ptr));
        ret = 0;
    }
    return ret;
}
