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

neThreadedHandlerManager::neThreadedHandlerManager()
{
    m_numHandlers = 0;
    m_threadedHandlers.clear();
}

neThreadedHandlerManager::~neThreadedHandlerManager()
{
    cancelHandlers();
    m_numHandlers = 0;
}

int neThreadedHandlerManager::startHandlers(int numHandlers)
{
    neThreadedHandlerObj *threadedHandler = (neThreadedHandlerObj *)0;

    if (numHandlers)
    {
        for(int i = 0; i < numHandlers; i++)
        {
            threadedHandler = new neThreadedHandlerObj();
            if (threadedHandler)
            {
                if (threadedHandler->initialize(i,NE_TH_RUNNING))
                {
                    eprintf("neThreadedHandlerManager::startHandlers  "
                            "| failed to start threadedhandler %d\n",i);
                    break;
                }
                else
                {
                    dprintf("neThreadedHandlerManager::startHandlers  "
                            "| %d OK\n",i);
                    m_threadedHandlers.push_back(threadedHandler);
                    assert(threadedHandler == m_threadedHandlers[i]);
                }
            }
            else
            {
                eprintf("neThreadedHandlerManager::startHandlers | "
                        "Cannot allocate thread handler %d\n",i);
                eprintf("neThreadedHandlerManager::startHandlers | "
                        "Aborting.\n");
                break;
            }
        }
    }
    m_numHandlers = m_threadedHandlers.size();
    return m_numHandlers;
}

int neThreadedHandlerManager::addMsgToThreadHandlerQueue(int handlerIndex,
                                                         neUser *user,
                                                         nemsgCommon *commonMsg,
                                                         unsigned long msgType,
                                                         char *errorMsg)
{
    int ret = 1;

    if (((handlerIndex > -1) && (handlerIndex < m_numHandlers)) &&
        user && msgType)
    {
        neUserMsgObj *msgObj = new neUserMsgObj();
        if (msgObj &&
            (msgObj->initialize(user,commonMsg,msgType,errorMsg) == 0))
        {
            neThreadedHandlerObj *threadedHandler = 
                m_threadedHandlers[handlerIndex];

            assert(threadedHandler);

            if (threadedHandler->addMsgToMsgQueue(msgObj) == 0)
            {
                ret = 0;
            }
            else
            {
                delete msgObj;
                msgObj = (neUserMsgObj *)0;
            }
        }
        else
        {
            delete msgObj;
            msgObj = (neUserMsgObj *)0;
        }
    }
    return ret;
}

void neThreadedHandlerManager::stopHandlers()
{
    std::vector<neThreadedHandlerObj *>::iterator iter;
    neThreadedHandlerObj *threadedHandler = (neThreadedHandlerObj *)0;

    for(iter = m_threadedHandlers.begin();
        iter != m_threadedHandlers.end(); iter++)
    {
        threadedHandler = (*iter);
        assert(threadedHandler);

        threadedHandler->stop();
        ncSleep(100);
    }
}

void neThreadedHandlerManager::cancelHandlers()
{
    std::vector<neThreadedHandlerObj *>::iterator iter;
    neThreadedHandlerObj *threadedHandler = (neThreadedHandlerObj *)0;

    for(iter = m_threadedHandlers.begin();
        iter != m_threadedHandlers.end(); iter++)
    {
        threadedHandler = (*iter);
        assert(threadedHandler);

        threadedHandler->cancel();
        delete threadedHandler;
        threadedHandler = (neThreadedHandlerObj *)0;
    }
    m_threadedHandlers.clear();
}

void neThreadedHandlerManager::removeUserForcefully(int handlerIndex,
                                                    neUser *user)
{
    neThreadedHandlerObj *threadedHandler = (neThreadedHandlerObj *)0;
    neUserMsgObj msgObj;

    assert(user);
    assert(user->getSocket());
    assert((handlerIndex > -1) && (handlerIndex < m_numHandlers));

    dprintf("neThreadedHandlerManager::removeUserForcefully | "
            "user = %x | handlerIndex = %d\n",user,handlerIndex);

    threadedHandler = m_threadedHandlers[handlerIndex];
    assert(threadedHandler);

    if (msgObj.initialize(user,(nemsgCommon *)0,0,(char *)0) == 0)
    {
        dprintf("neThreadedHandlerManager::removeUserForcefully | "
                "user = %x | handleIndex = %d\n",msgObj.getUser(),
                handlerIndex);

        threadedHandler->lockMsgQueue();
        threadedHandler->removeUserFromMsgQueue(&msgObj);
        neShareServerThreads::neServerUtils::handleForcedDisconnect(
            user,(nemsgForcedDisconnect *)0);
        threadedHandler->unlockMsgQueue();
    }
}
