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
extern unsigned long numNewUserManagerManagers;
extern unsigned long numDeletedUserManagerManagers;
extern unsigned long totalBytesAllocated;
extern unsigned long totalBytesFreed;
#endif

#include "neserverheaders.h"

extern neConfig g_config;

neUserManagerManager::neUserManagerManager()
{
    m_numManagers = 0;
    m_threadedManagers.clear();

#ifdef NESHARE_DEBUG
    numNewUserManagerManagers++;
    totalBytesAllocated += sizeof(neUserManagerManager);
#endif
}

neUserManagerManager::~neUserManagerManager()
{
    cancelManagers();
    m_numManagers = 0;

#ifdef NESHARE_DEBUG
    numDeletedUserManagerManagers++;
    totalBytesFreed += sizeof(neUserManagerManager);
#endif
}

int neUserManagerManager::startManagers(int numManagers)
{
    neUserManager *userManager = (neUserManager *)0;

    int maxUsers = (g_config.getMaxNumUsers() /
                    g_config.getNumUserManagers());

    if (numManagers)
    {
        for(int i = 0; i < numManagers; i++)
        {
            userManager = new neUserManager(maxUsers);
            if (userManager)
            {
                if (userManager->initialize(i,NE_UM_RUNNING))
                {
                    iprintf("neUserManagerManager::startManagers "
                            "| failed to start user manager %d\n",i);
                    iprintf("neUserManagerManager::startManagers | "
                            "Aborting.\n");
                    break;
                }
                else
                {
                    iprintf("neUserManagerManager::startManagers "
                            "     | %d OK (maxUsers = %d)\n",
                            i,maxUsers);

                    /* give all user managers the same status obj */
                    userManager->setServerStatusObj(&m_serverStatus);
                    m_threadedManagers.push_back(userManager);
                    assert(userManager == m_threadedManagers[i]);
                }
            }
            else
            {
                iprintf("neUserManagerManager::startManagers | Cannot "
                        "allocate user manager %d\n",i);
                iprintf("neUserManagerManager::startManagers | "
                        "Aborting.\n");
                break;
            }
        }
    }
    m_numManagers = m_threadedManagers.size();
    return m_numManagers;
}

int neUserManagerManager::addUserToUserManager(int managerIndex,
                                               neUser *user)
{
    int ret = 1;
    neUserManager *userManager = (neUserManager *)0;

    if (((managerIndex > -1) && (managerIndex < m_numManagers)) &&
        user)
    {
        userManager = m_threadedManagers[managerIndex];
        assert(userManager);

        ret = userManager->addUser(user);
    }
    return ret;
}

void neUserManagerManager::stopManagers()
{
    std::vector<neUserManager *>::iterator iter;
    neUserManager *userManager = (neUserManager *)0;

    for(iter = m_threadedManagers.begin();
        iter != m_threadedManagers.end(); iter++)
    {
        userManager = (*iter);
        assert(userManager);

        userManager->stop();
        ncSleep(100);
    }
}

void neUserManagerManager::cancelManagers()
{
    std::vector<neUserManager *>::iterator iter;
    neUserManager *userManager = (neUserManager *)0;

    for(iter = m_threadedManagers.begin();
        iter != m_threadedManagers.end(); iter++)
    {
        userManager = (*iter);
        assert(userManager);

        userManager->cancel();
        delete userManager;
        userManager = (neUserManager *)0;
    }
    m_threadedManagers.clear();
}

void neUserManagerManager::removeUserFromUserManager(int managerIndex,
                                                     neUser *user)
{
    neUserManager *userManager = (neUserManager *)0;
    
    assert(user);
    assert(user->getSocket());
    assert((managerIndex > -1) && (managerIndex < m_numManagers));

    userManager = m_threadedManagers[managerIndex];
    assert(userManager);

    userManager->removeUser(user);
}

neUserManager *neUserManagerManager::getUserManager(int managerIndex)
{
    neUserManager *userManager = (neUserManager *)0;

    if ((managerIndex > -1) && (managerIndex < m_numManagers))
    {
        userManager = m_threadedManagers[managerIndex];
        assert(userManager);
    }
    return userManager;
}
