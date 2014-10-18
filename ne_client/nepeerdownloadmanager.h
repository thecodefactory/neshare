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

#ifndef __NEPEERDOWNLOADMANAGER_H
#define __NEPEERDOWNLOADMANAGER_H

/* 
   this class is used for handling simultaneous downloads from
   other peers.  The key management is done using the fileID
   fields that each file request ack provided us.  Using this unique
   ID, we keep all incoming data straight and are able to output
   the proper data to the proper output files.

   Note: The fileIDs are only unique for a single peer.  Peer 1 and
   Peer 2 can have the same fileID and this class can handle that fine.
   However, if Peer 1 has two different downloads going simultaneously,
   both with the same fileID, we have no way to tell the data apart and
   thus are doomed to mix the file data.  Avoiding this is the purpose of
   this class and it is dependent only on peers assigning unique fileIDs
   for simultaneous downloads. -N.M.
*/

typedef void *(*neDownloadCallback)(nePeerDownloadStatus *status);

class nePeerDownloadManager
{
  public:
    nePeerDownloadManager();
    ~nePeerDownloadManager();

    /* sets the configuration object */
    void setConfig(neConfig *config);

    /* 
       given a fileID, a maxBlockSize, and an nePeer ptr,
       this method allocates all necessary book keeping info
       to handle downloading this particular file data from this
       peer.
   
       this is done by keeping a mapping between a calculated peerID
       (which is the sum of the fileID and the peer ptr address) to
       the information needed.
    */
    int addPeerDownload(unsigned long fileID, 
                        unsigned long maxBlockSize,
                        unsigned long filesize,
                        unsigned char md5checksum[16],
                        nePeer *peer,
                        char *filename);

    /* given a peer, retrieve the next chunk of file data
       and store it to the proper file */
    int getFileDataSend (nePeer *peer);

    /* given a peer, retrieve the fileID and close the
       file descriptor associated with it - then remove the
       peerDownload kept in internal mappings */
    int endFileDataSend(nePeer *peer);

    /*
      given a peer ip address and a filename, this
      call sets the cancelled flag and sends the cancel
      if sendCancelMsg arg is non-zero, a cancellation network
      message will be sent to the connected peer who is uploading.
      message. returns 0 on success; 1 otherwise.
    */
    int cancelPeerDownload(unsigned long ipAddr,
                           char *filename,
                           int sendCancelMsg = 0);
      
    /*
      given a peer ip address and a fileID, this
      call sets the cancelled flag and sends the cancel
      if sendCancelMsg arg is non-zero, a cancellation network
      message will be sent to the connected peer who is uploading.
      message. returns 0 on success; 1 otherwise.
    */
    int cancelPeerDownload(unsigned long ipAddr,
                           unsigned long fileID,
                           int sendCancelMsg = 0);
      
    /* removes all downloadRecs with the cancelled flag set */
    void removeCancelledDownloads();

    /*
      removes a peer download keep in internal mappings.
      if the cancellation argument is non-zero, the cancelled flag
      in the associated nePeerDownloadStatus object (if any) will be
      set before calling the status callback for the last time
      with the peer's downloadRec before deletion.
      returns 0 if the upload was removed properly, 1 otherwise.
    */
    int removePeerDownload(nePeer *peer, unsigned long fileID);

    /*
      removes all downloads in progress associated with the specified
      peer.  This is useful for a forceful disconnect of a peer
      at a higher level.  returns 0 on success; a non-zero value
      indicating the number of failed peer removals otherwise.
    */
    int removeAllPeerDownloads(nePeer *peer);

    /*
      returns the size of the file if the given filename exists 
      in the current download directory already.  returns 0 otherwise
    */
    unsigned long fileExistsLocally(char *filename);

    /*
      registers a call back to be called after a chunk has been
      downloaded.  The function passed in must return before any
      more data can be uploaded.
    */
    void registerCallback(neDownloadCallback fnptr);

    /* forces this class to only call the registered
       callback once at each millisecs interval */
    void setCallbackDelay(unsigned long millisecs);

  private:
    void callRegisteredCallback(PEER_DOWNLOAD_REC *downloadRec,
                                int force);
    PEER_DOWNLOAD_REC *isFileBeingDownloaded(unsigned long fileID,
                                             int socket,
                                             char *filename);

    PEER_DOWNLOAD_REC *getDownloadRec(unsigned long fileID, 
                                      unsigned long ipAddr);

    int wasRecentlyRemoved(unsigned long fileID);

    neDownloadCallback m_fnptr;
    neConfig *m_config;
    nePeerDownloadStatus m_peerDownloadStatus;
    unsigned long m_callbackTimeDelay;

    /* 
       this map is used internally to map a socket descriptor to a map
       which maps the fileID to the PEER_DOWNLOAD_REC object.
       If there are any collisions here, either the fileID we were
       given was genuinely the same as a previous one by the same peer
       (thus a breach of protocol). Note the reason socket descriptors
       are used as an initial mapping is because they are unique for
       each peer - unlike Ip addresses necessarily since two connected
       peers may possibly have the same Ip address (as in the case of
       firewalled users).
    */
    std::map< unsigned long, std::map <
        unsigned long, PEER_DOWNLOAD_REC * > > m_fdIDMap;
    ncMutex m_fdIDMapMutex;

    /*
      this map is used for caching the filenames of all downloads
      currently in progress.  This is useful for quickly determining
      if a given filename is already in progress (which is illegal)
      so that we can stop it before allowing it to start.
    */
    std::map< std::string, PEER_DOWNLOAD_REC * > m_filenameCacheMap;
    ncMutex m_filenameCacheMutex;

    /*
      this map keeps track of recently finished download records.
      the only time it should be consulted is when we receive file
      data and we cannot find the matching download record.  We then
      look in this list for the fileID to see if it was recently
      terminated.  In the normal case, this is invalid, so ignoring
      it is OK.  This is mostly useful in the case of cancellations
      which were removed from the download manager but data is still
      incoming for a while until the other peer stops sending it.
      After a fileID is in this list for 10 seconds, it is no longer
      valid upon consultation.  0 return value means that the fileID
      was recently removed, 1 means the fileID was not recently
      removed.
      FIXME: It's possible that two different peers could have the same
      fileID (but the worst case scenario is that after the expiration
      time, the situation remedies itself)
    */
    std::map< unsigned long, struct timeval > m_fileIDRemovalTimeMap;
    ncMutex m_fileIDRemovalTimeMapMutex;
};

#endif /* __NEPEERDOWNLOADMANAGER_H */
