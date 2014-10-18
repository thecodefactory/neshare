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

#ifndef __NEUSERMANAGER_H
#define __NEUSERMANAGER_H

/*
  reference counting macros for the user life lock map.
  see the classdocs reference for more information on their usage.
*/
const unsigned long NE_USER_LIFE_DEAD = 0x80000000;
#define UL_ADD_REF(refCount) \
refCount += 1
#define UL_DROP_REF(refCount) \
refCount -= 1 
#define UL_SET_DELETE_BIT(refCount) \
refCount |= NE_USER_LIFE_DEAD
#define UL_CAN_UNLOCK_USER(refCount) \
((refCount & (~NE_USER_LIFE_DEAD)) == 0)
#define UL_CAN_DELETE_USER(refCount) \
(refCount == NE_USER_LIFE_DEAD)

/* valid states that the user managers can be in */
#define NE_UM_UNKNOWN    -1
#define NE_UM_TERMINATED  0
#define NE_UM_RUNNING     1
#define NE_UM_CANCELLED   2

/* this is the generic threaded manager method prototype */
void *commonManagerFunc(void *ptr);


class neUserManager
{
  public:
    neUserManager(int maxUsers);
    ~neUserManager();

    /* returns 0 on success; 1 on failure */
    int initialize(int id, int state);

    /* attempts to gracefully stop the running manager thread */
    void stop();

    /* forcefully stops the running manager thread */
    void cancel();

    /* sets the neServerStatus object to use internally */
    void setServerStatusObj(neServerStatus *serverStatus);

    /*
      adds a user to the system; returns 0 on success; 1 on failure.
      NOTE: this class handles deallocation of the user objects.
    */
    int addUser(neUser *user);

    /*
      finds a user based on the specified criteria.  The ipAddr,
      ctrlPort, and ID are all gathered on the client side from
      a query result message. returns the user on success;
      (neUser *)0 on failure.
    */
    neUser *findUser(unsigned long ipAddr,
                     unsigned long ctrlPort,
                     unsigned long id);
   
    /*
      remove user's presence from system;
      returns 0 on success; 1 on failure
    */
    int removeUser(neUser *user);

    /* returns the number of users */
    unsigned long getNumUsers();

    /* returns the state of the threaded manager */
    int getState();

    /* sets the state of the threaded manager */
    void setState(int state);

    /* returns the id of the threaded manager */
    int getID();

    /*
      resets the internal ptr to the first user in the map.
      This is used in conjunction with getNextUser()
    */
    void resetPosition();

    /*
      returns the next user in map (used for forward iteration loops only);
      returns NULL when there are no more users
    */
    neUser *getNextUser();

    /*
      polls the user sockets and returns when activity is available.
      returns the number of available reads are ready, or -1 on error.
      if a markedUsers vector is passed into pollUserSockets, any users
      that need to be removed will be returned in that vector.
    */
    int pollUserSockets(std::vector<neUser *> *markedUsers = 0);

    /*
      resets the internal ptr to the next user that has a socket ready.
      This is used in conjunction with getNextUserSocketReady()
    */
    void resetUserSocketPosition();

    /*
      returns the next neUser object that has a message pending for read.
      This is used in conjunction with resetUserSocketPosition for forward
      iteration loops only.  Returns NULL when there are no more ready
      users.
    */
    neUser *getNextUserSocketReady();

    /*
      adds an entry to the specified users individual database. filesize
      is in bytes and it limited by the max value of an unsigned long.
      returns 0 on success; 1 on failure.
    */
    int addEntryToUser(neUser *user,
                       char *filename,
                       unsigned long filesize);

    /*
      updates the queryResultManager's keyword map with the entries of
      the given user.  if the g_config.getMinUserSharedFileLimit value
      is non-zero (which is set in the config file, or 0 by default),
      this function will return a value of -1 if the specified user
      does not have enough shared files. this function otherwise
      returns 0 on success and 1 on failure.
    */
    int mergeEntriesToKeywordMap(neUser *user);

    /*
      removes all entries from the user and empties 
      all owned keywords from the queryResultManager's keyword map
    */
    int removeEntriesFromUser(neUser *user);

    /*
      performs a search query based on the query and
      sends the results to targetUser
    */
    int performSearchQuery(neUserSearchQuery *query,
                           neUser *targetUser);

    /*
      locks a user - meaning that the user cannot be removed
      until the matching unlockUserLife call is made.  This keeps
      the user alive even when removeUser is called.
    */
    void lockUserLife(neUser *user);

    /*
      unlocks a user that was previously locked - will remove
      the user if a removal is pending
    */
    void unlockUserLife(neUser *user);

  private:
    /*
      given a user, checks to see if it's time to send either a ping or
      a status message.  if so, the message is sent, otherwise, the
      user's ping and/or status time is incremented by the specified
      delay. returns 1 on a send error; returns 0 on success
    */
    int checkPingAndStatus(neUser *user);

    /*
      given a user, checks to see if the user has timed out.  if so,
      this method returns 1; otherwise, returns 0 and decrements the
      time-to-live (ttl) by the specified delay
    */
    int checkTimeout(neUser *user);

    int m_maxUsers;
    int m_listenableUsers;
    int m_usersReady;
    int m_usersNext;
    int m_state;
    int m_id;
    struct pollfd *m_users;
    ncThread m_thread;
    ncMutex m_mutex;

    /* maps a user to a userfilelist */
    std::map< neUser *,neUserFileList > m_userMap;
    std::map< neUser *,neUserFileList >::iterator m_userMapPtr;

    /*
      this is a caching map that will be emptied and filled frequently.
      it stores a temporary mapping from socket descriptors to neUser
      objects.  the main reason is to keep a link between the ready
      polled sockets and the associated neUser objects (which contain
      those socket descriptors).
    */
    std::map< int, neUser * > m_userCacheMap;

    neServerStatus *m_serverStatus;

    /*
      a reference counting mechanism for preserving user life
      in a given thread, even if destroyed by another thread.
      see classdocs reference for more information.
    */
    std::map< neUser *, unsigned long > m_userLockLifeMap;
};

#endif /* __NEUSERMANAGER_H */
