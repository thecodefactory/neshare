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

nePeerManager::nePeerManager(int maxPeers)
{
    m_maxPeers = maxPeers;
    m_peers = (struct pollfd *)0;
    m_peersReady = 0;
    m_peersNext = 0;
    m_peerCacheMap.clear();
}

nePeerManager::~nePeerManager()
{
    /* clear the cache map */
    m_peerCacheMap.clear();

    /*
      somewhere, we need to disconnect from 
      other connected peers gracefully ??
    */


    /* delete any remaining peers */
    nePeer *peer = (nePeer *)0;
    std::vector<nePeer *>::iterator iter;
    for(iter = m_peerList.begin(); iter != m_peerList.end(); iter++)
    {
        peer = (*iter);
        delete peer;
    }
    m_peerList.clear();

    /* free the pollfd structures */
    if (m_peers)
    {
        free(m_peers);
    }
}

int nePeerManager::initialize()
{
    int ret = 1;

    m_peers = (struct pollfd *)malloc(m_maxPeers*sizeof(struct pollfd));
    if (m_peers)
    {
        for(int i = 0; i < m_maxPeers; i++)
        {
            m_peers[i].fd = -1;
            m_peers[i].events = 0;
            m_peers[i].revents = 0;
        }
        ret = 0;
    }
    return ret;
}

int nePeerManager::addPeer(nePeer *peer)
{
    int ret = 1;
    int numPeers = 0;
  
    /* this action must be completed atomically */
    m_mutex.lock();

    numPeers = m_peerList.size();
 
    /* check to see if the max number of peers are connected */
    if (numPeers < m_maxPeers)
    {
        /* make sure peer is not already in map */
        if (find(m_peerList.begin(),m_peerList.end(),peer) ==
            m_peerList.end())
        {
            m_peerList.push_back(peer);
            ret = 0;
        }
        else
        {
            iprintf("nePeerManager::addPeer | Peer already "
                    "in list. Skipping.\n");
        }
    }
    else
    {
        iprintf("nePeerManager::addPeer | Peer Rejected "
                "(max exceeded)\n");
    }
    m_mutex.unlock();
    return ret;
}

int nePeerManager::removePeer(nePeer *peer,
                              nePeerUploadManager *peerUploadManager,
                              nePeerDownloadManager *peerDownloadManager)
{
    int ret = 0;

    assert(peer);
    assert(peer->getSocket());

    /* tell the peer we're disconnecting */
    nemsgPeerDisconnect peerDisconnectMsg(peer->getSocket());
    peerDisconnectMsg.send();

    /* remove all peer downloads (if any) from this peer */
    if (peerDownloadManager)
    {
        iprintf("nePeerManager::removePeer | Removing all peer "
                "downloads\n");
        ret += peerDownloadManager->removeAllPeerDownloads(peer);
        iprintf("nePeerManager::removePeer | Finished Removing "
                "all peer downloads\n");
    }

    /* remove all peer uploads (if any) from this peer */
    if (peerUploadManager)
    {
        iprintf("nePeerManager::removePeer | Removing all peer "
                "uploads\n");
        ret += peerUploadManager->removeAllPeerUploads(peer);
        iprintf("nePeerManager::removePeer | Finished Removing "
                "all peer uploads\n");
    }

    /* this action must be completed atomically */
    m_mutex.lock();

    /* first, find the peer in the map */
    std::vector<nePeer *>::iterator iter =
        find(m_peerList.begin(),m_peerList.end(),peer);
    if (iter != m_peerList.end())
    {
        /* remove the peer from the peer list */
        m_peerList.erase(iter);

        /* and further, de-allocate the peer object */
        delete peer;
        ret = ((ret == 0) ? 0 : 1);
    }
    m_mutex.unlock();
    return ret;
}

int nePeerManager::getNumPeers()
{
    int numPeers = 0;
    m_mutex.lock();
    numPeers = m_peerList.size();
    m_mutex.unlock();
    return numPeers;
}

void nePeerManager::resetPosition()
{
    m_mutex.lock();
    m_peerListIter = m_peerList.begin();
    m_mutex.unlock();
}

nePeer *nePeerManager::getNextPeer()
{
    nePeer *peer = (nePeer *)0;
    m_mutex.lock();
    peer = ((m_peerListIter == m_peerList.end()) ?
            (nePeer *)0 :
            (*m_peerListIter)++);
    m_mutex.unlock();
    return peer;
}

int nePeerManager::pollPeerSockets(std::vector<nePeer *> *markedPeers)
{
    int ret = 0;
    int numPeers = 0;
    int index = 0;
    int pollTime = 100;
    int readyPeers = 0;
    std::vector<nePeer *>::iterator iter;
    std::map<int, nePeer *>::iterator cacheIter;
    nePeer *peer = (nePeer *)0;

    /* this action must be completed atomically */
    m_mutex.lock();

    numPeers = m_peerList.size();

    if (numPeers > 0)
    {
        /*
          clear the poll events and remove peers that had errors
          on the previous call to poll (if any)
        */
        for(int i = 0; i < m_maxPeers; i++)
        {
            if (markedPeers &&
                ((m_peers[i].fd != -1) &&
                 (m_peers[i].revents & POLLERR) ||
                 (m_peers[i].revents & POLLHUP) ||
                 (m_peers[i].revents & POLLNVAL)))
            {
                cacheIter = m_peerCacheMap.find(m_peers[i].fd);
                if (cacheIter != m_peerCacheMap.end())
                {
                    iprintf("nePeerManager::pollPeerSockets | "
                            "Marking Peer %x (poll failed).\n",peer);
                    markedPeers->push_back((*cacheIter).second);
                }
            }
            m_peers[i].fd = -1;
            m_peers[i].events = 0;
            m_peers[i].revents = 0;
        }
      
        /* empty the cache map */
        m_peerCacheMap.clear();

        for(iter = m_peerList.begin(); iter != m_peerList.end(); iter++)
        {
            peer = (*iter);
            assert(peer);
            assert(peer->getSocket());

            if (markedPeers)
            {
                /* remove peers that have timed out */
                if (checkTimeout(peer))
                {
                    iprintf("nePeerManager::pollPeerSockets | "
                            "Marking Peer %x (ttl expired).\n",peer);
                    markedPeers->push_back(peer);
                    continue;
                }
            }

            /*
              place the peer's socket in the pollfd array,
              and set the read event
            */
            m_peers[index].fd = peer->getSocket()->getSockfd();
            m_peers[index].events = POLLIN;
         
            /* update the cache map */
            m_peerCacheMap[m_peers[index].fd] = peer;
            index++;
        }
        readyPeers = (int)m_peerCacheMap.size();
        m_mutex.unlock();

        /* poll with a small fixed timeout */
        if (readyPeers)
        {
            ret = poll(m_peers,index,pollTime);

            /* ERASEME: FIXME: */
            if (ret)
            {
                assert((m_peers[0].revents != 0) || (m_peers[1].revents != 0));
            }

            /*
              ret cannot be higher than number
              of elements in cache map
            */
            assert((readyPeers + 1) > ret);
        }
    }
    else
    {
        m_mutex.unlock();
    }

    /*
      this must be assigned for the
      getNextPeerSocketReady method to work
    */
    m_peersReady = ret;
    return ret;
}

void nePeerManager::resetPeerSocketPosition()
{
    m_peersNext = 0;
}

nePeer *nePeerManager::getNextPeerSocketReady()
{
    int index = 0;
    nePeer *peer = (nePeer *)0;
    std::map< int, nePeer * >::iterator iter;

    /*
      scan for the next ready socket by looking
      only at the ones with non-zero revent flags
    */
    while(m_peersReady && (m_peersNext < m_maxPeers))
    {
        if (m_peers[m_peersNext].revents & POLLIN)
        {
            /* determine the index into the cache map */
            index = m_peers[m_peersNext].fd;

            /*
              index into the cache map to get the 
              nePeer object (using socket fd)
            */
            iter = m_peerCacheMap.find(index);

            /* if it's found, get the nePeer object ptr to return */
            if (iter != m_peerCacheMap.end())
            {
                peer = (*iter).second;

                /* decrement total number of ready peers */
                m_peersReady--;
            }
        }

        /* increment index to next position for next call */
        m_peersNext++;
    }
    return peer;
}

nePeer *nePeerManager::findPeer(unsigned long ip)
{
    nePeer *peer = (nePeer *)0;
    std::vector<nePeer *>::iterator iter;

    m_mutex.lock();
    for(iter = m_peerList.begin(); iter != m_peerList.end(); iter++)
    {
        if ((*iter)->getSocket()->getIpAddr() == ip)
        {
            peer = *iter;
            break;
        }
    }
    m_mutex.unlock();
    return peer;
}

int nePeerManager::checkTimeout(nePeer *peer)
{
    assert(peer);
    assert(peer->getSocket());
    return (peer->isTTLExpired());
}
