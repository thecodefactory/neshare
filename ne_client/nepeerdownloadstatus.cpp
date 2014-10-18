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

nePeerDownloadStatus::nePeerDownloadStatus()
{
    m_statusObjIter;
}

nePeerDownloadStatus::~nePeerDownloadStatus()
{
    PEER_DOWNLOAD_STATUS *status = (PEER_DOWNLOAD_STATUS *)0;
    std::vector<PEER_DOWNLOAD_STATUS *>::iterator iter;
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        status = (*iter);
        free(status);
    }
    m_statusObjs.clear();
    m_statusObjsCache.clear();
}

int nePeerDownloadStatus::addDownloadStatusObj(PEER_DOWNLOAD_REC *downloadRec,
                                               unsigned long totalBytes,
                                               char *filename)
{
    int ret = 1;
    PEER_DOWNLOAD_STATUS *downloadStatus = (PEER_DOWNLOAD_STATUS *)0;
    downloadStatus = (PEER_DOWNLOAD_STATUS *)
        malloc(sizeof(PEER_DOWNLOAD_STATUS));
    if (downloadStatus)
    {
        downloadStatus->downloadRec = downloadRec;
        downloadStatus->currentBytes = 0;
        downloadStatus->totalBytes = totalBytes;
        gettimeofday(&(downloadStatus->startTime),(struct timezone *)0);
        gettimeofday(&(downloadStatus->currentTime),(struct timezone *)0);
        strncpy(downloadStatus->filename,
                neUtils::getFormattedFilename(filename),
                NE_MSG_MAX_DATA_LEN);
        downloadStatus->complete = 0;
        downloadStatus->error = 0;
        downloadStatus->cancelled = 0;
        m_mutex.lock();
        m_statusObjs.push_back(downloadStatus);
        m_mutex.unlock();
        ret = 0;
    }   
    return ret;
}

int nePeerDownloadStatus::updateStatus(PEER_DOWNLOAD_REC *downloadRec)
{
    int ret = 1;
    PEER_DOWNLOAD_STATUS *downloadStatus = (PEER_DOWNLOAD_STATUS *)0;
    std::vector<PEER_DOWNLOAD_STATUS *>::iterator iter;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        downloadStatus = (*iter);
        if (downloadStatus->downloadRec == downloadRec)
        {
            downloadStatus->currentBytes += downloadRec->buflen;
            gettimeofday(&(downloadStatus->currentTime),
                         (struct timezone *)0);
            ret = 0;
            break;
        }
    }
    m_mutex.unlock();
    return ret;
}

void nePeerDownloadStatus::removeStatusObj(PEER_DOWNLOAD_REC *downloadRec)
{
    PEER_DOWNLOAD_STATUS *downloadStatus = (PEER_DOWNLOAD_STATUS *)0;
    std::vector<PEER_DOWNLOAD_STATUS *>::iterator iter;
    std::vector<PEER_DOWNLOAD_STATUS *>::iterator foundIter;
    int found = 0;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        downloadStatus = (*iter);
        if (downloadStatus->downloadRec == downloadRec)
        {
            ++found;
            foundIter = iter;
            break;
        }
    }
    if (found)
    {
        m_statusObjs.erase(foundIter);
        free(downloadStatus);
    }
    m_mutex.unlock();
}

void nePeerDownloadStatus::setComplete(PEER_DOWNLOAD_REC *downloadRec)
{
    std::vector<PEER_DOWNLOAD_STATUS *>::iterator iter;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        if ((*iter)->downloadRec == downloadRec)
        {
            (*iter)->complete = 1;
            break;
        }
    }
    m_mutex.unlock();
}

void nePeerDownloadStatus::setCancelled(PEER_DOWNLOAD_REC *downloadRec)
{
    std::vector<PEER_DOWNLOAD_STATUS *>::iterator iter;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        if ((*iter)->downloadRec == downloadRec)
        {
            (*iter)->cancelled = 1;
            break;
        }
    }
    m_mutex.unlock();
}

void nePeerDownloadStatus::setError(PEER_DOWNLOAD_REC *downloadRec)
{
    std::vector<PEER_DOWNLOAD_STATUS *>::iterator iter;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        if ((*iter)->downloadRec == downloadRec)
        {
            (*iter)->error = 1;
            break;
        }
    }
    m_mutex.unlock();
}

void nePeerDownloadStatus::setPosition(PEER_DOWNLOAD_REC *downloadRec,
                                       unsigned long position)
{
    std::vector<PEER_DOWNLOAD_STATUS *>::iterator iter;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        if ((*iter)->downloadRec == downloadRec)
        {
            (*iter)->currentBytes = position;
            break;
        }
    }
    m_mutex.unlock();
}

void nePeerDownloadStatus::resetStatusObjPtr()
{
    std::vector<PEER_DOWNLOAD_STATUS *>::iterator iter;

    m_statusObjsCache.clear();
    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        m_statusObjsCache.push_back(*iter);
    }
    m_mutex.unlock();
    m_statusObjIter = m_statusObjsCache.begin();
}

PEER_DOWNLOAD_STATUS *nePeerDownloadStatus::getNextDownloadStatus()
{
    PEER_DOWNLOAD_STATUS *downloadStatus = (PEER_DOWNLOAD_STATUS *)0;
    if (m_statusObjIter != m_statusObjsCache.end())
    {
        downloadStatus = (*m_statusObjIter);
        m_statusObjIter++;
    }
    return downloadStatus;
}
