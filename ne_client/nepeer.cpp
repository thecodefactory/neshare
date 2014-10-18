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

#include "neclientheaders.h"

nePeer::nePeer(ncSocket *sock, int ttl)
{
    m_sock = sock;
    m_syncVal = 0;
    m_originalTTL = ttl;
    m_errorCount = 0;
    m_removed = 0;
    resetTTL();
}

nePeer::~nePeer()
{
    delete m_sock;
    m_sock = (ncSocket*)0;
    m_originalTTL = 0;
    m_errorCount = 0;
}

bool nePeer::operator< (nePeer &peer)
{
    return (m_sock < peer.getSocket());
}

bool nePeer::operator== (nePeer &peer)
{
    return (m_sock == peer.getSocket());
}

ncSocket *nePeer::getSocket()
{
    return m_sock;
}

void nePeer::setSyncVal(unsigned long syncVal)
{
    m_syncVal = syncVal;
}

unsigned long nePeer::getSyncVal()
{
    return m_syncVal;
}

int nePeer::isTTLExpired()
{
    int ret = 0;
    struct timeval now;
    gettimeofday(&now,0);
    if (timercmp(&now,&m_ttl,>))
    {
        ret = 1;
    }
    return ret;
}

void nePeer::resetTTL()
{
    struct timeval now;
    gettimeofday(&now,0);
    m_ttl.tv_sec = now.tv_sec + m_originalTTL;
    m_ttl.tv_usec = now.tv_usec;
}

void nePeer::setRemoved()
{
    m_removed = 1;
}

int nePeer::wasRemoved()
{
    return m_removed;
}

void nePeer::incrementErrorCount()
{
    ++m_errorCount;
}

int nePeer::getErrorCount()
{
    return m_errorCount;
}
