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

#ifndef __NEPEERUPLOADSTATUS_H
#define __NEPEERUPLOADSTATUS_H

/*
  This class is used for keeping track of simultaneous uploads
  that are happening in the nePeerUploadManager class.  It is
  used internally to the nePeerUploadManager class only, however
  it is exported (via callback) to a registered callback with 
  the nePeerUploadManager.
*/

/* 
   NOTE: This struct is not *exactly* identical to the
   PEER_DOWNLOAD_REC defined in nepeerdownloadmanager.h.
   This is because the upload will retry a number of times
   before giving up with the entire upload.  we store that
   errorCount in the PEER_UPLOAD_REC to know when to quit.
*/
typedef struct
{
    unsigned long fileID;
    unsigned long maxBlockSize;
    nePeer *peer;
    char *buf;
    unsigned long buflen;
    FILE *fd;
    unsigned long errorCount;
    unsigned long bytesRemaining;
    char filename[NE_MSG_MAX_DATA_LEN];
} PEER_UPLOAD_REC;

typedef struct
{
    PEER_UPLOAD_REC *uploadRec;
    unsigned long currentBytes;
    unsigned long totalBytes;
    struct timeval startTime;
    struct timeval currentTime;
    char filename[NE_MSG_MAX_DATA_LEN];
    unsigned long complete;
    unsigned long error;
    unsigned long cancelled;
} PEER_UPLOAD_STATUS;

class nePeerUploadStatus
{
  public:
    nePeerUploadStatus();
    ~nePeerUploadStatus();

    /*
      adds an upload record to internal bookkeeping;
      returns 0 on success; 1 on failure
    */
    int addUploadStatusObj(PEER_UPLOAD_REC *uploadRec,
                           unsigned long totalBytes,
                           char *filename);

    /*
      updates the status of the object matching the
      specified uploadRec;  returns 0 on success;
      1 on failure
    */
    int updateStatus(PEER_UPLOAD_REC *uploadRec);

    /*
      removes the status object matching the
      specified uploadRec
    */
    void removeStatusObj(PEER_UPLOAD_REC *uploadRec);

    /* 
       sets a flag specifying that the upload is complete.
       This is useful, but is essentially the same feature
       as checking if the currentBytes == totalBytes
    */
    void setComplete(PEER_UPLOAD_REC *uploadRec);

    /*
      sets the starting position of the upload explicitly.
      This is useful for resumes
    */
    void setPosition(PEER_UPLOAD_REC *uploadRec,
                     unsigned long position);

    /*
      sets a flag in the PEER_UPLOAD_STATUS object
      associated with the specified uploadRec to
      indicate that a cancellation has occured.
    */
    void setCancelled(PEER_UPLOAD_REC *uploadRec);

    /*
      sets a flag in the PEER_UPLOAD_STATUS object
      associated with the specified uploadRec to
      indicate that an error has occured.
    */
    void setError(PEER_UPLOAD_REC *uploadRec);

    /* resets the internal status object pointer */
    void resetStatusObjPtr();

    /*
      returns the next PEER_UPLOAD_STATUS object.
      This method should be used in conjunction with
      resetStatusObjPtr in a forward iteration loop.
    */
    PEER_UPLOAD_STATUS *getNextUploadStatus();

  private:
    ncMutex m_mutex;
    std::vector<PEER_UPLOAD_STATUS *> m_statusObjs;
    std::vector<PEER_UPLOAD_STATUS *> m_statusObjsCache;
    std::vector<PEER_UPLOAD_STATUS *>::iterator m_statusObjIter;
};

#endif /* __NEPEERUPLOADSTATUS_H */
