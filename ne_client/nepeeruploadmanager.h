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

#ifndef __NEPEERUPLOADMANAGER_H
#define __NEPEERUPLOADMANAGER_H

/*
  this class is used for handling simultaneous uploads to
  other peers. see the notes in nepeerdownloadmanager.h
  regarding the fileID requirements.  this class takes the
  simplest approach of all - assigning an incrementing number
  that is used across all peers.  A roll over is okay and fits
  the requirements just fine as long as an upload is not started
  by a peer before a roll over; a roll over occurs and loops all
  the way back, and then another upload is started by the peer.
  Seeing as how a roll over will occur roughly every 2^32 uploads
  initiated, this assignment should be okay.

  Note: all this assumes that only one nePeerUploadManager object
  is instantiated -- which had better be the case (grr) -N.M.

  Note: The idea behind the m_finishedUploads map and the 
  m_finishedMutex are not that weird.  Since two threads can
  be building up a list of finished uploads at the same time
  (i.e. through removeAllPeers and sendPeerData), we need to
  make sure that a single upload isn't removed more than once (thus the
  map which guarantees a single fileID to uploadRec object mapping),
  and that a remove is not attempted at the same time (thus the mutex
  protecting the removal).  This works because the first thread locks
  the mutex and removes everything in it (including the fileID to 
  uploadRec mapping) and then unlocks the mutex.  The second thread
  then sees what is left only and removes (or adds) more objects
  and mappings at that point.  -N.M.
*/

/*
  This is currently only used for marking an upload as cancelled
  so that we can remove it at a later time when it's more
  convenient (although cancellations can happen asynchronously).
  Uploads are actually removed from the sendPeerData method.
*/
#define PEER_UPLOAD_OK        0
#define PEER_UPLOAD_CANCELLED 1
typedef struct
{
    PEER_UPLOAD_REC *uploadRec;
    unsigned long state;
} PEER_UPLOAD_STATE;

const unsigned long MAX_RESEND_ON_ERROR_COUNT = 5;

typedef void *(*neUploadCallback)(nePeerUploadStatus *status);

class nePeerUploadManager
{
  public:
    nePeerUploadManager(neConfig *config, neFileManager *fileManager);
    ~nePeerUploadManager();

    /* returns the current number of uploads being handled */
    int getNumUploads();
      
    /* 
       schedules a file upload to be handled.  if position is
       non-zero, the upload manager treats the upload as a resume and
       sends data and meta-data as if the file started at the 
       specified byte position.
    */
    int addPeerUpload(nePeer *peer, char *filename, unsigned long position = 0);
      
    /* 
       removes an upload (usually when finished, cancelled, or
       an error occurs) from the management of this class.
       if the cancellation argument is non-zero, the cancelled flag
       in the associated nePeerUploadStatus object (if any) will be
       set before calling the status callback for the last time
       with the peer's uploadRec before deletion.
       returns 0 if the upload was removed properly, 1 otherwise.
    */
    int removePeerUpload(nePeer *peer, unsigned long fileID);

    /*
      removes all uploads in progress associated with the specified
      peer.  This is useful for a forceful disconnect of a peer
      at a higher level.  returns 0 on success; a non-zero value
      indicating the number of failed peer removals otherwise.
    */
    int removeAllPeerUploads(nePeer *peer);

    /*
      cancels the upload on the specified fileID and peer;
      if sendCancelMsg arg is non-zero, a cancellation network
      message will be sent to the connected peer who is downloading.
      returns 1 if an error occurs or the upload was not found
    */
    int cancelPeerUpload(nePeer *peer,
                         unsigned long fileID,
                         int sendCancelMsg = 0);
      
    /* sends another round of data to all peers that have been added.
       this means that up to maxBlockSize bytes are sent to each
       registered UploadRec that is being managed */
    void sendPeerData();

    /* registers a call back to be called after all chunks have been
       uploaded.  The function passed in must return before any
       more data can be uploaded. */
    void registerCallback(neUploadCallback fnptr);

    /* forces this class to only call the registered callback once
       a number of milliseconds has past. The default is one second. */
    void setCallbackDelay(unsigned long millisecs);

  private:
    void callRegisteredCallback(int force);
    int isFileBeingUploaded(nePeer *peer, char *filename);
    int verifyValidUploadFile(nePeer *peer,
                              char *filename,
                              char *realFilename,
                              struct stat *statbuf);
    int sendAck(PEER_UPLOAD_REC *uploadRec,
                unsigned char *md5checksum,
                unsigned long position,
                struct stat *statbuf,
                char *filename,
                char *realFilename);
    int sendRequestAck(PEER_UPLOAD_REC *uploadRec,
                       unsigned char *md5checksum,
                       struct stat *statbuf,
                       char *filename,
                       char *realFilename);
    int sendResumeAck(PEER_UPLOAD_REC *uploadRec,
                      unsigned char *md5checksum,
                      unsigned long position,
                      struct stat *statbuf,
                      char *filename,
                      char *realFilename);

    unsigned long m_id;
    unsigned long m_callbackTimeDelay;
    struct timeval m_nextCallbackTime;
    neConfig *m_config;
    neUploadCallback m_fnptr;
    nePeerUploadStatus m_peerUploadStatus;
    neFileManager *m_fileManager;
    std::vector<PEER_UPLOAD_REC *> m_uploads;
    std::map< unsigned long, PEER_UPLOAD_REC *> m_finishedUploads;
    ncMutex m_mutex;
    ncMutex m_finishedMutex;
    std::vector<PEER_UPLOAD_STATE> m_peerUploadStates;
    ncMutex m_peerUploadStateMutex;
};

#endif /* __NEPEERUPLOADMANAGER_H */

