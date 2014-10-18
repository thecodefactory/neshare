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

extern int errno;

nePeerUploadManager::nePeerUploadManager(neConfig *config,
                                         neFileManager *fileManager)
{
   m_id = 0;
   m_callbackTimeDelay = 1000;
   m_nextCallbackTime.tv_sec = 0;
   m_nextCallbackTime.tv_usec = 0;
   m_config = config;
   m_fnptr = (neUploadCallback)0;
   m_fileManager = fileManager;
   m_uploads.clear();
   m_finishedUploads.clear();
   m_peerUploadStates.clear();
}

nePeerUploadManager::~nePeerUploadManager()
{
   /* note: all threads using the nePeerUploadManager
      must be stopped before calling this destructor. */

   /* de-allocate any remaining uploadRec objects
      and any memory internal to them ; also
      remove them from the internal upload list */
   PEER_UPLOAD_REC *uploadRec = (PEER_UPLOAD_REC *)0;
   std::vector<PEER_UPLOAD_REC *>::iterator iter = m_uploads.begin();

   for(iter = m_uploads.begin(); iter != m_uploads.end(); iter++)
   {
      uploadRec = (*iter);

      fclose(uploadRec->fd);
      free(uploadRec->buf);
      free(uploadRec);      
   }
   m_uploads.clear();
   m_finishedUploads.clear();
   m_peerUploadStates.clear();
}

int nePeerUploadManager::getNumUploads()
{
   int numUploads = 0;
   m_mutex.lock();
   numUploads = m_uploads.size();
   m_mutex.unlock();
   return numUploads;
}

int nePeerUploadManager::addPeerUpload(nePeer *peer,
                                       char *filename,
                                       unsigned long position)
{
   int ret = 1;
   char realFilename[NE_MSG_MAX_DATA_LEN];
   unsigned char md5checksum[16];
   struct stat statbuf;
   PEER_UPLOAD_STATE uploadState;

   memset(&statbuf,0,sizeof(statbuf));
   memset(realFilename,0,NE_MSG_MAX_DATA_LEN*sizeof(char));
   memset(md5checksum,0,16*sizeof(unsigned char));

   /* make sure the file requested is valid */
   if (verifyValidUploadFile(peer,filename,
                             (char *)realFilename,&statbuf) == 0)
   {
      neUtils::computeQuickMD5Checksum(realFilename,md5checksum);
   
      FILE *fd = (FILE *)0;
      if ((fd = fopen(realFilename,"r")) != NULL)
      {
         /* allocate and prepare a new uploadRec object */
         PEER_UPLOAD_REC *uploadRec = (PEER_UPLOAD_REC *)
            malloc(sizeof(PEER_UPLOAD_REC));
         assert(uploadRec);
         memset(uploadRec,0,sizeof(PEER_UPLOAD_REC));
      
         uploadRec->fd = fd;
         uploadRec->fileID = m_id++;
         uploadRec->maxBlockSize = /*m_config->getMaxBlockSize()*/4096;
         uploadRec->peer = peer;
         uploadRec->buf = (char *)malloc(
             uploadRec->maxBlockSize*sizeof(char));
         uploadRec->buflen = 0;
         uploadRec->bytesRemaining = (statbuf.st_size - position);
         memcpy(uploadRec->filename,filename,
                MIN(strlen(filename),NE_MSG_MAX_DATA_LEN-1));
         assert(uploadRec->buf);

         if (fseek(uploadRec->fd,(long)position,SEEK_SET) == -1)
         {
            eprintf("nePeerUploadManager::sendResumeAck | fseek "
                    "(pos = %lu) failed. errno = %x\n",position,errno);
         }
         else
         {
            ret = sendAck(uploadRec,md5checksum,position,
                          &statbuf,filename,(char *)realFilename);
         }
         if (ret)
         {
            eprintf("nePeerUploadManager::addPeerUpload | "
                    "filerequest/resume ack send failed.\n");
            fclose(uploadRec->fd);
            free(uploadRec->buf);
            free(uploadRec);
         }
         else
         {
            /* store an upload state object */
            uploadState.uploadRec = uploadRec;
            uploadState.state = PEER_UPLOAD_OK;
            m_peerUploadStateMutex.lock();
            m_peerUploadStates.push_back(uploadState);
            m_peerUploadStateMutex.unlock();
         }
      }
      else
      {
         eprintf("nePeerUploadManager::addPeerUpload | Local file "
                 "%s cannot be opened.\n",realFilename);
      }
   }
   else
   {
      nemsgFileRequestAck fileRequestAckMsg(peer->getSocket());
      ret = fileRequestAckMsg.send(INVALID_FILE_REQUEST,
                                   (unsigned long)0,(unsigned long)0,
                                   md5checksum,(char *)0);
   }
   return ret;
}

void nePeerUploadManager::sendPeerData()
{
   PEER_UPLOAD_REC *uploadRec = (PEER_UPLOAD_REC *)0;
   nemsgFileDataSend fileDataSendMsg((ncSocket *)0);
   std::vector<PEER_UPLOAD_REC *>::iterator iter = m_uploads.begin();
   std::vector<PEER_UPLOAD_STATE>::iterator siter;
   std::map<unsigned long,PEER_UPLOAD_REC *>::iterator finishedIter;

   /* this action must be completed atomically */
   m_mutex.lock();
   
   /* for each uploadRec we know about */
   for(iter = m_uploads.begin(); iter != m_uploads.end(); iter++)
   {
      /* get the uploadRec */
      uploadRec = (*iter);
      if (!uploadRec)
      {
         eprintf("nePeerUploadManager::sendPeerData | FIXME: found "
                 "an invalid uploadRec! Skipping.\n");
         continue;
      }
      /* and read some file data */
      if ((uploadRec->buflen = fread(uploadRec->buf,
                                     sizeof(char),
                                     uploadRec->maxBlockSize,
                                     uploadRec->fd)))
      {
         /* if read is ok, package it in a message and send it */
         fileDataSendMsg.setSocket(uploadRec->peer->getSocket());
         if (fileDataSendMsg.send(uploadRec->fileID,
                                  uploadRec->buflen,
                                  uploadRec->buf) != 0)
         {
            /* otherwise, increment the errorCount */
            eprintf("nePeerUploadManager::sendPeerData | data send "
                    "failed (sock %d)\n",
                    uploadRec->peer->getSocket()->getSockfd());
            uploadRec->errorCount++;

            /* NOTE: if a peer send fails, it probably isn't connected */
            uploadRec->peer->incrementErrorCount();

            /*
              and update the status object (if any)
              with the error flag set
            */
            if (m_fnptr)
            {
               m_peerUploadStatus.setError(uploadRec);
               m_peerUploadStatus.updateStatus(uploadRec);
            }
         }
         else
         {
            uploadRec->bytesRemaining -= uploadRec->buflen;

            /* then update the status object (if any) */
            if (m_fnptr)
            {
               if (uploadRec->bytesRemaining == 0)
               {
                  m_peerUploadStatus.setComplete(uploadRec);
               }
               m_peerUploadStatus.updateStatus(uploadRec);
            }

            /* and reset the errorCount */
            uploadRec->errorCount = 0;
         }
      }
      else if ((uploadRec->buflen != uploadRec->maxBlockSize) &&
               (uploadRec->buflen != (unsigned long)-1))
      {
         /*
           if we think we've reached the end, set the complete flag on the
           uploadStatus object (if any) and update it for the last time
         */
         if (m_fnptr)
         {
            if (uploadRec->bytesRemaining == 0)
            {
               m_peerUploadStatus.setComplete(uploadRec);
            }
            else
            {
               m_peerUploadStatus.setError(uploadRec);
            }
            m_peerUploadStatus.updateStatus(uploadRec);

            callRegisteredCallback(1);
         }

         /* if at the end of the file, cleanup */
         if (feof(uploadRec->fd))
         {
            iprintf("nePeerUploadManager::sendPeerData | Sending "
                    "SendEnd on fileID %lu\n",uploadRec->fileID);

            nemsgFileDataSendEnd fileDataSendEndMsg(
                uploadRec->peer->getSocket());
            if (fileDataSendEndMsg.send(uploadRec->fileID))
            {
               eprintf("nePeerUploadManager::sendPeerData | File "
                       "data send end message send failed.\n");
               eprintf("nePeerUploadManager::sendPeerData | "
                       "removing upload file ID %lu.\n",
                       uploadRec->fileID);
               uploadRec->peer->incrementErrorCount();
            }

            /* mark the current uploadRec as finished
               (add to finished list for later removal) */
            m_finishedMutex.lock();
            m_finishedUploads[uploadRec->fileID] = uploadRec;
            m_finishedMutex.unlock();
            iprintf("nePeerUploadManager::sendPeerData | File "
                    "send complete\n");
            continue;
         }
         else if (ferror(uploadRec->fd))
         {
            eprintf("File read error on fileID %lu.  Trying "
                    "to recover.\n",uploadRec->fileID);
            uploadRec->errorCount++;
            continue;
         }
      }
      else
      {
         eprintf("nePeerUploadManager::sendPeerData | file read failed "
                 "on fd %d (errno = %d)\n",uploadRec->fd,errno);
         uploadRec->errorCount++;

         /* update the status object (if any) with the error flag set */
         if (m_fnptr)
         {
            m_peerUploadStatus.setError(uploadRec);
            m_peerUploadStatus.updateStatus(uploadRec);
         }
      }
      /* 
         if the send failed, we will skip it and try again the
         next round until we've tried a few times (at which point
         we just give up and remove the upload forcefully).
      */
      if (uploadRec->errorCount == MAX_RESEND_ON_ERROR_COUNT)
      {
         /* mark the current uploadRec as finished
            (add to finished list for later removal) */
         m_finishedMutex.lock();
         m_finishedUploads[uploadRec->fileID] = uploadRec;
         m_finishedMutex.unlock();
         iprintf("nePeerUploadManager::sendPeerData | "
                 "MAX_RESEND_ON_ERROR_COUNT maxed out.\n");
         iprintf("nePeerUploadManager::sendPeerData | File "
                 "upload Stopped.\n");

         /*
           and update the status object (if any)
           with the error flag set
         */
         if (m_fnptr)
         {
            m_peerUploadStatus.setError(uploadRec);
            m_peerUploadStatus.updateStatus(uploadRec);
            callRegisteredCallback(1);
         }
         continue;
      }
   }
   m_mutex.unlock();

   /*
     set all marked cancelled uploadStatus objects as
     cancelled and mark them for deletion; then clear the
     marked cancelled list
   */
   m_peerUploadStateMutex.lock();
   for(siter = m_peerUploadStates.begin(); 
       siter != m_peerUploadStates.end(); siter++)
   {
      if ((*siter).state == PEER_UPLOAD_CANCELLED)
      {
         m_finishedMutex.lock();
         m_finishedUploads[(*siter).uploadRec->fileID] =
             (*siter).uploadRec;
         m_finishedMutex.unlock();
         m_peerUploadStatus.setCancelled((*siter).uploadRec);
      }
   }
   m_peerUploadStateMutex.unlock();

   /*
     call the registered callback (if any) if it's time
     with the newly updated status info objects.
   */
   if (m_fnptr)
   {
      callRegisteredCallback((!m_finishedUploads.empty() ? 1 : 0));
   }

   /* now remove all finished uploadRecs (and statusObjs if any) */
   m_finishedMutex.lock();
   for(finishedIter = m_finishedUploads.begin(); 
       finishedIter != m_finishedUploads.end(); finishedIter++)
   {
      uploadRec = (*finishedIter).second;
      removePeerUpload(uploadRec->peer,uploadRec->fileID);
   }
   m_finishedUploads.clear();
   m_finishedMutex.unlock();
}

int nePeerUploadManager::removeAllPeerUploads(nePeer *peer)
{
   int ret = 0;
   PEER_UPLOAD_REC *uploadRec = (PEER_UPLOAD_REC *)0;
   std::vector<PEER_UPLOAD_REC *>::iterator iter;
   std::map<unsigned long,PEER_UPLOAD_REC *>::iterator finishedIter;

   /* this action must be completed atomically */
   m_mutex.lock();

   /* mark every upload that matches the specified peer */
   for(iter = m_uploads.begin(); iter != m_uploads.end(); iter++)
   {
      uploadRec = (*iter);
      if (uploadRec->peer == peer)
      {
         iprintf("nePeerUploadManager::removeAllPeerUploads | "
                 "Marked fileID %lu\n",uploadRec->fileID);

         m_finishedMutex.lock();
         m_finishedUploads[uploadRec->fileID] = uploadRec;
         m_finishedMutex.unlock();

         /* also, set the error on the status object for a last
            time callback before we destroy the uploadRec objects */
         if (m_fnptr)
         {
            m_peerUploadStatus.setError(uploadRec);
         }
      }
   }
   m_mutex.unlock();

   /* and now remove them one at a time */
   m_finishedMutex.lock();
   for(finishedIter = m_finishedUploads.begin(); 
       finishedIter != m_finishedUploads.end(); finishedIter++)
   {
      uploadRec = (*finishedIter).second;
      ret += removePeerUpload(uploadRec->peer,uploadRec->fileID);
   }
   m_finishedUploads.clear();
   m_finishedMutex.unlock();
   return ret;
}


int nePeerUploadManager::removePeerUpload(nePeer *peer,
                                          unsigned long fileID)
{
   int ret = 1;
   PEER_UPLOAD_REC *uploadRec = (PEER_UPLOAD_REC *)0;
   std::vector<PEER_UPLOAD_REC *>::iterator iter;
   std::vector<PEER_UPLOAD_STATE>::iterator siter;

   /* this action must be completed atomically */
   m_mutex.lock();

   iprintf("nePeerUploadManager::removePeerUpload | called to remove "
           "fileID %lu\n",fileID);
   
   /* first, locate the corresponding uploadRec in the upload list */
   for(iter = m_uploads.begin(); iter != m_uploads.end(); iter++)
   {
      if (((*iter)->peer == peer) && ((*iter)->fileID == fileID))
      {
         break;
      }
   }
   if (iter != m_uploads.end())
   {
      uploadRec = (*iter);

      /* remove the associated uploadState object */
      m_peerUploadStateMutex.lock();
      for(siter = m_peerUploadStates.begin(); 
           siter != m_peerUploadStates.end(); siter++)
      {
         if ((*siter).uploadRec == uploadRec)
         {
            m_peerUploadStates.erase(siter);
            break;
         }
      }
      m_peerUploadStateMutex.unlock();

      /* now remove the uploadRec iterator from the list */
      m_uploads.erase(iter);
      
      /*
        finally, de-allocate and uninitialize the 
        uploadRec (and statusObj if any)
      */
      if (m_fnptr)
      {
         m_peerUploadStatus.removeStatusObj(uploadRec);
      }
      fclose(uploadRec->fd);
      free(uploadRec->buf);
      free(uploadRec);
      ret = 0;
   }
   else
   {
      eprintf("nePeerUploadManager::removePeerUpload | "
              "FIXME: peer not found.\n");
   }
   m_mutex.unlock();
   return ret;
}

int nePeerUploadManager::cancelPeerUpload(nePeer *peer,
                                          unsigned long fileID,
                                          int sendCancelMsg)
{
   int ret = 1;
   std::vector<PEER_UPLOAD_STATE>::iterator siter;

   /* locate the associated uploadState object (if any) */
   m_peerUploadStateMutex.lock();
   for(siter = m_peerUploadStates.begin(); 
        siter != m_peerUploadStates.end(); siter++)
   {
      if (((*siter).uploadRec->peer == peer) &&
          ((*siter).uploadRec->fileID == fileID))
      {
         /* if found, set the cancelled flag */
         (*siter).state = PEER_UPLOAD_CANCELLED;
         m_peerUploadStateMutex.unlock();

         if (sendCancelMsg)
         {
            /*
              if requested, send a cancellation message
              to the peer who is currently downloading
            */
            nemsgFileDataCancel fileDataCancelMsg(peer->getSocket());
            if (fileDataCancelMsg.send(fileID) == 0)
            {
               peer->getSocket()->flush();
            }
         }
         ret = 0;
      }
   }
   if (ret)
   {
      m_peerUploadStateMutex.unlock();
   }
   return ret;
}


void nePeerUploadManager::registerCallback(neUploadCallback fnptr)
{
   iprintf("nePeerUploadManager::registerCallback | callback registered.\n");
   m_fnptr = fnptr;
}

void nePeerUploadManager::setCallbackDelay(unsigned long millisecs)
{
   m_callbackTimeDelay = millisecs;
}


void nePeerUploadManager::callRegisteredCallback(int force)
{
   struct timeval now;
   gettimeofday(&now,0);
   if (force || (timercmp(&now,&m_nextCallbackTime,>)))
   {
      /* set the next callback time */
      m_nextCallbackTime.tv_sec = now.tv_sec + (m_callbackTimeDelay/1000);
      m_nextCallbackTime.tv_usec = now.tv_usec + (m_callbackTimeDelay%1000);
      
      m_peerUploadStatus.resetStatusObjPtr();
      (*m_fnptr)(&m_peerUploadStatus);
   }
}

int nePeerUploadManager::isFileBeingUploaded(nePeer *peer, char *filename)
{
   int ret = 0;
   int fnameLen1 = 0;
   int fnameLen2 = strlen(filename);
   std::vector<PEER_UPLOAD_REC *>::iterator iter;

   assert(peer);
   assert(filename);

   m_mutex.lock();
   for(iter = m_uploads.begin(); iter != m_uploads.end(); iter++)
   {
      if ((*iter)->peer == peer)
      {
         fnameLen1 = strlen((*iter)->filename);
         if ((fnameLen1 == fnameLen2) &&
             (memcmp((*iter)->filename,filename,fnameLen1)) == 0)
         {
            ret = 1;
            break;
         }
      }
   }
   m_mutex.unlock();
   return ret;
}

int nePeerUploadManager::verifyValidUploadFile(nePeer *peer, 
                                               char *filename,
                                               char *realFilename,
                                               struct stat *statbuf)
{
   int ret = 1;

   assert(peer);
   assert(filename);
   assert(realFilename);
   assert(statbuf);

   /* make sure the file isn't being 
      downloaded already by the same peer */
   if (isFileBeingUploaded(peer,filename) == 0)
   {
      /* make sure the filename is valid file
         according to the filemanager */
      if (m_fileManager->lookupEncodedName(filename,
                                           realFilename,
                                           NE_MSG_MAX_DATA_LEN) == 0)
      {
         /* finally, make sure the file exists locally */
         if (stat(realFilename,statbuf) != -1)
         {
            ret = 0;
         }
      }
      else
      {
         iprintf("nePeerUploadManager::verifyValidUploadFile | "
                 "Invalid file requested: %s\n",filename);
      }
   }
   return ret;
}

int nePeerUploadManager::sendRequestAck(PEER_UPLOAD_REC *uploadRec,
                                        unsigned char *md5checksum,
                                        struct stat *statbuf,
                                        char *filename,
                                        char *realFilename)
{
   int ret = 1;
   assert(uploadRec);
   assert(md5checksum);
   assert(statbuf);
   assert(filename);

   nemsgFileRequestAck fileRequestAckMsg(uploadRec->peer->getSocket());

   if (fileRequestAckMsg.send(uploadRec->fileID,
                              uploadRec->maxBlockSize,
                              (unsigned long)statbuf->st_size,
                              md5checksum,
                              filename) == 0)
   {
      /* add the new uploadRec to the current upload list */
      m_mutex.lock();
      m_uploads.push_back(uploadRec);
      m_mutex.unlock();
      
      /* check for a registered callback */
      if (m_fnptr)
      {
         if (m_peerUploadStatus.addUploadStatusObj(uploadRec,
                                                   statbuf->st_size,
                                                   realFilename))
         {
            iprintf("nePeerUploadManager::sendRequestAck | "
                    "Ignoring status object.\n");
         }
      }
      uploadRec->peer->getSocket()->flush();
      ret = 0;
   }
   return ret;
}

int nePeerUploadManager::sendResumeAck(PEER_UPLOAD_REC *uploadRec,
                                       unsigned char *md5checksum,
                                       unsigned long position,
                                       struct stat *statbuf,
                                       char *filename,
                                       char *realFilename)
{
   int ret = 1;
   assert(uploadRec);
   assert(md5checksum);
   assert(statbuf);
   assert(filename);

   nemsgFileResumeAck fileResumeAckMsg(uploadRec->peer->getSocket());
   if (fileResumeAckMsg.send(uploadRec->fileID,
                             uploadRec->maxBlockSize,
                             (unsigned long)statbuf->st_size,
                             md5checksum,
                             filename) == 0)
   {
      /* add the new uploadRec to the current upload list */
      m_mutex.lock();
      m_uploads.push_back(uploadRec);
      m_mutex.unlock();
      
      /* check for a registered callback */
      if (m_fnptr)
      {
         if (m_peerUploadStatus.addUploadStatusObj(
                 uploadRec,
                 (unsigned long)statbuf->st_size,
                 realFilename))
         {
            iprintf("nePeerUploadManager::sendResumeAck | "
                    "Ignoring status object.\n");
         }
         else
         {
            m_peerUploadStatus.setPosition(uploadRec,position);
         }
      }
      uploadRec->peer->getSocket()->flush();
      ret = 0;
   }
   return ret;
}


int nePeerUploadManager::sendAck(PEER_UPLOAD_REC *uploadRec,
                                 unsigned char *md5checksum,
                                 unsigned long position,
                                 struct stat *statbuf,
                                 char *filename,
                                 char *realFilename)
{
   int ret = 1;

   /* send the appropriate ack message based on incoming args */
   if (position)
   {
      ret = sendResumeAck(uploadRec,
                          (unsigned char *)md5checksum,
                          position,
                          statbuf,
                          filename,
                          realFilename);
      if (ret)
      {
         nemsgFileResumeAck fileResumeAckMsg(uploadRec->peer->getSocket());
         fileResumeAckMsg.send(INVALID_FILE_REQUEST,
                               (unsigned long)0,(unsigned long)0,
                               (unsigned char *)INVALID_MD5_CHECKSUM,
                               (char *)0);
      }
   }
   else
   {
      ret = sendRequestAck(uploadRec,
                           (unsigned char *)md5checksum,
                           statbuf,
                           filename,
                           realFilename);
      if (ret)
      {
         nemsgFileRequestAck fileRequestAckMsg(uploadRec->peer->getSocket());
         fileRequestAckMsg.send(INVALID_FILE_REQUEST,
                                (unsigned long)0,(unsigned long)0,
                                (unsigned char *)INVALID_MD5_CHECKSUM,
                                (char *)0);
      }
   }
   return ret;
}
