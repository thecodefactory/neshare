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

#include "neserverheaders.h"

extern neConfig g_config;
extern neQueryResultManager *g_queryResultManager;
extern neDiskMemAllocator g_userFileAllocator;

namespace neShareServerThreads
{
    static neUserManagerManager s_userManagerManager;
    static neThreadedHandlerManager s_threadedHandlerManager;

    /* this is called by each neUserManager's initialize method */
    void *processUsers(void *ptr)
    {
        int numReady = 0;
        int curHandlerIndex = 0;
        int threadedHandlerIndex = 0;
        unsigned long msgType = 0;

        neUser *user = (neUser *)0;
        std::vector<neUser *> markedUsers;

        int numMessagesHandled = 0;
        int maxMessagesToHandle = g_config.getMaxMessagesToHandle();

        neUserManager *userManager = (neUserManager *)ptr;
        assert(userManager);

        /*
          try to balance the handlerIndices so all user managers
          won't clobber the first couple of handlers with the
          first few users
        */
        threadedHandlerIndex = (userManager->getID() %
                                g_config.getNumThreadedHandlers());

        while(userManager->getState() == NE_UM_RUNNING)
        {
            numReady = userManager->pollUserSockets(&markedUsers);

            /* remove all users (if any) that need to be removed */
            removeMarkedUsers(&markedUsers);
            assert(markedUsers.empty());

            if (numReady == 0)
            {
                ncSleep(100);
                continue;
            }
            else if (numReady == -1)
            {
                eprintf("neShareServerThreads::processUsers | "
                        "userManager->pollUserSockets failed.\n");
                eprintf("neShareServerThreads::processUsers | "
                        "FIXME: Thread terminating.\n");
                return (void *)0;
            }

            /*
              iterate over each ready socket and do a peek read.  handle it
              according to the message type. NOTE: all ready Users are
              stored temporarily in the neUserManager cache map.
            */
            userManager->resetUserSocketPosition();
            while((user = userManager->getNextUserSocketReady()))
            {
                /* life lock the user to avoid deletion from under us */
                userManager->lockUserLife(user);

                numMessagesHandled = 0;

                /* reset the user's TTL since they have activity */
                user->resetTTL();

                /*
                  determine which threaded handler to place the user in. see
                  nethreadedhandler.h for a description of the affinityvalue.
                */
                curHandlerIndex = user->getAffinityValue();
                if (curHandlerIndex == -1)
                {
                    curHandlerIndex = threadedHandlerIndex++;
                    if (threadedHandlerIndex ==
                        g_config.getNumThreadedHandlers())
                    {
                        threadedHandlerIndex = 0;
                    }
                    user->setAffinityValue(curHandlerIndex);
                }

                do
                {
                    msgType = neUtils::peekMessage(user->getSocket());
                    /*
                      strip the message from the socket without processing it
                      and pass it off to a threadedhandler to be handled later
                    */
                    if ((msgType == 0) ||
                        (neServerUtils::recvRawMessage(
                            &s_threadedHandlerManager,
                            curHandlerIndex,
                            user,msgType)))
                    {
                        dprintf("Ignoring msg type %x on disconnecting "
                                "user %x.\n",msgType,user);
                        user->incrementErrorCount();
                        break;
                    }

                } while(msgType &&
                        (++numMessagesHandled < maxMessagesToHandle));

                userManager->unlockUserLife(user);
            }
        }
        dprintf("Stopping user processing thread.\n");
        return (void *)0;
    }

    void *processLoginMessage(void *ptr)
    {
        ncSocket *newSock = (ncSocket *)ptr;
        if (newSock)
        {
#ifdef NESHARE_DEBUG
            totalBytesAllocated += sizeof(ncSocket);
#endif

            neUserManager *userManager = (neUserManager *)0;
            int curManagerIndex = 0;
            static int userManagerIndex = 0;

            unsigned long newUserConnection = 0;
            unsigned long newUserFirewallStatus = 0;
            unsigned long newUserControlPort = 0;

            /* read the new user information sent in login msg */
            nemsgLogin loginMsg(newSock);
            if (loginMsg.recv(&newUserConnection,
                              &newUserFirewallStatus,
                              &newUserControlPort) == 0)
            {
                curManagerIndex = userManagerIndex++;
                if (userManagerIndex == g_config.getNumUserManagers())
                {
                    userManagerIndex = 0;
                }
                userManager =
                    s_userManagerManager.getUserManager(curManagerIndex);
                if (userManager)
                {
                    neUser *newUser = new neUser(userManager);
                    if (newUser->initialize(newSock,
                                            newUserConnection,
                                            g_config.getUserTTL(),
                                            (int)newUserFirewallStatus,
                                            (int)newUserControlPort))
                    {
                        delete newUser;
                        newUser = (neUser *)0;

                        iprintf("neShareServerThreads::processLoginMessage "
                                "| New user initialization failed. "
                                "Skipping\n");
                        return (void *)0;
                    }

                    /*
                      the user manager handles the deallocation
                      of the neUser objects
                    */
                    if (userManager->addUser(newUser))
                    {
                        /*
                          if the addUser method fails, reject client,
                          (delete it) and return
                        */
                        iprintf("neShareServerThreads::processLoginMessage "
                                "| New user addition failed. Rejecting.\n");
                        neServerUtils::rejectNewUser(newUser);
                        return (void *)0;
                    }

                    /* send a login ack */
                    nemsgLoginAck loginAckMsg(newSock);
                    newUser->lockWriteLock();
                    if (loginAckMsg.send(
                            g_config.getGreetingMessageFile()) == 0)
                    {
                        newSock->flush();
                        newUser->unlockWriteLock();
                        iprintf("New %s User %x Added: Total Count "
                                "is %d.\n",(newUserFirewallStatus ?
                                            "Firewalled" :
                                            "Non-Firewalled"),newUser,
                                userManager->getNumUsers());
                    }
                    else
                    {
                        newUser->unlockWriteLock();
                        iprintf("neShareServerThreads::processLoginMessage "
                                "| Login ack send failed. Rejecting.\n");
                        neServerUtils::rejectNewUser(newUser);
                        return (void *)0;
                    }
                }
                else
                {
#ifdef NESHARE_DEBUG
                    totalBytesFreed += sizeof(ncSocket);
#endif
                    eprintf("FIXME: NULL User manager found!\n");
                    delete newSock;
                }
            }
            else
            {
#ifdef NESHARE_DEBUG
                totalBytesFreed += sizeof(ncSocket);
#endif
                /* drop the connection on read failure */
                delete newSock;
            }
        }
        return (void *)0;
    }
      
    void startThreads()
    {
        /* initialize server structures */
        g_queryResultManager = new neQueryResultManager();
        if (!g_queryResultManager)
        {
            eprintf("Fatal error: Cannot allocate required "
                    "query manager.\n");
            exit(1);
        }

        /* initialize any disk mem allocators */
        if (g_userFileAllocator.initialize(g_config.getUserFilePageFile(),
                                           sizeof(neUserFile),
                                           g_config.getNumReservedUserFiles()))
        {
            eprintf("Fatal error: Cannot initialize disk mem allocator "
                    "%s\n",g_config.getUserFilePageFile());
            exit(1);
        }
      
        /* start up the specified number of threaded user managers */
        int numManagers = s_userManagerManager.startManagers
            (g_config.getNumUserManagers());

        if (numManagers < 1)
        {
            eprintf("Fatal error: Cannot operate server without any "
                    "user managers.  Please make sure the config "
                    "variable NUMUSERMANAGERS is set to 1 or more.\n");
            delete g_queryResultManager;
            exit(1);
        }

        if (numManagers != g_config.getNumUserManagers())
        {
            eprintf("Fatal error: Cannot start all threaded user managers.\n");
            eprintf("Fatal error: %d were OK; please specify a lower value.\n",
                    numManagers);
            delete g_queryResultManager;
            s_userManagerManager.stopManagers();
            exit(1);
        }

        /* start up the specified number of threaded handlers */
        int numHandlers = s_threadedHandlerManager.startHandlers(
            g_config.getNumThreadedHandlers());

        if (numHandlers < 1)
        {
            eprintf("Fatal error: Cannot operate server without any "
                    "message handlers.  Please make sure the config "
                    "variable NUMTHREADEDHANDLERS is set to 1 or more.\n");
            delete g_queryResultManager;
            s_userManagerManager.stopManagers();
            exit(1);
        }

        if (numHandlers != g_config.getNumThreadedHandlers())
        {
            eprintf("Fatal error: Cannot start all threaded message "
                    "handlers.\n");
            eprintf("Fatal error: %d were OK; please specify a lower "
                    "value.\n",numHandlers);
            delete g_queryResultManager;
            s_userManagerManager.stopManagers();
            exit(1);
        }
    }

    void stopThreads()
    {
        /* attempt to stop the user managers gracefully */
        s_userManagerManager.stopManagers();

        /* now cancel the managers, if they haven't stopped already */
        s_userManagerManager.cancelManagers();

        /* attempt to stop the handlers gracefully */
        s_threadedHandlerManager.stopHandlers();

        /* now cancel the handlers, if they haven't stopped already */
        s_threadedHandlerManager.cancelHandlers();

        delete g_queryResultManager;
        g_queryResultManager = (neQueryResultManager *)0;

        /* shutdown any disk mem allocators */
        g_userFileAllocator.shutdown();
    }

    void removeMarkedUsers(std::vector<neUser *> *markedUsers)
    {
        neUserManager *userManager = (neUserManager *)0;
        std::vector<neUser *>::iterator iter;

        assert(markedUsers);

        for(iter = markedUsers->begin();
            iter != markedUsers->end(); iter++)
        {
            assert((*iter));
            assert((*iter)->getSocket());

            userManager = (*iter)->getUserManager();
            assert(userManager);

            /*
              the reason we use the threadedHandler removal as opposed to
              the direct userManager->removeUser method is because we need
              to make sure that any messages associated with this user in
              the threadedHandler msgQueues are invalidated.  If we're
              sure that the user has no msgs in the msgQueue, use the
              userManager->removeUser method.
            */
            if ((*iter)->getAffinityValue() == -1)
            {
                userManager->removeUser(*iter);
            }
            else
            {
                s_threadedHandlerManager.removeUserForcefully
                    ((*iter)->getAffinityValue(),(*iter));
            }
            userManager->unlockUserLife(*iter);
        }
        markedUsers->clear();
    }
}
