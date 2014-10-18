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

#ifdef NESHARE_DEBUG
extern unsigned long numNewUsers;
extern unsigned long numDeletedUsers;
extern unsigned long totalBytesAllocated;
extern unsigned long totalBytesFreed;
#endif

#include "neserverheaders.h"

neUser::neUser(neUserManager *userManager)
{
    m_connection = 0;
    m_firewalled = 0;
    m_controlPort = 0;
    m_errorCount = 0;
    m_originalTTL = 0;
    m_affinityValue = -1;
    m_disconnecting = 0;
    m_sock = (ncSocket *)0;
    m_userManager = userManager;
    m_validFileList = 1;

#ifdef NESHARE_DEBUG
    numNewUsers++;
    totalBytesAllocated += sizeof(neUser);
#endif
}

neUser::~neUser()
{
    m_connection = 0;
    m_firewalled = 0;
    m_controlPort = 0;
    m_errorCount = 0;
    m_originalTTL = 0;
    m_affinityValue = -1;
    m_disconnecting = 0;
    m_validFileList = 0;

    delete m_sock;
    m_sock = (ncSocket *)0;
    m_userManager = (neUserManager *)0;

#ifdef NESHARE_DEBUG
    numDeletedUsers++;
    totalBytesFreed += sizeof(ncSocket);
    totalBytesFreed += sizeof(neUser);
#endif

    if (m_mutex.isLocked())
    {
        eprintf("FIXME: User %x's mutex is still locked!\n",this);
        m_mutex.unlock();
    }
}

bool neUser::operator< (neUser &user)
{
    /* this is an arbitrary measure of LT */
    assert(user.getSocket());
    return (m_sock->getSockfd() < user.getSocket()->getSockfd());
}

bool neUser::operator== (neUser &user)
{
    /* this is not a perfect measure of equality, but it should suffice */
    return (m_sock->getSockfd() == user.getSocket()->getSockfd());
}

int neUser::initialize(ncSocket *sock, 
                       int connection, 
                       int ttl, 
                       int firewallStatus,
                       int controlPort)
{
    int ret = 1;
    if (sock)
    {
        m_connection = connection;
        m_firewalled = firewallStatus;
        m_controlPort = controlPort;
        m_errorCount = 0;
        m_originalTTL = ttl;
        m_sock = sock;
        memset(&m_ttl,0,sizeof(m_ttl));
        memset(&m_pingTime,0,sizeof(m_pingTime));
        memset(&m_statusTime,0,sizeof(m_statusTime));
        resetTTL();
        ret = 0;
    }
    return ret;
}

ncSocket *neUser::getSocket()
{
    return m_sock;
}

int neUser::isTTLExpired()
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

void neUser::resetTTL()
{
    struct timeval now;
    gettimeofday(&now,0);
    m_ttl.tv_sec = now.tv_sec + m_originalTTL;
    m_ttl.tv_usec = now.tv_usec;
}

int neUser::isPingTime()
{
    struct timeval now;
    gettimeofday(&now,0);
    return (timercmp(&now,&m_pingTime,>));
}

int neUser::isFileListValid()
{
    int ret = 0;
    m_mutex.lock();
    ret = m_validFileList;
    m_mutex.unlock();
    return ret;
}

void neUser::setFileListInvalid()
{
    m_mutex.lock();
    m_validFileList = 0;
    m_mutex.unlock();
}


void neUser::setPingTime(int seconds)
{
    struct timeval now;
    gettimeofday(&now,0);
    m_pingTime.tv_sec = now.tv_sec + seconds;
    m_pingTime.tv_usec = now.tv_usec;
}

int neUser::isStatusTime()
{
    struct timeval now;
    gettimeofday(&now,0);
    return (timercmp(&now,&m_statusTime,>));
}

int neUser::isDisconnecting()
{
    int ret = 0;
    m_mutex.lock();
    ret = m_disconnecting;
    m_mutex.unlock();
    return ret;
}

void neUser::setStatusTime(int seconds)
{
    struct timeval now;
    gettimeofday(&now,0);
    m_statusTime.tv_sec = now.tv_sec + seconds;
    m_statusTime.tv_usec = now.tv_usec;
}

void neUser::setDisconnecting()
{
    m_mutex.lock();
    m_disconnecting = 1;
    m_mutex.unlock();
}

void neUser::setAffinityValue(int val)
{
    m_affinityValue = val;
}

int neUser::getConnectionSpeed()
{
    return m_connection;
}

int neUser::getFirewallStatus()
{
    return m_firewalled;
}

int neUser::getControlPort()
{
    return m_controlPort;
}

void neUser::incrementErrorCount()
{
    ++m_errorCount;
}

int neUser::getErrorCount()
{
    return m_errorCount;
}

int neUser::getAffinityValue()
{
    return m_affinityValue;
}

void neUser::lockWriteLock()
{
    m_writeLock.lock();
}

void neUser::unlockWriteLock()
{
    m_writeLock.unlock();
}

neUserManager *neUser::getUserManager()
{
    return m_userManager;
}
