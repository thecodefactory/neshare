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

#ifndef __NEUSER_H
#define __NEUSER_H

class neUser
{
  public:
    neUser(neUserManager *userManager);
    ~neUser();

    bool operator< (neUser &user);
    bool operator== (neUser &user);
   
    /*
      initializes the user object with a socket and a connection
      speed, as well as a time to live (in seconds).  firewallStatus
      should be 0 if the user is NOT behind a firewall; non-zero otherwise.
      returns 0 on success; 1 on error
    */
    int initialize(ncSocket *sock, 
                   int connection, 
                   int ttl, 
                   int firewallStatus,
                   int controlPort);
   
    /* returns a socket pointer on success; NULL on error */
    ncSocket *getSocket();
   
    /*
      the use of the timerstamps (for determining if it's time to send
      a ping message or a status message) was added so that a separate
      thread can be removed which primarily dealt with determining this
      information.  It's cleaner from that perspective, but requires this
      user class to be a little smarter than it should be.  The
      time-to-live (ttl) is handled in a similar manner.
    */

    /* returns 1 if the ttl is 0 */
    int isTTLExpired();

    /* resets the ttl to it's initial value (from initialize call) */
    void resetTTL();

    /* returns 1 if a ping message should be sent; 0 otherwise */
    int isPingTime();

    /* returns 1 if a status message should be sent; 0 otherwise */
    int isStatusTime();

    /*
      returns 1 if we're disconnecting (and thus should not
      recieve any more incoming messages); 0 otherwise
    */
    int isDisconnecting();

    /* returns 1 if the user's filelist is valid. 0 otherwise */
    int isFileListValid();

    /* sets the user's filelist to be invalid */
    void setFileListInvalid();

    /*
      sets the amount of time that must have elapsed in order
      for isPingTime and isStatusTime to return 1
    */
    void setPingTime(int seconds);
    void setStatusTime(int seconds);

    /*
      sets a flag which should be checked meaning that we
      should not be able to handle any more messages
    */
    void setDisconnecting();

    /*
      sets the threadedHandlerAffinity value.
      see nethreadedhandler.h for a better description.
    */
    void setAffinityValue(int val);

    /* returns the connection speed of the user */
    int getConnectionSpeed();

    /*
      returns non-zero if the user if behind a firewall.
      returns 0 if the user is NOT behind a firewall
    */
    int getFirewallStatus();

    /*
      returns the port on which to connect to this user for
      control requests (such as file transfer, etc).
      if the value is 0, it usually indicates the user is
      behind a firewall
    */
    int getControlPort();

    /*
      increments an internal counter for tracking network
      related error occurences for this particular user
    */
    void incrementErrorCount();

    /* returns the number of error occurences accumulated */
    int getErrorCount();

    /*
      returns the threadedHandlerAffinity value, or -1
      if there was none assigned.
      see nethreadedhandler.h for a better description
    */
    int getAffinityValue();

    /*
      locks this user's write lock so that if used properly,
      no two threads can be writing socket data to this
      user simultaneously
    */
    void lockWriteLock();

    /* unlocks this user's write lock */
    void unlockWriteLock();

    /*
      returns a pointer to the assigned userManager passed
      in to the constructor at user creation time
    */
    neUserManager *getUserManager();

  private:
    int m_connection;
    int m_firewalled;
    int m_controlPort;
    int m_errorCount;
    int m_originalTTL;
    int m_affinityValue;
    int m_disconnecting;
    int m_validFileList;
    struct timeval m_ttl;
    struct timeval m_pingTime;
    struct timeval m_statusTime;
    ncMutex m_mutex;
    ncMutex m_writeLock;
    ncSocket *m_sock;
    neUserManager *m_userManager;
};

#endif /* __NEUSER_H */
