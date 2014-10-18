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

#ifndef __NETHREADEDHANDLERMANAGER_H
#define __NETHREADEDHANDLERMANAGER_H

class neThreadedHandlerManager
{
  public:
    neThreadedHandlerManager();
    ~neThreadedHandlerManager();

    /*
      attempts to start numHandlers handler threads.
      returns the number of handler threads actually created.
    */
    int startHandlers(int numHandlers);

    /*
      places a message in the handler thread queue
      corresponding to the specified index. The user
      should be the user which requires the handling,
      and the nemsgCommon is the already received message.
      The handlerIndex must be above zero, and below numHandlers
      (as passed into startHandlers).  the passed in commonMsg ptr
      will be de-allocated when it is no longer needed.  The
      caller only worries about the initial allocation.
      The errorMsg string is logged if the handler fails.
      returns 0 on success; 1 on failure.
    */
    int addMsgToThreadHandlerQueue(int handlerIndex,
                                   neUser *user,
                                   nemsgCommon *commonMsg,
                                   unsigned long msgType,
                                   char *errorMsg);

    /* attempts to gracefully stop all handlers */
    void stopHandlers();

    /*
      forcefully terminate all handler threads and frees
      any claimed resources used by the handler threads
    */
    void cancelHandlers();

    /*
      forcefully removes a specified user from the
      specified threadHandler (invalidating all queued
      messages and freeing the user resources
    */
    void removeUserForcefully(int handlerIndex, neUser *user);

  private:
    int m_numHandlers;
    std::vector<neThreadedHandlerObj *> m_threadedHandlers;
};

#endif /* __NETHREADEDHANDLERMANAGER_H */
