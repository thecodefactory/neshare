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

#include "nemsgsearchresults.h"

nemsgSearchResults::nemsgSearchResults(ncSocket *sock)
{
    m_sock = sock;
}

nemsgSearchResults::~nemsgSearchResults()
{
    m_sock = (ncSocket *)0;
}

int nemsgSearchResults::send(unsigned long numResults)
{
    int ret = 1;
   
    /* fill out nemsgSearchResults header */
    NEMSGSEARCHRESULTS_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGSEARCHRESULTS_HEADER));
    msgHeader.msgType = htonl(NE_MSG_SEARCH_RESULTS);
    msgHeader.numResults = htonl(numResults);

    /* write the data */
    if (m_sock->writeData((void *)&msgHeader,
                          sizeof(NEMSGSEARCHRESULTS_HEADER)) == NC_OK)
    {
        ret = 0;
    }
    return ret;
}

int nemsgSearchResults::recv(unsigned long *numResults)
{
    int ret = 1;
    NEMSGSEARCHRESULTS_HEADER msgHeader;
    memset(&msgHeader,0,sizeof(NEMSGSEARCHRESULTS_HEADER));

    if (m_sock->readData((void *)&msgHeader,
                         sizeof(NEMSGSEARCHRESULTS_HEADER)) == NC_OK)
    {
        msgHeader.msgType = ntohl(msgHeader.msgType);
        msgHeader.numResults = ntohl(msgHeader.numResults);

        if (msgHeader.msgType != NE_MSG_SEARCH_RESULTS)
        {
            return ret;
        }
        *numResults = msgHeader.numResults;
        ret = 0;
    }
    return ret;
}
