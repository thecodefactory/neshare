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

#ifndef __NEPEER_H
#define __NEPEER_H

class nePeer
{
  public:
    nePeer(ncSocket *sock, int ttl);
    ~nePeer();

    bool operator< (nePeer &peer);
    bool operator== (nePeer &peer);

    /* returns a socket pointer on success; NULL on error */
    ncSocket *getSocket();

    /* used only for synchronizing network events between the
       neShareClientThreads::processPeers
       method and the neClientConnection object. */
    void setSyncVal(unsigned long syncVal);
    unsigned long getSyncVal();

    /* returns 1 if the ttl is 0 */
    int isTTLExpired();

    /* resets the ttl to it's initial value (from constructor) */
    void resetTTL();

    /*
      sets the removed flag -- for internal use only by the
      peer manager.  see neclientutils.cpp
    */
    void setRemoved();

    /* returns 1 if the peer was removed already by the peer manager */
    int wasRemoved();

    /*
      increments an internal counter for tracking network
      related error occurences for this particular peer
    */
    void incrementErrorCount();

    /* returns the number of error occurences accumulated */
    int getErrorCount();

  private:
    int m_errorCount;
    int m_originalTTL;
    int m_removed;
    struct timeval m_ttl;
    ncSocket *m_sock;
    unsigned long m_syncVal;
};

#endif /* __NEPEER_H */
