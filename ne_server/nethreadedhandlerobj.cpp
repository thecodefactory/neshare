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
extern unsigned long numNewThreadedHandlerObjs;
extern unsigned long numDeletedThreadedHandlerObjs;
extern unsigned long totalBytesAllocated;
extern unsigned long totalBytesFreed;
#endif

#include "neserverheaders.h"

neThreadedHandlerObj::neThreadedHandlerObj()
{
    m_id = -1;
    m_state = NE_TH_UNKNOWN;

#ifdef NESHARE_DEBUG
    numNewThreadedHandlerObjs++;
    totalBytesAllocated += sizeof(neThreadedHandlerObj);
#endif
}

neThreadedHandlerObj::~neThreadedHandlerObj()
{
    int numFreed = 0;

    cancel();

    if (!m_msgQueue.empty())
    {
        iprintf("neThreadedHandlerObj::~neThreadedHandlerObj | WARNING! "
                "There are messages still pending on threadedhandler "
                "%d's msg queue\n",m_id);

        neUserMsgObj *msgObj = (neUserMsgObj *)0;
        while(!m_msgQueue.empty())
        {
            msgObj = m_msgQueue.front();
            m_msgQueue.pop();
            delete msgObj;
            msgObj = (neUserMsgObj *)0;
            ++numFreed;
        }
    }
#ifdef NESHARE_DEBUG
    numDeletedThreadedHandlerObjs++;
    totalBytesFreed += sizeof(neThreadedHandlerObj);
#endif
}

int neThreadedHandlerObj::initialize(int id, int state)
{
    int ret = 1;

    m_id = id;
    m_state = state;
    assert(m_msgQueue.empty());

    if (m_thread.start(commonMsgHandler,(void *)this) == NC_OK)
    {
        if (m_thread.detach() == NC_OK)
        {
            ret = 0;
        }
        else
        {
            m_thread.stop(0);
        }
    }
    return ret;
}

void neThreadedHandlerObj::stop()
{
    dprintf("neThreadedHandlerObj::stop   | stopping    "
            "(id = %d)\n",m_id);
    m_state = NE_TH_CANCELLED;
}

void neThreadedHandlerObj::cancel()
{
    if (m_state != NE_TH_TERMINATED)
    {
        dprintf("neThreadedHandlerObj::cancel | terminating "
                "(id = %d)\n",m_id);

        m_thread.stop(0);
        m_state = NE_TH_TERMINATED;
    }
}

int neThreadedHandlerObj::getID()
{
    return m_id;
}

int neThreadedHandlerObj::getState()
{
    return m_state;
}

void neThreadedHandlerObj::setState(int state)
{
    m_state = state;
}

void neThreadedHandlerObj::lockMsgQueue()
{
    m_msgQueueMutex.lock();
}

void neThreadedHandlerObj::unlockMsgQueue()
{
    m_msgQueueMutex.unlock();
}

int neThreadedHandlerObj::msgQueueIsEmpty()
{
    return ((m_msgQueue.empty()) ? 1 : 0);
}

neUserMsgObj *neThreadedHandlerObj::popNextMsgFromQueue()
{
    neUserMsgObj *msgObj = m_msgQueue.front();
    m_msgQueue.pop();
    return msgObj;
}

int neThreadedHandlerObj::addMsgToMsgQueue(neUserMsgObj *msgObj)
{
    int ret = 1;

    m_msgQueueMutex.lock();
    /*
      while waiting to obtain this lock, a user may
      go from connected state to disconnecting
    */
    if (msgObj && (!msgObj->getUser()->isDisconnecting()))
    {
        m_msgQueue.push(msgObj);
        ret = 0;
    }
    m_msgQueueMutex.unlock();
    return ret;
}

void neThreadedHandlerObj::removeUserFromMsgQueue(neUserMsgObj *msgObj)
{
    int msgsFreed = 0;
    neUserMsgObj *tmpMsgObj = (neUserMsgObj *)0;
    std::vector<neUserMsgObj *> tmpVector;

    assert(msgObj);
    assert(msgObj->getUser());

    tmpVector.clear();

    dprintf("neThreadedHandlerObj::removeUserFromMsgQueue | "
            "user = %x | handlerID = %d\n",msgObj->getUser(),m_id);

    /* stash keepable msgs in a tmpVector; free others */
    while(!m_msgQueue.empty())
    {
        tmpMsgObj = m_msgQueue.front();
        m_msgQueue.pop();

        if (tmpMsgObj->getUser() == msgObj->getUser())
        {
            dprintf("Freeing msg %x for user %x\n",
                    tmpMsgObj->getMsgType(),tmpMsgObj->getUser());
            delete tmpMsgObj;
            tmpMsgObj = (neUserMsgObj *)0;
            ++msgsFreed;
        }
        else
        {
            tmpVector.push_back(tmpMsgObj);
        }
    }
    assert(m_msgQueue.empty());

    /* put everything from the tmpVector back into the queue */
    std::vector<neUserMsgObj *>::iterator iter;
    for(iter = tmpVector.begin(); iter != tmpVector.end(); iter++)
    {
        m_msgQueue.push(*iter);
    }
    tmpVector.clear();

    dprintf("neThreadedHandlerObj::removeUserFromMsgQueue | Freed %d "
            "messages (queue size is now %d)\n",msgsFreed,
            m_msgQueue.size());
}

int dispatchMessageHandler(neUserMsgObj *msgObj)
{
    int ret = NE_TH_USER_FAILED;

    assert(msgObj);
    assert(msgObj->getUser());
    assert(msgObj->getCommonMsg());

    switch(msgObj->getMsgType())
    {
        case NE_MSG_DISCONNECT:
            /*
              this tells the caller that the user should be removed normally
              by handling the disconnect at a more convenient time
            */
            ret = NE_TH_USER_REMOVE;
            break;
        case NE_MSG_ENTRY_SET_START:
            ret = neShareServerThreads::neServerUtils::handleEntrySetStart(
                msgObj->getUser(),
                (nemsgEntrySetStart *)msgObj->getCommonMsg());
            break;
        case NE_MSG_ENTRY:
            ret = neShareServerThreads::neServerUtils::handleEntry(
                msgObj->getUser(),
                (nemsgEntry *)msgObj->getCommonMsg());
            ret = 0;
            break;
        case NE_MSG_ENTRY_SET_END:
            ret = neShareServerThreads::neServerUtils::handleEntrySetEnd(
                msgObj->getUser(),
                (nemsgEntrySetEnd *)msgObj->getCommonMsg());
            break;
        case NE_MSG_FORCED_DISCONNECT:
            /*
              this tells the caller that the user should be removed forcefully
              by handling the disconnect at a more convenient time
            */
            ret = NE_TH_USER_FORCE;
            break;
        case NE_MSG_PONG:
            ret = neShareServerThreads::neServerUtils::handlePong(
                msgObj->getUser(),
                (nemsgPong *)msgObj->getCommonMsg());
            break;
        case NE_MSG_SEARCH_QUERY:
            ret = neShareServerThreads::neServerUtils::handleSearchQuery(
                msgObj->getUser(),
                (nemsgSearchQuery  *)msgObj->getCommonMsg());
            break;
        case NE_MSG_PUSH_REQUEST:
            ret = neShareServerThreads::neServerUtils::handlePushRequest(
                msgObj->getUser(),
                (nemsgPushRequest *)msgObj->getCommonMsg());
            break;
        default:
            eprintf("neThreadedHandler::dispatchMessageHandler | FIXME: "
                    "Cannot dispatch a message of unknown type.\n");
    }
    return ret;
}

void *commonMsgHandler(void *ptr)
{
    int ret = 0;
    int tid = -1;
    neUserManager *userManager = (neUserManager *)0;

    neThreadedHandlerObj *threadedHandler = (neThreadedHandlerObj *)ptr;
    if (threadedHandler)
    {
        tid = threadedHandler->getID();
        dprintf("commonMsgHandler | threadedHandler %d started.\n",tid);

        while(threadedHandler->getState() == NE_TH_RUNNING)
        {
            if (threadedHandler->msgQueueIsEmpty())
            {
                ncSleep(10);
            }
            else
            {
                threadedHandler->lockMsgQueue();
                if (threadedHandler->msgQueueIsEmpty())
                {
                    threadedHandler->unlockMsgQueue();
                    continue;
                }
                neUserMsgObj *msgObj =
                    threadedHandler->popNextMsgFromQueue();
                assert(msgObj);
                assert(msgObj->getUser());

                userManager = msgObj->getUser()->getUserManager();
                assert(userManager);

                /*
                  preserve the user life throughout
                  the message dispatch
                */
                userManager->lockUserLife(msgObj->getUser());

                ret = dispatchMessageHandler(msgObj);
                if (ret)
                {
                    /*
                      mark the user as disconnecting so that we don't
                      accept many more messages from this user
                    */
                    msgObj->getUser()->setDisconnecting();
                    switch(ret)
                    {
                        case NE_TH_USER_FAILED:
                            eprintf("commonMsgHandler(%d) | "
                                    "dispatchMessageHandler "
                                    "failed on user %x\n",
                                    tid,msgObj->getUser());
                            if (msgObj->getErrorMsg())
                            {
                                eprintf(msgObj->getErrorMsg());
                            }
                            break;
                        case NE_TH_USER_REMOVE:
                            dprintf("commonMsgHandler(%d) | "
                                    "disconnecting user %x "
                                    "normally\n",tid,msgObj->getUser());
                            break;
                        case NE_TH_USER_FORCE:
                            dprintf("commonMsgHandler(%d) | "
                                    "disconnecting user %x "
                                    "forcefully\n",tid,msgObj->getUser());
                            break;
                    }

                    /*
                      flush queue; invalidating all matching user messages.
                      we have to lock the user life before the delete to
                      avoid the main thread (neShareServerThreads) from being
                      affected while possibly using the user being deleted.
                      (i.e. a reverse lock user life is done)
                    */
                    threadedHandler->removeUserFromMsgQueue(msgObj);
                    if (ret == NE_TH_USER_REMOVE)
                    {
                        neShareServerThreads::neServerUtils::handleDisconnect(
                            msgObj->getUser(),
                            (nemsgDisconnect *)msgObj->getCommonMsg());
                    }
                    else
                    {
                        neShareServerThreads::neServerUtils::handleForcedDisconnect(
                            msgObj->getUser(),
                            (nemsgForcedDisconnect *)msgObj->getCommonMsg());
                    }
                }
                /* unlock user and remove if necessary */
                userManager->unlockUserLife(msgObj->getUser());

                delete msgObj;
                msgObj = (neUserMsgObj *)0;
                threadedHandler->unlockMsgQueue();
            }
        }

        dprintf("commonMsgHandler(%d) | threadedHandler %d terminating "
                "gracefully.\n",tid,tid);
        threadedHandler->setState(NE_TH_TERMINATED);
    }
    else
    {
        eprintf("commonMsgHandler(%d) | threadedHandler is invalid. "
                "Cannot start!\n",tid);
    }
    return (void *)0;
}
