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

#ifndef __NETHREADEDHANDLEROBJ_H
#define __NETHREADEDHANDLEROBJ_H

/* valid states that the handlers can be in */
#define NE_TH_UNKNOWN    -1
#define NE_TH_TERMINATED  0
#define NE_TH_RUNNING     1
#define NE_TH_CANCELLED   2

/* valid return values for the internal dispatchMessageHandler method */
#define NE_TH_USER_OK     0
#define NE_TH_USER_FAILED 1
#define NE_TH_USER_REMOVE 2
#define NE_TH_USER_FORCE  3

/* this is the generic threaded handler method prototype */
void *commonMsgHandler(void *ptr);

/*
  this is a helper method for the generic threaded handler method.
  returns NE_TH_USER_OK on success; NE_TH_USER_FAILED on failure;
  NE_TH_USER_REMOVE if the user should be disconnected normally;
  NE_TH_USER_FORCE if the user should be disconnected forcefully
*/
int dispatchMessageHandler(neUserMsgObj *msgObj);

class neThreadedHandlerObj
{
  public:
    neThreadedHandlerObj();
    ~neThreadedHandlerObj();

    int initialize(int id, int state);

    void stop();
    void cancel();

    int getID();
    int getState();
    void setState(int state);

    void lockMsgQueue();
    void unlockMsgQueue();
    int msgQueueIsEmpty();
    neUserMsgObj *popNextMsgFromQueue();
    int addMsgToMsgQueue(neUserMsgObj *msgObj);
    void removeUserFromMsgQueue(neUserMsgObj *msgObj);

  private:
    int m_id;
    int m_state;
    ncThread m_thread;
    std::queue<neUserMsgObj *> m_msgQueue;
    ncMutex m_msgQueueMutex;
};

#endif /* __NETHREADEDHANDLEROBJ_H */
