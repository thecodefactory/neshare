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

nePeerDownloadManager::nePeerDownloadManager()
{
    m_config = (neConfig *)0;
    m_fnptr = (neDownloadCallback)0;
    m_callbackTimeDelay = 1000;
    m_fdIDMap.clear();
    m_filenameCacheMap.clear();
    m_fileIDRemovalTimeMap.clear();
}

nePeerDownloadManager::~nePeerDownloadManager()
{
    std::map< unsigned long, std::map <
        unsigned long, PEER_DOWNLOAD_REC * > >::iterator
        iter = m_fdIDMap.begin();
    std::map< unsigned long, PEER_DOWNLOAD_REC * >::iterator innerMapIter;
    PEER_DOWNLOAD_REC *downloadRec = (PEER_DOWNLOAD_REC *)0;

    /* this action must be completed atomically */
    m_fdIDMapMutex.lock();

    /*
      de-alocate all remaining downloadRec objects and also the 
      allocated buffers that they contain
    */
    while(iter != m_fdIDMap.end())
    {
        innerMapIter = ((*iter).second).begin();
        while(innerMapIter != ((*iter).second).end())
        {
            downloadRec = (*innerMapIter).second;

            /* close the fd inside the downloadRec */
            fclose(downloadRec->fd);

            /* free the filename inside the downloadRec */
            free(downloadRec->filename);

            /* free the buffer inside the downloadRec */
            free(downloadRec->buf);

            /* free the downloadRec object */
            free(downloadRec);
            innerMapIter++;
        }
        /* empty the inner map */
        ((*iter).second).clear();
        iter++;
    }
    /* empty the top-level map */
    m_fdIDMap.clear();

    /* empty the filename cache map */
    m_filenameCacheMutex.lock();
    m_filenameCacheMap.clear();
    m_filenameCacheMutex.unlock();

    /* empty the fileIDRemovalTimeMap */
    m_fileIDRemovalTimeMapMutex.lock();
    m_fileIDRemovalTimeMap.clear();
    m_fileIDRemovalTimeMapMutex.unlock();

    m_fdIDMapMutex.unlock();
}

void nePeerDownloadManager::setConfig(neConfig *config)
{
    m_fdIDMapMutex.lock();
    m_config = config;
    m_fdIDMapMutex.unlock();
}

int nePeerDownloadManager::addPeerDownload(unsigned long fileID,
                                           unsigned long maxBlockSize,
                                           unsigned long filesize,
                                           unsigned char md5checksum[16],
                                           nePeer *peer,
                                           char *filename)
{
    int ret = 1;
    FILE *outputfd = (FILE *)0;
    std::string formattedFileName =
        neUtils::getFormattedOutputFilename(filename);
    std::string outputFile = std::string(m_config->getSavePath()) +
        formattedFileName;
    PEER_DOWNLOAD_REC *downloadRec = (PEER_DOWNLOAD_REC *)0;

    /* make sure the specified file is not already being downloaded */
    downloadRec = isFileBeingDownloaded(fileID,
                                        peer->getSocket()->getSockfd(),
                                        (char *)outputFile.c_str());

    if (downloadRec != (PEER_DOWNLOAD_REC *)0)
    {
        iprintf("nePeerDownloadManager::addPeerDownload | File is "
                "already being downloaded!\n");
        return ret;
    }

    /* this action must be completed atomically */
    m_fdIDMapMutex.lock();

    /*
      NOTE: we're opening the file with mode "a" which means if it
      exists, the file position is moved to the end of the file,
      and if it doesn't exist, the file is created.  Thus, we
      handle resumes or new downloads in the same manner. -N.M.
    */
    if ((outputfd = fopen(outputFile.c_str(),"a")) == NULL)
    {
        eprintf("nePeerDownloadManager::addPeerDownload | File %s "
                "cannot be opened for writing.  Skipping.\n",
                outputFile.c_str());
        m_fdIDMapMutex.unlock();
        return ret;
    }

    /* seek to the end of the file */
    if (fseek(outputfd,0L,SEEK_END) == -1)
    {
        eprintf("nePeerDownloadManager::addPeerDownload | Error "
                "seeking to end of file.\n");
        m_fdIDMapMutex.unlock();
        return ret;
    }

    /* now create a new record and associate it with this peer and fileID */
    downloadRec = (PEER_DOWNLOAD_REC *)malloc(sizeof(PEER_DOWNLOAD_REC));
    if (downloadRec)
    {
        downloadRec->fd = outputfd;
        downloadRec->fileID = fileID;
        downloadRec->peer = peer;
        downloadRec->maxBlockSize = maxBlockSize;
        downloadRec->buf = (char *)malloc(maxBlockSize*sizeof(char));
        downloadRec->buflen = 0;
        downloadRec->filename = (char *)calloc(outputFile.length()+1,
                                               sizeof(char));
        strcpy(downloadRec->filename,outputFile.c_str());
        downloadRec->nextCallbackTime.tv_sec = 0;
        downloadRec->nextCallbackTime.tv_usec = 0;
        memcpy(downloadRec->md5checksum,md5checksum,
               16*sizeof(unsigned char));

        /*
          add the entry to the fdID Map
          (mapping fd->fileID->downloadRec)
        */
        std::map< unsigned long, PEER_DOWNLOAD_REC * >& innerMap = 
            m_fdIDMap[peer->getSocket()->getSockfd()];
        innerMap[fileID] = downloadRec;

        /* add the filename to the filename cache */
        m_filenameCacheMutex.lock();
        m_filenameCacheMap[std::string(downloadRec->filename)] = downloadRec;
        m_filenameCacheMutex.unlock();

        /* check for a registered callback */
        if (m_fnptr)
        {
            unsigned long position = (unsigned long)ftell(downloadRec->fd);
            if (m_peerDownloadStatus.addDownloadStatusObj(downloadRec,filesize,
                                                          downloadRec->filename))
            {
                iprintf("nePeerDownloadManager::addPeerDownload | "
                        "Ignoring status object.\n");
            }
            else if (position)
            {
                /*
                  update the status object position
                  if we're resuming a file
                */
                m_peerDownloadStatus.setPosition(downloadRec,position);
            }
        }
        ret = 0;
    }
    else
    {
        eprintf("nePeerDownloadManager::addPeerDownload | Not enough "
                "memory for required allocation.  Skipping.\n");
    }
    m_fdIDMapMutex.unlock();
    return ret;
}

int nePeerDownloadManager::getFileDataSend(nePeer *peer)
{
    int ret = 1;
    nemsgFileDataSend dataSendMsg(peer->getSocket());

    /*
      first, retrieve the required
      NE_MSG_FILE_DATA_SEND meta-components
    */
    unsigned long msgType = dataSendMsg.getMessageType();
    unsigned long fileID = dataSendMsg.getFileID();
    unsigned long dataLength = dataSendMsg.getDataLength();
    unsigned long sockfd = (unsigned long)peer->getSocket()->getSockfd();
    unsigned long writeLen = 0;

    /*
      if the msgType and fileID are valid then get
      the corresponding downloadRec
    */
    if ((msgType == NE_MSG_FILE_DATA_SEND) && (fileID != INVALID_FILE_ID))
    {
        /* then retrieve the downloadRec corresponding to this peer */
        PEER_DOWNLOAD_REC *downloadRec = getDownloadRec(fileID,sockfd);
        if (downloadRec)
        {
            /* stash the current data length in the downloadRec */
            downloadRec->buflen = dataLength;

            /* now that we have the downloadRec, read the incoming data */
            if (dataSendMsg.readData((void *)downloadRec->buf,
                                     downloadRec->maxBlockSize) == 0)
            {
                /* now that we have the incoming data, write it to disk */
                writeLen = MIN(downloadRec->maxBlockSize,downloadRec->buflen);
                if (fwrite((void *)downloadRec->buf,sizeof(char),
                           writeLen,downloadRec->fd) != writeLen)
                {
                    eprintf("nePeerDownloadManager::getFileDataSend | "
                            "ERROR! Disk write failed.\n");
                    eprintf("nePeerDownloadManager::getFileDataSend | "
                            "Failed to write %d bytes.\n",writeLen);
                    eprintf("nePeerDownloadManager::getFileDataSend | "
                            "File will be corrupt.\n");

                    /* cancel the download */
                    cancelPeerDownload(
                        downloadRec->peer->getSocket()->getIpAddr(),
                        downloadRec->filename);
                }
                else
                {
                    ret = 0;
                }
            }
            else
            {
                eprintf("nePeerDownloadManager::getFileDataSend | "
                        "ERROR! Data from peer cannot be read.\n");

                m_peerDownloadStatus.setError(downloadRec);

                /* cancel the download */
                cancelPeerDownload(
                    downloadRec->peer->getSocket()->getIpAddr(),
                    downloadRec->filename);
            }

            /*
              if we have a callback registered, update the status
              object and call the registered callback
            */
            if (m_fnptr)
            {
                m_peerDownloadStatus.updateStatus(downloadRec);
                callRegisteredCallback(downloadRec, 0);
            }
        }
        else
        {
            /*
              The only way it could happen is if getDownloadRec(...)
              is buggy - or if this method is called without having
              called addPeerDownload before it - or if
              nemsgFileDataSend::getFileID is broken - or the if peer
              really sent garbage.  The most common case though is
              that the referenced downloadRec was recently removed
              (which happens in the case of a cancellation)
            */

            /* discard whatever data was sent */
            neUtils::discardSocketData(peer->getSocket(),
                                       (int)dataLength);

            /* if the peer was recently removed, this is not an error */
            if (wasRecentlyRemoved(fileID))
            {
                dprintf("nePeerDownloadManager::getFileDataSend | "
                        "Discarding %lu bytes on fileID %lu\n",
                        dataLength,fileID);
                ret = 0;
            }
            else
            {
                eprintf("nePeerDownloadManager::getFileDataSend | "
                        "ERROR! Retrieved data from an unknown peer.\n");
            }
        }
    }
    else
    {
        eprintf("nePeerDownloadManager::getFileDataSend | Invalid "
                "File ID sent from peer. Ignoring.\n");
        eprintf("nePeerDownloadManager::getFileDataSend | msgType "
                "= %lu | File ID = %lu\n",msgType,fileID);
    }

    /* remove all cancelled downloadRecs (if any) */
    removeCancelledDownloads();
    return ret;
}

int nePeerDownloadManager::endFileDataSend(nePeer *peer)
{
    int ret = 1;
    unsigned long fileID = 0;

    /* first, retrieve the fileID of the incoming data */
    nemsgFileDataSendEnd dataSendEndMsg(peer->getSocket());
    if (dataSendEndMsg.recv(&fileID))
    {
        eprintf("nePeerDownloadManager::endFileDataSend | failed to "
                "recv data send end message\n");
    }
    else if (fileID != INVALID_FILE_ID)
    {
        /* 
           if we have a registered callback, set the complete flag
           and call the callback one last time.
        */
        if (m_fnptr)
        {
            PEER_DOWNLOAD_REC *downloadRec = 
                getDownloadRec(fileID,peer->getSocket()->getSockfd());
            if (downloadRec)
            {
                m_peerDownloadStatus.setComplete(downloadRec);
                callRegisteredCallback(downloadRec, 1);
            }
        }
        if (removePeerDownload(peer,fileID))
        {
            eprintf("nePeerDownloadManager::endFileDataSend | failed to "
                    "remove peer download (%lu)\n",fileID);
        }
        else
        {
            ret = 0;
        }
    }
    else
    {
        eprintf("nePeerDownloadManager::getFileDataSendEnd | Invalid File "
                "ID sent from peer. Ignoring.\n");
    }
    return ret;
}

int nePeerDownloadManager::cancelPeerDownload(unsigned long ipAddr,
                                              char *filename,
                                              int sendCancelMsg)
{
    int ret = 1;
    PEER_DOWNLOAD_REC *downloadRec = (PEER_DOWNLOAD_REC *)0;
    std::map< unsigned long, std::map <
        unsigned long, PEER_DOWNLOAD_REC * > >::iterator
        iter;
    std::map< unsigned long, PEER_DOWNLOAD_REC * > *innerMap = 0;
    std::map< unsigned long, PEER_DOWNLOAD_REC * >::iterator innerMapIter;

    iprintf("nepeerManager::cancelPeerDownload | cancelling peer "
            "download (%s | %s)\n",neUtils::ipToStr(ipAddr),filename);

    /* this action must be completed atomically */
    m_fdIDMapMutex.lock();

    /* lookup the IP address in maps to see if we find anything */
    iter = m_fdIDMap.begin();
    while(iter != m_fdIDMap.end())
    {
        innerMap = &((*iter).second);

        if (innerMap)
        {
            innerMapIter = innerMap->begin();
            while(innerMapIter != innerMap->end())
            {
                downloadRec = (*innerMapIter).second;
                if (ipAddr == downloadRec->peer->getSocket()->getIpAddr())
                {
                    /*
                      Note: while there is a better way to determine
                      which file is being cancelled, basing it on Ip
                      address AND distinct filename should be good
                      enough since you can't download multiple files
                      with the same filename from the same peer Ip address
                    */
                    std::string formattedFileName =
                        neUtils::getFormattedOutputFilename(filename);
                    std::string outputFile =
                        std::string(m_config->getSavePath()) +
                        formattedFileName;
                    if (strcmp(outputFile.c_str(),
                               downloadRec->filename) == 0)
                    {
                        m_fdIDMapMutex.unlock();

                        if (sendCancelMsg)
                        {
                            /*
                              send a cancellation message to the peer
                              who is uploading
                            */
                            nemsgFileDataCancel fileDataCancelMsg(
                                downloadRec->peer->getSocket());
                            ret = fileDataCancelMsg.send(
                                downloadRec->fileID);
                            downloadRec->peer->getSocket()->flush();
                        }
                        if (m_fnptr)
                        {
                            m_peerDownloadStatus.setCancelled(
                                downloadRec);
                        }
                        ret = 0;
                    }
                }
                else
                {
                    /*
                      if the ip doesn't match,
                      move to the next 'bucket'
                    */
                    break;
                }
                innerMapIter++;
            }
        }
        iter++;
    }
    m_fdIDMapMutex.unlock();
    return ret;
}

int nePeerDownloadManager::cancelPeerDownload(unsigned long ipAddr,
                                              unsigned long fileID,
                                              int sendCancelMsg)
{
    int ret = 1;
    PEER_DOWNLOAD_REC *downloadRec = (PEER_DOWNLOAD_REC *)0;
    std::map< unsigned long, std::map <
        unsigned long, PEER_DOWNLOAD_REC * > >::iterator iter;
    std::map< unsigned long, PEER_DOWNLOAD_REC * > *innerMap = 0;
    std::map< unsigned long, PEER_DOWNLOAD_REC * >::iterator
        innerMapIter;

    iprintf("nePeerDownloadManager::cancelPeerDownload | Cancelling "
            "peer download (%s | ID %lu)\n",
            neUtils::ipToStr(ipAddr),fileID);

    /* this action must be completed atomically */
    m_fdIDMapMutex.lock();

    /* lookup the IP address in maps to see if we find anything */
    iter = m_fdIDMap.begin();
    while(iter != m_fdIDMap.end())
    {
        innerMap = &((*iter).second);

        if (innerMap)
        {
            innerMapIter = innerMap->begin();
            while(innerMapIter != innerMap->end())
            {
                downloadRec = (*innerMapIter).second;
                if (ipAddr == downloadRec->peer->getSocket()->getIpAddr())
                {
                    if (fileID == downloadRec->fileID)
                    {
                        m_fdIDMapMutex.unlock();
                  
                        if (sendCancelMsg)
                        {
                            /*
                              send a cancellation message to the
                              peer who is uploading
                            */
                            nemsgFileDataCancel fileDataCancelMsg(
                                downloadRec->peer->getSocket());
                            ret = fileDataCancelMsg.send(
                                downloadRec->fileID);
                            downloadRec->peer->getSocket()->flush();
                        }
                        if (m_fnptr)
                        {
                            m_peerDownloadStatus.setCancelled(
                                downloadRec);
                        }
                        ret = 0;
                    }
                }
                else
                {
                    /*
                      if the ip doesn't match,
                      move to the next 'bucket'
                    */
                    break;
                }
                innerMapIter++;
            }
        }
        iter++;
    }
    m_fdIDMapMutex.unlock();
    return ret;
}

void nePeerDownloadManager::removeCancelledDownloads()
{
    PEER_DOWNLOAD_REC *downloadRec = (PEER_DOWNLOAD_REC *)0;
    PEER_DOWNLOAD_STATUS *status = (PEER_DOWNLOAD_STATUS *)0;
    std::vector<PEER_DOWNLOAD_REC *> finishedDownloads;
    std::vector<PEER_DOWNLOAD_REC *>::iterator fdIter;

    if (m_fnptr)
    {
        finishedDownloads.clear();

        /* mark the downloadRecs with the cancelled flag set */
        m_peerDownloadStatus.resetStatusObjPtr();
        while((status = m_peerDownloadStatus.getNextDownloadStatus()))
        {
            if (status->cancelled)
            {
                finishedDownloads.push_back(status->downloadRec);
            }
        }
    }

    /*
      call the callback for the last time with
      all cancelled downloads still intact
    */
    if (!finishedDownloads.empty())
    {
        callRegisteredCallback((PEER_DOWNLOAD_REC *)0, 1);
    }

    /* remove all marked downloads one at a time */
    for(fdIter = finishedDownloads.begin(); 
        fdIter != finishedDownloads.end(); fdIter++)
    {
        downloadRec = (*fdIter);
        removePeerDownload(downloadRec->peer,downloadRec->fileID);
    }
}

int nePeerDownloadManager::removeAllPeerDownloads(nePeer *peer)
{
    int ret = 1;
    PEER_DOWNLOAD_REC *downloadRec = (PEER_DOWNLOAD_REC *)0;
    std::map< unsigned long, std::map < unsigned long, PEER_DOWNLOAD_REC * > >::iterator
        iter;
    std::map< unsigned long, PEER_DOWNLOAD_REC * > *innerMap = 0;
    std::map< unsigned long, PEER_DOWNLOAD_REC * >::iterator innerMapIter;
    std::vector<PEER_DOWNLOAD_REC *> finishedDownloads;
    std::vector<PEER_DOWNLOAD_REC *>::iterator fdIter;

    /* this action must be completed atomically */
    m_fdIDMapMutex.lock();

    /* search for the 'bucket' associated with this peer's socket desc */
    iter = m_fdIDMap.begin();
    while(iter != m_fdIDMap.end())
    {
        if ((unsigned long)peer->getSocket()->getSockfd() ==
            (*iter).first)
        {
            /* once we find it, mark every downloadRec in it */
            innerMap = &((*iter).second);
         
            if (innerMap)
            {
                innerMapIter = innerMap->begin();
                while(innerMapIter != innerMap->end())
                {
                    downloadRec = (*innerMapIter).second;
                    if (downloadRec->peer == peer)
                    {
                        finishedDownloads.push_back(downloadRec);
                    }
                    else
                    {
                        /*
                          the check for the matching peer above is a
                          sanity check. print errors if it fails - but
                          remove this once convinced that this part
                          works all the time.
                        */
                        eprintf("nePeerDownloadManager::removeAllPeerDownloads"
                                " | FIXME: inconsistent mapping from "
                                "socket desc to peer object!\n");
                    }
                    innerMapIter++;
                }
            }
        }
        iter++;
    }
    m_fdIDMapMutex.unlock();

    /* finally, remove all marked downloads one at a time */
    for(fdIter = finishedDownloads.begin(); 
        fdIter != finishedDownloads.end(); fdIter++)
    {
        downloadRec = (*fdIter);
        ret += removePeerDownload(downloadRec->peer,
                                  downloadRec->fileID);
    }
    return ret;
}

int nePeerDownloadManager::removePeerDownload(nePeer *peer,
                                              unsigned long fileID)
{
    int ret = 1;
    unsigned char md5checksum[16];
    unsigned long sockfd = (unsigned long)peer->getSocket()->getSockfd();
    struct timeval expirationTime;
    std::map< unsigned long, std::map < unsigned long, PEER_DOWNLOAD_REC * > >::iterator
        iter;
    std::map< unsigned long, PEER_DOWNLOAD_REC * > *innerMap = 0;
    std::map< unsigned long, PEER_DOWNLOAD_REC * >::iterator innerMapIter;
    std::map< std::string, PEER_DOWNLOAD_REC * >::iterator filenameCacheIter;

    /* if the fileID is valid then get the corresponding downloadRec */
    PEER_DOWNLOAD_REC *downloadRec = getDownloadRec(fileID,sockfd);
    if (downloadRec)
    {
        /* this action must be completed atomically */
        m_fdIDMapMutex.lock();

        /* first flush and close the file decriptor */
        fflush(downloadRec->fd);
        fclose(downloadRec->fd);
      
        /* then check if the md5checksums match */
        if (memcmp(downloadRec->md5checksum,INVALID_MD5_CHECKSUM,
                   16*sizeof(unsigned char)) != 0)
        {
            neUtils::computeQuickMD5Checksum(
                downloadRec->filename,md5checksum);

            if (memcmp(md5checksum,downloadRec->md5checksum,
                       16*sizeof(unsigned char)) != 0)
            {
                /* DEBUG - ERASE ME */
                dprintf("COMPUTED MD5SUM: ");
                for(int i = 0; i < 16; i++)
                {
                    dprintf("%2x", md5checksum[i]);
                }
                dprintf("\n");

                dprintf("RECEIVED MD5SUM: ");
                for(int i = 0; i < 16; i++)
                {
                    dprintf("%2x", downloadRec->md5checksum[i]);
                }
                dprintf("\n");

                /*
                  if the checksums don't match, warn the user (?).
                  Better to call a user-registered error logging
                  callback
                */
                iprintf("nePeerDownloadManager::removePeerDownload | "
                        "Checksum failure on %s\n",
                        downloadRec->filename);
                iprintf("nePeerDownloadManager::removePeerDownload | "
                        "File may be corrupt or invalid.\n");
            }
        }

        /*
          then, remove this downloadRec from the
          internal maps after locating it
        */
        if ((iter = m_fdIDMap.find(sockfd)) != m_fdIDMap.end())
        {
            innerMap = &((*iter).second);
            if ((innerMapIter = innerMap->find(fileID)) != innerMap->end())
            {
                /* should assert that downloadRec ==
                   (*innerMapIter).second */
                /* erase the fileID and downloadRec from the map */
                innerMap->erase(innerMapIter);
            
                /*
                  de-allocate and uninitialize the
                  downloadRec (and statusObj if any)
                */
                if (m_fnptr)
                {
                    m_peerDownloadStatus.removeStatusObj(downloadRec);
                }
            
                /*
                  add the fileID to the recently removed map;
                  if the fileID already existed, overwrite is ok
                */
                gettimeofday(&expirationTime, NULL);
                expirationTime.tv_sec += 10;
                m_fileIDRemovalTimeMapMutex.lock();
                m_fileIDRemovalTimeMap[downloadRec->fileID] =
                    expirationTime;
                m_fileIDRemovalTimeMapMutex.unlock();

                /* remove the filename from the filename cache map */
                m_filenameCacheMutex.lock();
                filenameCacheIter = 
                    m_filenameCacheMap.find(
                        std::string(downloadRec->filename));
                assert(filenameCacheIter != m_filenameCacheMap.end());
                m_filenameCacheMap.erase(filenameCacheIter);
                m_filenameCacheMutex.unlock();

                /* delete the allocated memory from the downloadRec */
                free(downloadRec->filename);
                free(downloadRec->buf);
                free(downloadRec);

                /*
                  now check if the innerMap is empty so we can erase the
                  link to the ip address if it's no longer valid
                */
                if (innerMap->size() == 0)
                {
                    m_fdIDMap.erase(iter);
                }
            
                ret = 0;
            }
            else
            {
                /* FIXME: handle error case better */
                eprintf("nePeerDownloadManager::removePeerDownload | "
                        "DownloadRec references a peer download file "
                        "ID which does not exist in maps.  Ignoring.\n");
            }
        }
        else
        {
            /* FIXME: handle error case better */
            eprintf("nePeerDownloadManager::removePeerDownload | "
                    "DownloadRec references a peer which does not "
                    "exist in maps.  Ignoring.\n");
        }
    }
    else
    {
        eprintf("nePeerDownloadManager::removePeerDownload | "
                "peer references a DownloadRec which cannot "
                "be found. (fileID = %lu | ipAddr = %s)\n",
                fileID,neUtils::ipToStr(peer->getSocket()->getIpAddr()));
    }
    m_fdIDMapMutex.unlock();
    return ret;
}

unsigned long nePeerDownloadManager::fileExistsLocally(char *filename)
{
    unsigned long filesize = 0;
    struct stat statbuf;
    memset(&statbuf,0,sizeof(statbuf));

    std::string formattedFileName =
        neUtils::getFormattedOutputFilename(filename);
    std::string outputFile =
        std::string(m_config->getSavePath()) + formattedFileName;
    if (stat(outputFile.c_str(),&statbuf) == 0)
    {
        filesize = (unsigned long)statbuf.st_size;
    }
    return filesize;
}

void nePeerDownloadManager::registerCallback(neDownloadCallback fnptr)
{
    iprintf("nePeerDownloadManager::registerCallback | "
            "callback registered.\n");
    m_fnptr = fnptr;
}

void nePeerDownloadManager::setCallbackDelay(unsigned long millisecs)
{
    m_callbackTimeDelay = millisecs;
}

void nePeerDownloadManager::callRegisteredCallback(
    PEER_DOWNLOAD_REC *downloadRec,
    int force)
{
    /* note that if force is 1, downloadRec may be NULL */
    struct timeval now;
    gettimeofday(&now,0);

    if (force ||
        (downloadRec ?
         (timercmp(&now,&(downloadRec->nextCallbackTime),>)) : 0))
    {
        /*
          set the next callback time for this
          download in particular (if specified)
        */
        if (downloadRec)
        {
            downloadRec->nextCallbackTime.tv_sec = now.tv_sec +
                (m_callbackTimeDelay/1000);
            downloadRec->nextCallbackTime.tv_usec = now.tv_usec +
                (m_callbackTimeDelay%1000);
        }

        m_peerDownloadStatus.resetStatusObjPtr();
        (*m_fnptr)(&m_peerDownloadStatus);
    }
}

PEER_DOWNLOAD_REC *nePeerDownloadManager::isFileBeingDownloaded(
    unsigned long fileID, int socket, char *filename)
{
    PEER_DOWNLOAD_REC *downloadRec = getDownloadRec(fileID,socket);
    std::map< std::string, PEER_DOWNLOAD_REC * >::iterator iter;

    /*
      make sure this record does not already
      exist in download record map
    */
    if (downloadRec != (PEER_DOWNLOAD_REC *)0)
    {
        eprintf("nePeerDownloadManager::isFileBeingDownloaded | "
                "This fileID and Peer exists in map already! "
                "Skipping.\n");
        eprintf("nePeerDownloadManager::isFileBeingDownloaded | "
                "FIXME: Unique indexing scheme is not unique "
                "enough!\n");
    }
    else
    {
        /* 
           query the filename cache map to make sure another
           download isn't currently writing to the same filename
        */
        m_filenameCacheMutex.lock();
        if ((iter = m_filenameCacheMap.find(filename)) !=
            m_filenameCacheMap.end())
        {
            downloadRec = (*iter).second;
        }
        m_filenameCacheMutex.unlock();

        if (downloadRec != (PEER_DOWNLOAD_REC *)0)
        {
            eprintf("nePeerDownloadManager::isFileBeingDownloaded "
                    "| A copy of this file is already being "
                    "downloaded!  Cancelling.\n");
        }
    }
    /* success will return (PEER_DOWNLOAD_REC *)0 */
    return downloadRec;
}


PEER_DOWNLOAD_REC *nePeerDownloadManager::getDownloadRec(
    unsigned long fileID,
    unsigned long sockfd)
{
    PEER_DOWNLOAD_REC *downloadRec = (PEER_DOWNLOAD_REC *)0;
    std::map< unsigned long, std::map <
        unsigned long, PEER_DOWNLOAD_REC * > >::iterator iter;
    std::map< unsigned long, PEER_DOWNLOAD_REC * > *innerMap = 0;
    std::map< unsigned long, PEER_DOWNLOAD_REC * >::iterator
        innerMapIter;

    /* this action must be completed atomically */
    m_fdIDMapMutex.lock();

    /* first, lookup the socket desc to see if we find anything */
    if ((iter = m_fdIDMap.find(sockfd)) != m_fdIDMap.end())
    {
        innerMap = &((*iter).second);
      
        /* if we found something ... */
        if (innerMap)
        {
            /*
              then lookup the fileID in the inner
              map to get the downloadRec
            */
            if ((innerMapIter = innerMap->find(fileID)) !=
                innerMap->end())
            {
                /* get the download rec pointer */
                downloadRec = (*innerMapIter).second;
            }
        }
    }
    m_fdIDMapMutex.unlock();
    return downloadRec;
}

int nePeerDownloadManager::wasRecentlyRemoved(unsigned long fileID)
{
    int ret = 0;
    struct timeval now;

    std::map< unsigned long, struct timeval >::iterator iter;

    m_fileIDRemovalTimeMapMutex.lock();
    iter = m_fileIDRemovalTimeMap.find(fileID);
    if (iter != m_fileIDRemovalTimeMap.end())
    {
        gettimeofday(&now, NULL);
        if (timercmp(&now,&((*iter).second),<))
        {
            ret = 1;
        }
        else
        {
            /* remove stale fileID to help remedy the
               fileIDs which will hang around for a while */
            m_fileIDRemovalTimeMap.erase(iter);
        }
    }
    m_fileIDRemovalTimeMapMutex.unlock();
    return ret;
}
