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

nemsgEntrySetStart::nemsgEntrySetStart(ncSocket *sock)
    : nemsgCommon(sock)
{
}

nemsgEntrySetStart::~nemsgEntrySetStart()
{
}

int nemsgEntrySetStart::send(unsigned long numEntries)
{
    int ret = 1;
    NEMSGENTRYSETSTART_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGENTRYSETSTART_HEADER));
    msgHeader.msgType = htonl(NE_MSG_ENTRY_SET_START);
    msgHeader.numEntries = htonl(numEntries);

    if (m_sock->writeData((void *)&msgHeader,
                          sizeof(NEMSGENTRYSETSTART_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgEntrySetStart::recv()
{
    int ret = 1;

    if (m_sock->readData((void *)m_buf,
                         sizeof(NEMSGENTRYSETSTART_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgEntrySetStart::recv(unsigned long *numEntries)
{
    int ret = 1;
    NEMSGENTRYSETSTART_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGENTRYSETSTART_HEADER));

    if (numEntries &&
        m_sock->readData((void *)&msgHeader,
                         sizeof(msgHeader)) == NC_OK)
    {
        if (ntohl(msgHeader.msgType) == NE_MSG_ENTRY_SET_START)
        {
            *numEntries = ntohl(msgHeader.numEntries);
            ret = 0;
        }
    }
    return ret;
}

int nemsgEntrySetStart::format(unsigned long *numEntries)
{
    int ret = 1;
    char *ptr = m_buf;
    unsigned long tokenType = ntohl(*((unsigned long *)ptr));

    if (numEntries && (tokenType == NE_MSG_ENTRY_SET_START))
    {
        ptr += sizeof(unsigned long);
        *numEntries = ntohl(*((unsigned long *)ptr));
        ret = 0;
    }
    return ret;
}
