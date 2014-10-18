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

nePeerUploadStatus::nePeerUploadStatus()
{
    m_statusObjIter;
}

nePeerUploadStatus::~nePeerUploadStatus()
{
    PEER_UPLOAD_STATUS *status = (PEER_UPLOAD_STATUS *)0;
    std::vector<PEER_UPLOAD_STATUS *>::iterator iter;
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        status = (*iter);
        free(status);
    }
    m_statusObjs.clear();
    m_statusObjsCache.clear();
}

int nePeerUploadStatus::addUploadStatusObj(PEER_UPLOAD_REC *uploadRec,
                                           unsigned long totalBytes,
                                           char *filename)
{
    int ret = 1;
    PEER_UPLOAD_STATUS *uploadStatus = (PEER_UPLOAD_STATUS *)0;
    uploadStatus = (PEER_UPLOAD_STATUS *)
        malloc(sizeof(PEER_UPLOAD_STATUS));
    if (uploadStatus)
    {
        uploadStatus->uploadRec = uploadRec;
        uploadStatus->currentBytes = 0;
        uploadStatus->totalBytes = totalBytes;
        gettimeofday(&(uploadStatus->startTime),(struct timezone *)0);
        gettimeofday(&(uploadStatus->currentTime),(struct timezone *)0);
        strncpy(uploadStatus->filename,
                neUtils::getFormattedFilename(filename),
                NE_MSG_MAX_DATA_LEN);
        uploadStatus->complete = 0;
        uploadStatus->error = 0;
        uploadStatus->cancelled = 0;
        m_mutex.lock();
        m_statusObjs.push_back(uploadStatus);
        m_mutex.unlock();
        ret = 0;
    }   
    return ret;
}

int nePeerUploadStatus::updateStatus(PEER_UPLOAD_REC *uploadRec)
{
    int ret = 1;
    PEER_UPLOAD_STATUS *uploadStatus = (PEER_UPLOAD_STATUS *)0;
    std::vector<PEER_UPLOAD_STATUS *>::iterator iter;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        uploadStatus = (*iter);
        if (uploadStatus->uploadRec == uploadRec)
        {
            uploadStatus->currentBytes += uploadRec->buflen;
            gettimeofday(&(uploadStatus->currentTime),(struct timezone *)0);
            ret = 0;
            break;
        }
    }
    m_mutex.unlock();
    return ret;
}

void nePeerUploadStatus::removeStatusObj(PEER_UPLOAD_REC *uploadRec)
{
    PEER_UPLOAD_STATUS *uploadStatus = (PEER_UPLOAD_STATUS *)0;
    std::vector<PEER_UPLOAD_STATUS *>::iterator iter;
    std::vector<PEER_UPLOAD_STATUS *>::iterator foundIter;
    int found = 0;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        uploadStatus = (*iter);
        if (uploadStatus->uploadRec == uploadRec)
        {
            ++found;
            foundIter = iter;
            break;
        }
    }
    if (found)
    {
        m_statusObjs.erase(foundIter);
        free(uploadStatus);
    }
    m_mutex.unlock();
}

void nePeerUploadStatus::setComplete(PEER_UPLOAD_REC *uploadRec)
{
    std::vector<PEER_UPLOAD_STATUS *>::iterator iter;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        if ((*iter)->uploadRec == uploadRec)
        {
            (*iter)->complete = 1;
            break;
        }
    }
    m_mutex.unlock();
}

void nePeerUploadStatus::setCancelled(PEER_UPLOAD_REC *uploadRec)
{
    std::vector<PEER_UPLOAD_STATUS *>::iterator iter;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        if ((*iter)->uploadRec == uploadRec)
        {
            (*iter)->cancelled = 1;
            break;
        }
    }
    m_mutex.unlock();
}

void nePeerUploadStatus::setError(PEER_UPLOAD_REC *uploadRec)
{
    std::vector<PEER_UPLOAD_STATUS *>::iterator iter;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        if ((*iter)->uploadRec == uploadRec)
        {
            (*iter)->error = 1;
            break;
        }
    }
    m_mutex.unlock();
}

void nePeerUploadStatus::setPosition(PEER_UPLOAD_REC *uploadRec,
                                     unsigned long position)
{
    std::vector<PEER_UPLOAD_STATUS *>::iterator iter;

    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        if ((*iter)->uploadRec == uploadRec)
        {
            (*iter)->currentBytes = position;
            break;
        }
    }
    m_mutex.unlock();
}

void nePeerUploadStatus::resetStatusObjPtr()
{
    std::vector<PEER_UPLOAD_STATUS *>::iterator iter;

    m_statusObjsCache.clear();
    m_mutex.lock();
    for(iter = m_statusObjs.begin(); iter != m_statusObjs.end(); iter++)
    {
        m_statusObjsCache.push_back(*iter);
    }
    m_mutex.unlock();
    m_statusObjIter = m_statusObjsCache.begin();
}

PEER_UPLOAD_STATUS *nePeerUploadStatus::getNextUploadStatus()
{
    PEER_UPLOAD_STATUS *uploadStatus = (PEER_UPLOAD_STATUS *)0;
    if (m_statusObjIter != m_statusObjsCache.end())
    {
        uploadStatus = (*m_statusObjIter);
        m_statusObjIter++;
    }
    return uploadStatus;
}
