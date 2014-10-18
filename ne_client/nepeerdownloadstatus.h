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

#ifndef __NEPEERDOWNLOADSTATUS_H
#define __NEPEERDOWNLOADSTATUS_H

/*
  This class is used for keeping track of simultaneous downloads
  that are happening in the nePeerDownloadManager class.  It is
  used internally to the nePeerDownloadManager class only, however
  it is exported (via callback) to a registered callback with 
  the nePeerDownloadManager.
*/

typedef struct
{
    unsigned long fileID;
    unsigned long maxBlockSize;
    nePeer *peer;
    char *buf;
    unsigned long buflen;
    FILE *fd;
    char *filename;
    struct timeval nextCallbackTime;
    unsigned char md5checksum[16];
} PEER_DOWNLOAD_REC;

typedef struct
{
    PEER_DOWNLOAD_REC *downloadRec;
    unsigned long currentBytes;
    unsigned long totalBytes;
    struct timeval startTime;
    struct timeval currentTime;
    char filename[NE_MSG_MAX_DATA_LEN];
    unsigned long complete;
    unsigned long error;
    unsigned long cancelled;
} PEER_DOWNLOAD_STATUS;

class nePeerDownloadStatus
{
  public:
    nePeerDownloadStatus();
    ~nePeerDownloadStatus();

    /*
      adds a download record to internal bookkeeping;
      returns 0 on success; 1 on failure
    */
    int addDownloadStatusObj(PEER_DOWNLOAD_REC *downloadRec,
                             unsigned long totalBytes,
                             char *filename);

    /*
      updates the status of the object matching the
      specified downloadRec;  returns 0 on success;
      1 on failure
    */
    int updateStatus(PEER_DOWNLOAD_REC *downloadRec);

    /*
      removes the status object matching the
      specified downloadRec
    */
    void removeStatusObj(PEER_DOWNLOAD_REC *downloadRec);

    /* 
       sets a flag specifying that the download is complete.
       This is useful, but is essentially the same feature
       as checking if the currentBytes == totalBytes
    */
    void setComplete(PEER_DOWNLOAD_REC *downloadRec);

    /*
      sets the starting position of the download 
      explicitly. This is useful for resumes
    */
    void setPosition(PEER_DOWNLOAD_REC *downloadRec,
                     unsigned long position);

    /*
      sets a flag in the PEER_DOWNLOAD_STATUS object
      associated with the specified downloadRec to
      indicate that a cancellation has occured.
    */
    void setCancelled(PEER_DOWNLOAD_REC *downloadRec);

    /*
      sets a flag in the PEER_DOWNLOAD_STATUS object
      associated with the specified downloadRec to
      indicate that an error has occured.
    */
    void setError(PEER_DOWNLOAD_REC *downloadRec);

    /* resets the internal status object pointer */
    void resetStatusObjPtr();

    /*
      returns the next PEER_DOWNLOAD_STATUS object.
      This method should be used in conjunction with
      resetStatusObjPtr in a forward iteration loop.
    */
    PEER_DOWNLOAD_STATUS *getNextDownloadStatus();

  private:
    ncMutex m_mutex;
    std::vector<PEER_DOWNLOAD_STATUS *> m_statusObjs;
    std::vector<PEER_DOWNLOAD_STATUS *> m_statusObjsCache;
    std::vector<PEER_DOWNLOAD_STATUS *>::iterator m_statusObjIter;
};

#endif /* __NEPEERDOWNLOADSTATUS_H */
