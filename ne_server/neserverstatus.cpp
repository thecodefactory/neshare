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

#include "neserverheaders.h"

neServerStatus::neServerStatus()
{
    m_totalNumUsers = 0;
    m_totalNumUserFiles = 0;
    m_totalSizeofFilesInMB = 0;
}

neServerStatus::~neServerStatus()
{
    m_totalNumUsers = 0;
    m_totalNumUserFiles = 0;
    m_totalSizeofFilesInMB = 0;
}

unsigned long neServerStatus::getNumUsers()
{
    unsigned long ret = 0;
    m_mutex.lock();
    ret = m_totalNumUsers;
    m_mutex.unlock();
    return ret;
}

unsigned long neServerStatus::getNumUserFiles()
{
    unsigned long ret = 0;
    m_mutex.lock();
    ret = m_totalNumUserFiles;
    m_mutex.unlock();
    return ret;
}

unsigned long neServerStatus::getTotalSizeofFilesInMB()
{
    unsigned long ret = 0;
    m_mutex.lock();
    ret = m_totalSizeofFilesInMB;
    m_mutex.unlock();
    return ret;
}

void neServerStatus::incNumUsers(int numUsers)
{
    m_mutex.lock();
    m_totalNumUsers += numUsers;
    m_mutex.unlock();
}

void neServerStatus::incNumUserFiles(int numUserFiles)
{
    m_mutex.lock();
    m_totalNumUserFiles += numUserFiles;
    m_mutex.unlock();
}

void neServerStatus::incTotalSizeofFilesInMB(int numMB)
{
    m_mutex.lock();
    m_totalSizeofFilesInMB += numMB;
    m_mutex.unlock();
}


void neServerStatus::decNumUsers(int numUsers)
{
    m_mutex.lock();
    m_totalNumUsers -= numUsers;
    m_mutex.unlock();
}

void neServerStatus::decNumUserFiles(int numUserFiles)
{
    m_mutex.lock();
    m_totalNumUserFiles -= numUserFiles;
    m_mutex.unlock();
}

void neServerStatus::decTotalSizeofFilesInMB(int numMB)
{
    m_mutex.lock();
    m_totalSizeofFilesInMB -= numMB;
    m_mutex.unlock();
}
