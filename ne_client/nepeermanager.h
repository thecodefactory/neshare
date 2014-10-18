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

#ifndef __NEPEERMANAGER_H
#define __NEPEERMANAGER_H

/* NOTE: This class is based on the server's neUserManager class */

class nePeerManager
{
  public:
    nePeerManager(int maxPeers);
    ~nePeerManager();

    /*
      must be called before using any other methods.
      returns 0 on success; 1 on failure
    */
    int initialize();

    /*
      adds a peer to the system; returns 0 on success; 1 on failure
      this class handles deallocation of the peer objects
    */
    int addPeer(nePeer *peer);

    /* 
       remove peer presence from system; if the peerUploadManager
       of peerDownloadManager arguments are not null, all uploads
       and downloads associated with the peer will also be removed.
       returns 0 on success; 1 on failure 
    */
    int removePeer(nePeer *peer,
                   nePeerUploadManager *peerUploadManager,
                   nePeerDownloadManager *peerDownloadManager);

    /* returns the number of peers */
    int getNumPeers();

    /*
      given an ip address, returns a peer pointer if one matches.
      NULL is returned if the matching peer is not found
    */
    nePeer *findPeer(unsigned long ip);

    /*
      resets the internal ptr to the first peer in the list.
      This is used in conjunction with getNextPeer()
    */
    void resetPosition();

    /*
      returns the next peer in list (used for forward iteration loops only);
      returns NULL when there are no more peers
    */
    nePeer *getNextPeer();

    /*
      polls the peer sockets and returns when activity is available.
      returns the number of available reads are ready, or -1 on error.
      if markedPeers is non-null, peers that cause errors will be
      stored in it for later removal.
    */
    int pollPeerSockets(std::vector<nePeer *> *markedPeers);

    /*
      resets the internal ptr to the next peer that has a socket ready.
      This is used in conjunction with getNextPeerSocketReady()
    */
    void resetPeerSocketPosition();

    /*
      returns the next nePeer object that has a message pending for read.
      This is used in conjunction with resetPeerSocketPosition for forward
      iteration loops only.  Returns NULL when there are no more ready peers.
    */
    nePeer *getNextPeerSocketReady();

  private:
    int checkTimeout(nePeer *peer);

    int m_maxPeers;
    int m_peersReady;
    int m_peersNext;
    struct pollfd *m_peers;
    ncMutex m_mutex;

    /*
      this is a vector containing all peers
      connected to the local client
    */
    std::vector< nePeer * > m_peerList;
    std::vector< nePeer * >::iterator m_peerListIter;

    /*
      this is a caching map that will be emptied and filled frequently.
      it stores a temporary mapping from socket descriptors to nePeer
      objects.  the main reason is to keep a link between the ready
      polled sockets and the associated nePeer objects (which contain
      those socket descriptors).
    */
    std::map< int, nePeer * > m_peerCacheMap;
};

#endif /* __NEPEERMANAGER_H */
