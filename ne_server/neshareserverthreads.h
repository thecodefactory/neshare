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

#ifndef __NESHARESERVERTHREADS_H
#define __NESHARESERVERTHREADS_H

extern neUserManager *gs_userManager;

/*
  a namespace which stores all of the thread functions for the neshare_server.
  These functions are started on server initialization and unless noted are
  persistent throughout the life of the running server.
*/
namespace neShareServerThreads
{
    namespace neServerUtils
    {
        int rejectNewUser(neUser *user);

        void *getAllEntries(void *ptr);
        void *getAllKeywords(void *ptr);

        /*
          these are called from the dispatchMessageHandler
          method in nethreadedhandler.cpp
        */
        int handleDisconnect(neUser *user, nemsgDisconnect *msg);
        int handleEntrySetStart(neUser *user, nemsgEntrySetStart *msg);
        int handleEntry(neUser *user, nemsgEntry *msg);
        int handleEntrySetEnd(neUser *user, nemsgEntrySetEnd *msg);
        int handleForcedDisconnect(neUser *user,
                                   nemsgForcedDisconnect *msg);
        int handlePong(neUser *user, nemsgPong *msg);
        int handleSearchQuery(neUser *user, nemsgSearchQuery *msg);
        int handlePushRequest(neUser *user, nemsgPushRequest *msg);

        /* a helper method for neShareServerThreads::processUsers */
        int recvRawMessage(neThreadedHandlerManager *threadedHandlerManager,
                           int threadedHandlerIndex,
                           neUser *user,
                           unsigned long msgType);
    }

    /*
      thread function which listens to all user sockets and dispatches
      based on what activities are ready.  start this thread before listening
      for connections (and after g_userManager is allocated properly).
    */
    void *processUsers(void *ptr);

    /*
      thread function callback routine which handles all new user (login)
      network messages. This is the only place in the server where a user
      is added to the usermanager.
    */
    void *processLoginMessage(void *ptr);

    /*
      this is the function which starts all of the threads, calling the
      above thread functions initially, and initializes necessary object.
    */
    void startThreads();

    /*
      this stops the neShare server threads
      and does clean up of objects
    */
    void stopThreads();

    /* a helper method to remove all users specified in the vector */
    void removeMarkedUsers(std::vector<neUser *> *markedUsers);
}

#endif /* __NESHARESERVERTHREADS_H */
