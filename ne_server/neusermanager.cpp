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

#ifdef NESHARE_DEBUG
extern unsigned long numNewUserManagers;
extern unsigned long numDeletedUserManagers;
extern unsigned long totalBytesAllocated;
extern unsigned long totalBytesFreed;
#endif

#include "neserverheaders.h"

extern neConfig g_config;
extern neQueryResultManager *g_queryResultManager;
extern neDiskMemAllocator g_userFileAllocator;

neUserManager::neUserManager(int maxUsers)
{
    m_maxUsers = maxUsers;
    m_listenableUsers = 0;
    m_usersReady = 0;
    m_usersNext = 0;
    m_users = (struct pollfd *)0;
    m_state = 0;
    m_id = -1;
    m_serverStatus = (neServerStatus *)0;

    assert(m_userMap.empty());
    assert(m_userCacheMap.empty());
    assert(m_userLockLifeMap.empty());

#ifdef NESHARE_DEBUG
    numNewUserManagers++;
    totalBytesAllocated += sizeof(neUserManager);
#endif
}

neUserManager::~neUserManager()
{
    cancel();

    m_maxUsers = 0;
    m_listenableUsers = 0;
    m_usersReady = 0;
    m_usersNext = 0;
    m_state = 0;
    m_id = -1;
    m_serverStatus = (neServerStatus *)0;

    /* clear the cache map */
    m_userCacheMap.clear();

    /* delete any remaining users and their lists */
    std::map<neUser *,neUserFileList >::iterator iter;
    neUser *user = (neUser *)0;
    neUserFileList *userList = (neUserFileList *)0;

    for(iter = m_userMap.begin(); iter != m_userMap.end(); iter++)
    {
        user = ((*iter).first);
        userList = &((*iter).second);

        delete user;
        user = (neUser *)0;

        userList->empty();
        userList = (neUserFileList *)0;
    }
    m_userMap.clear();

    /* free the pollfd structures */
    if (m_users)
    {
#ifdef NESHARE_DEBUG
        totalBytesFreed += sizeof(m_maxUsers*sizeof(struct pollfd));
#endif
        free(m_users);
        m_users = (struct pollfd *)0;
    }

    /* clear other maps */
    m_userLockLifeMap.clear();

#ifdef NESHARE_DEBUG
    numDeletedUserManagers++;
    totalBytesFreed += sizeof(neUserManager);
#endif
}

int neUserManager::initialize(int id, int state)
{
    int ret = 1;

    m_id = id;
    m_state = state;

    m_users = (struct pollfd *)malloc(m_maxUsers*sizeof(struct pollfd));
    if (m_users)
    {
#ifdef NESHARE_DEBUG
        totalBytesAllocated += sizeof(m_maxUsers*sizeof(struct pollfd));
#endif
        for(int i = 0; i < m_maxUsers; i++)
        {
            m_users[i].fd = -1;
            m_users[i].events = 0;
            m_users[i].revents = 0;
        }

        if (m_thread.start(commonManagerFunc,(void *)this) == NC_OK)
        {
            if (m_thread.detach() == NC_OK)
            {
                ret = 0;
            }
            else
            {
                m_thread.stop(0);
                free(m_users);
                m_users = (struct pollfd *)0;
            }
        }
        else
        {
            free(m_users);
            m_users = (struct pollfd *)0;
        }
    }
    else
    {
        eprintf("Cannot allocate user manager poll structures!\n");
    }
    return ret;
}

void neUserManager::stop()
{
    dprintf("neUserManager::stop          | stopping    "
            "(id = %d)\n",m_id);
    m_state = NE_UM_CANCELLED;
}

void neUserManager::cancel()
{
    if (m_state != NE_UM_TERMINATED)
    {
        dprintf("neUserManager::cancel        | terminating "
                "(id = %d)\n",m_id);

        m_thread.stop(0);
        m_state = NE_UM_TERMINATED;
    }
}

void neUserManager::setServerStatusObj(neServerStatus *serverStatus)
{
    if (serverStatus)
    {
        m_serverStatus = serverStatus;
    }
}


int neUserManager::addUser(neUser *user)
{
    int ret = 1;
    int numUsers = 0;

    assert(user);
    assert(user->getSocket());

    /* this action must be completed atomically */
    m_mutex.lock();
    numUsers = m_userMap.size();
   
    /* check to see if the max number of users are connected */
    if (numUsers < m_maxUsers)
    {
        /* make sure user is not already in map */
        if (m_userMap.find(user) == m_userMap.end())
        {
            /* set the new user up with a new blank file list */

            /* and add the user to the user map */
            m_userMap[user] = neUserFileList();

            /* set the ping and status times */
            user->setPingTime(g_config.getUserPingDelay());
            user->setStatusTime(g_config.getUserStatusDelay());

            /* finally, update the status variables */
            m_serverStatus->incNumUsers(1);
            ret = 0;
        }
        else
        {
            iprintf("neUserManager::addUser | User already in list. "
                    "Skipping.\n");
        }
    }
    else
    {
        iprintf("User Rejected\n (numUsers = %d | maxUsers = %d)\n",
                numUsers,m_maxUsers);
    }
    m_mutex.unlock();
    return ret;
}

neUser *neUserManager::findUser(unsigned long ipAddr,
                                unsigned long ctrlPort,
                                unsigned long id)
{
    neUser *user = (neUser *)0;
    std::map<neUser *,neUserFileList >::iterator iter;

    m_mutex.lock();
    for(iter = m_userMap.begin(); iter != m_userMap.end(); iter++)
    {
        if (((*iter).first->getSocket()->getSockfd() == (int)id) &&
            ((*iter).first->getSocket()->getIpAddr() == ipAddr) &&
            ((*iter).first->getControlPort() == (int)ctrlPort))
        {
            user = (*iter).first;
            break;
        }
    }
    m_mutex.unlock();
    return user;
}


int neUserManager::removeUser(neUser *user)
{
    int ret = 1;
    neUserFileList *userFileList = (neUserFileList *)0;
    std::map< neUser *, unsigned long >::iterator lifeLockIter;

    assert(user);
    assert(user->getSocket());

    /*
      invalidate the user's file list since we're removing it.
      this allows a thread that's sending userfile results to
      know that the result gathered from this user should not
      be sent.

      FIXME: protocol should be updated to also send the number
      of results *after* all results are sent, so the client will
      know that the count is short (if this occurs) ??
    */
    user->setFileListInvalid();

    /*
      first, remove all entries from the user 
      (i.e. reclaim all resources the user consumes)
    */
    ret = removeEntriesFromUser(user);

    /* this action must be completed atomically */
    m_mutex.lock();

    /* first, find the user in the map and get the associated file list */
    std::map<neUser *,neUserFileList >::iterator iter = m_userMap.find(user);
    if (iter != m_userMap.end())
    {
        /* decrement the status variable tracking number of users */
        m_serverStatus->decNumUsers(1);

        assert(user == (*iter).first);
        userFileList = &((*iter).second);

        std::map<int,neUser *>::iterator cacheMapIter = m_userCacheMap.find(
            user->getSocket()->getSockfd());

        /* if the user was found in the cache map, remove it */
        if (cacheMapIter != m_userCacheMap.end())
        {
            m_userCacheMap.erase(cacheMapIter);
        }

        /* remove the user from the user map */
        m_userMap.erase(iter);

        /* delete neUserFileList object */
        userFileList->empty();
        userFileList = (neUserFileList *)0;

        /* only free the user if it's not life locked currently */
        lifeLockIter = m_userLockLifeMap.find(user);
        if (lifeLockIter == m_userLockLifeMap.end())
        {
            dprintf("neUserManager::removeUser | deleting user %x\n",user);
            delete user;
            user = (neUser *)0;
        }
        else
        {
            /*
              if life locked, set the deletion bit in the
              reference count for removal on unlock
            */
            dprintf("neUserManager::removeUser | preserving "
                    "removed user %x\n",user);
            UL_SET_DELETE_BIT((*lifeLockIter).second);
        }
        ret = 0;
    }
    m_mutex.unlock();
    return ret;
}

unsigned long neUserManager::getNumUsers()
{
    return (m_serverStatus ? m_serverStatus->getNumUsers() : 0);
}

int neUserManager::getID()
{
    return m_id;
}

int neUserManager::getState()
{
    return m_state;
}

void neUserManager::setState(int state)
{
    m_state = state;
}

void neUserManager::resetPosition()
{
    m_mutex.lock();
    m_userMapPtr = m_userMap.begin();
    m_mutex.unlock();
}

neUser *neUserManager::getNextUser()
{
    neUser *user = (neUser *)0;
    m_mutex.lock();
    std::map<neUser *,neUserFileList >::iterator iter = m_userMapPtr++;
    user = ((iter == m_userMap.end()) ? (neUser *)0 : (*iter).first);
    m_mutex.unlock();
    return user;
}

int neUserManager::pollUserSockets(std::vector<neUser *> *markedUsers)
{
    int ret = 0;
    int index = 0;
    int numUsers = 0;
    int readyUsers = 0;
    int pollTime = 25;
    neUser *user = (neUser *)0;
    std::map<neUser *,neUserFileList >::iterator iter;
    std::map<int, neUser *>::iterator cacheIter;

    /* this action must be completed atomically */
    m_mutex.lock();

    numUsers = m_userMap.size();
    if (numUsers > 0)
    {
        /*
          clear the poll events and remove users that had errors
          on the previous call to poll (if any)
        */
        for(int i = 0; i < m_maxUsers; i++)
        {
            if (markedUsers &&
                ((m_users[i].fd != -1) &&
                 (m_users[i].revents & POLLERR) ||
                 (m_users[i].revents & POLLHUP) ||
                 (m_users[i].revents & POLLNVAL)))
            {
                cacheIter = m_userCacheMap.find(m_users[i].fd);
                if (cacheIter != m_userCacheMap.end())
                {
                    m_mutex.unlock();
                    lockUserLife((*cacheIter).second);
                    m_mutex.lock();
                    markedUsers->push_back((*cacheIter).second);
                }
            }
            m_users[i].fd = -1;
            m_users[i].events = 0;
            m_users[i].revents = 0;
        }

        /* clear the cache map */
        m_userCacheMap.clear();

        for(iter = m_userMap.begin(); iter != m_userMap.end(); iter++)
        {
            user = (*iter).first;
            assert(user);
            assert(user->getSocket());

            /*
              if we were passed a valid markedUsers vector, we need
              to fill it with users that need to be removed (if any)
              so that the caller can remove them later.
            */
            if (markedUsers)
            {
                /* first, check and make sure the user hasn't timed out */
                if (checkTimeout(user))
                {
                    dprintf("neUserManager::pollUserSockets | "
                            "Marking User %x (ttl expired).\n",user);

                    /*
                      lock user life here, but be sure to unlock
                      on markedUser deletion (in neShareServerThreads)
                    */
                    m_mutex.unlock();
                    lockUserLife(user);
                    m_mutex.lock();
                    markedUsers->push_back(user);
                    continue;
                }

                /*
                  check if a ping or status message should
                  be sent to the client
                */
                if (checkPingAndStatus(user))
                {
                    dprintf("neUserManager::pollUserSockets | "
                            "Marking User %x (ping/status send failed).\n",
                            user);

                    /*
                      lock user life here, but be sure to unlock
                      on markedUser deletion (in neShareServerThreads)
                    */
                    m_mutex.unlock();
                    lockUserLife(user);
                    m_mutex.lock();
                    markedUsers->push_back(user);
                    continue;
                }
            }

            /*
              place the user's socket in the pollfd
              array, and set the read event
            */
            m_users[index].fd = user->getSocket()->getSockfd();
            m_users[index].events = POLLIN;

            /* update the cache map */
            m_userCacheMap[m_users[index].fd] = user;
            index++;
        }
        readyUsers = (int)m_userCacheMap.size();
        m_mutex.unlock();

        /* poll with a small fixed timeout */
        if (readyUsers)
        {
            ret = poll(m_users,index,pollTime);

            /*
              ret cannot be higher than number
              of elements in cache map
            */
            assert((readyUsers + 1) >  ret);
        }
    }
    else
    {
        m_mutex.unlock();
    }

    /*
      this must be assigned for the
      getNextUserSocketReady method to work
    */
    m_usersReady = ret;
    return ret;
}

void neUserManager::resetUserSocketPosition()
{
    m_usersNext = 0;
}

neUser *neUserManager::getNextUserSocketReady()
{
    int index = 0;
    neUser *user = (neUser *)0;
    std::map< int, neUser * >::iterator iter;

    /*
      scan for the next ready socket by looking
      only at the ones with non-zero revent flags
    */
    while(m_usersReady && (m_usersNext < m_maxUsers))
    {
        if (m_users[m_usersNext].revents & POLLIN)
        {
            /* determine the index into the cache map */
            index = m_users[m_usersNext].fd;

            /*
              index into the cache map to get the
              neUser object (using socket fd)
            */
            iter = m_userCacheMap.find(index);

            /* if it's found, get the neUser object ptr to return */
            if (iter != m_userCacheMap.end())
            {
                user = (*iter).second;

                /* decrement total number of ready users */
                m_usersReady--;
            }
        }

        /* increment to next position for next call */
        m_usersNext++;
    }
    return user;
}

int neUserManager::addEntryToUser(neUser *user,
                                  char *filename, 
                                  unsigned long filesize)
{
    int ret = 1;
    neUserFile *userFile = (neUserFile *)0;
    neUserFileList *userFileList = (neUserFileList *)0;
    void *newUserFilePtr = (void *)0;

    assert(user);
    assert(user->getSocket());
    assert(filename);

    /* a blank entry isn't an error; but doesn't do anything */
    if (strlen(filename) == 0)
    {
        return 0;
    }

    /* 
       first, construct the userFile (entry) and parse out keywords.
       the neUserFile objects are freed by the neUserFileList class.

       NOTE: neUserFile objects are actually stored and executed
       on disk to preserve server memory; thus the g_userFileAllocator
       allocation in conjunction with the use of 'in place new'.
    */
    newUserFilePtr = g_userFileAllocator.alloc();
    if (!newUserFilePtr)
    {
        return ret;
    }

    userFile = new (newUserFilePtr) neUserFile(user);
    //userFile = new neUserFile(user);
    if (!userFile || userFile->initialize(filename, filesize))
    {
        eprintf("neUserManager::addEntryToUser(%s,%d) failed.\n",
                filename,filesize);
        userFile->~neUserFile();
        g_userFileAllocator.free(userFile);
        //delete userFile;
        userFile = (neUserFile *)0;
        return ret;
    }

    /* get the userfilelist associated with this user */
    m_mutex.lock();
    std::map<neUser *,neUserFileList >::iterator iter = m_userMap.find(user);
    if (iter != m_userMap.end())
    {
        userFileList = &((*iter).second);
        assert(userFileList);
    }
    m_mutex.unlock();

    /* add the entry (neUserFile) to the users entry list */
    if (userFileList && userFileList->addUserFile(userFile) == 0)
    {
        /* update the status variables */
        int filesizeInMBytes = BYTES_TO_MBYTES(filesize);
        m_serverStatus->incNumUserFiles(1);
        m_serverStatus->incTotalSizeofFilesInMB
            (((filesizeInMBytes == 0) ? 1 : filesizeInMBytes));
        ret = 0;
    }
    else
    {
        eprintf("neUserManager::addEntriesToUser | User %x not "
                "found in list\n",user);
        userFile->~neUserFile();
        g_userFileAllocator.free(userFile);
        //delete userFile;
        userFile = (neUserFile *)0;
    }
    return ret;
}

int neUserManager::mergeEntriesToKeywordMap(neUser *user)
{
    int ret = 1;
    neUserFileList *userFileList = (neUserFileList *)0;

    m_mutex.lock();

    assert(user);
    assert(user->getSocket());

    /* get the userfilelist associated with this user */
    std::map<neUser *,neUserFileList >::iterator iter = m_userMap.find(user);
    if (iter != m_userMap.end())
    {
        assert((*iter).first == user);
        userFileList = &((*iter).second);
        assert(userFileList);
    }

    if (user && userFileList)
    {
        /*
          check to see if the user is sharing enough files (based on 
          the current size of their shared file list)
        */
        if (g_config.getMinUserSharedFileLimit() && 
            (userFileList->getTotalSizeofUserFiles() < 
             (unsigned long)g_config.getMinUserSharedFileLimit()))
        {
            ret = -1;
        }
        else
        {
            /* add entries to the queryManager's keyword Map */
            ret = g_queryResultManager->mergeEntriesToKeywordMap
                (user,userFileList);
        }
    }
    m_mutex.unlock();
    return ret;
}

int neUserManager::removeEntriesFromUser(neUser *user)
{
    int ret = 1;
    neUserFileList *userFileList = (neUserFileList *)0;

    /* this action must be completed atomically */
    m_mutex.lock();

    assert(user);
    assert(user->getSocket());

    /* first, find the user and their userList in the map */
    std::map<neUser *,neUserFileList >::iterator iter = m_userMap.find(user);
    if (iter != m_userMap.end())
    {
        userFileList = &((*iter).second);
        assert(userFileList);

        dprintf("neUserManager::removeEntriesFromUser | UserFileList of "
                "user %x has %d elements.\n",user,
                userFileList->getNumUserFiles());

        /* then update the status variables related to the user file list */
        m_serverStatus->decNumUserFiles(userFileList->getNumUserFiles());
        m_serverStatus->decTotalSizeofFilesInMB
            (userFileList->getTotalSizeofUserFiles());

        /* remove all of the user entries from the keyword map */
        if (g_queryResultManager->removeEntriesFromUser(user,
                                                        userFileList) == 0)
        {
            /* finally, empty the neUserFileList associated with the user */
            userFileList->empty();

            dprintf("neUserManager::removeEntriesFromUser | UserFileList "
                    "of user %x has %d elements.\n",user,
                    userFileList->getNumUserFiles());
            ret = 0;
        }
    }
    m_mutex.unlock();
    return ret;
}

int neUserManager::performSearchQuery(neUserSearchQuery *query,
                                      neUser *targetUser)
{
    int ret = 1;
    neQueryResult *result = (neQueryResult *)0;

    if (query && (query->isValidQuery()))
    {
        result = g_queryResultManager->performSearchQuery(query,targetUser);
        ret = g_queryResultManager->sendQueryResults(result,targetUser);
        delete result;
    }
    return ret;
}

void neUserManager::lockUserLife(neUser *user)
{
    std::map< neUser *, unsigned long >::iterator iter;

    m_mutex.lock();
    assert(user);
    assert(user->getSocket());

    iter = m_userLockLifeMap.find(user);
    if (iter == m_userLockLifeMap.end())
    {
        /* add user to life lock map with initial ref count */
        m_userLockLifeMap[user] = 1;
    }
    else
    {
        /* increment the reference count on this lock */
        UL_ADD_REF((*iter).second);
    }
    m_mutex.unlock();
}

void neUserManager::unlockUserLife(neUser *user)
{
    std::map< neUser *, unsigned long >::iterator iter;

    m_mutex.lock();
    assert(user);
    assert(user->getSocket());

    iter = m_userLockLifeMap.find(user);
    if (iter != m_userLockLifeMap.end())
    {
        assert(user == (*iter).first);

        /* decrement the reference count on this lock */
        UL_DROP_REF((*iter).second);

        /*
          if the user has been removed while locked,
          free the user now
        */
        if (UL_CAN_DELETE_USER((*iter).second))
        {
            /* remove the user from the lockLife map */
            m_userLockLifeMap.erase(iter);

            dprintf("neUserManager::unlockUserLife | "
                    "deleting user %x\n",user);
            delete user;
            user = (neUser *)0;
        }
        else if (UL_CAN_UNLOCK_USER((*iter).second))
        {
            /* remove the user from the lockLife map */
            m_userLockLifeMap.erase(iter);
        }
    }
    else
    {
        eprintf("neUserManager::unlockUserLife | "
                "FIXME: Locked user no longer present!\n");
    }
    m_mutex.unlock();
}

int neUserManager::checkPingAndStatus(neUser *user)
{
    int ret = 0;
    nemsgPing pingMsg((ncSocket *)0);
    nemsgStatus statusMsg((ncSocket *)0);
    static int pingTime = g_config.getUserPingDelay();
    static int statusTime = g_config.getUserStatusDelay();

    assert(user);
    assert(user->getSocket());

    /* check if a ping message should be sent to the user */
    if (user->isPingTime())
    {
        pingMsg.setSocket(user->getSocket());
        user->lockWriteLock();
        if (pingMsg.send())
        {
            iprintf("Removing User %x (ping send failed).\n",user);
            ret = 1;
        }
        else
        {
            user->setPingTime(pingTime);

            /* check if a status message should be sent to the user */
            if (user->isStatusTime())
            {
                statusMsg.setSocket(user->getSocket());
                if (statusMsg.send
                    (m_serverStatus->getNumUsers(),
                     m_serverStatus->getNumUserFiles(),
                     m_serverStatus->getTotalSizeofFilesInMB()))
                {
                    iprintf("Removing User %x (status send failed).\n",user);
                    ret = 1;
                }
                else
                {
                    user->setStatusTime(statusTime);
                }
            }
        }
        user->unlockWriteLock();
    }
    return ret;
}

int neUserManager::checkTimeout(neUser *user)
{
    assert(user);
    assert(user->getSocket());
    return (user->isTTLExpired());
}

void *commonManagerFunc(void *ptr)
{
    int umid = 0;

    neUserManager *userManager = (neUserManager *)ptr;
    if (userManager)
    {
        umid = userManager->getID();
        dprintf("commonManagerFunc | threadedManager %d started.\n",umid);

        /* this will not return until server exit */
        neShareServerThreads::processUsers(userManager);

        userManager->setState(NE_UM_TERMINATED);
    }
    return (void *)0;
}
