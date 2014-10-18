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

namespace neShareServerThreads
{
   namespace neServerUtils
   {
      int rejectNewUser(neUser *user)
      {
         int ret = 0;
         neUserManager *userManager = (neUserManager *)0;

         assert(user);
         
         userManager = user->getUserManager();
         assert(userManager);

         /* send the login failed message and delete the user */
         nemsgLoginFailed loginFailed(user->getSocket());
         user->lockWriteLock();
         if (loginFailed.send(g_config.getLoginFailedMessageFile()))
         {
            eprintf("neServerUtils::rejectNewUser | "
                    "login failed send failed.\n");
            ret = 1;
         }
         else
         {
            user->getSocket()->flush();
         }
         user->unlockWriteLock();

         /* clean up user if it was added to the user manager properly */
         if (userManager->removeUser(user))
         {
             /* otherwise, just delete the user */
             delete user;
         }
         user = (neUser *)0;
         return ret;
      }
      
      int handleDisconnect(neUser *user, nemsgDisconnect *msg)
      {
         int ret = 0;
         unsigned long userSymbol = (unsigned long)user;
         neUserManager *userManager = (neUserManager *)0;

         assert(user);
         assert(msg);
         
         userManager = user->getUserManager();
         assert(userManager);

         /* read disconnect msg from user */
         if (msg->format())
         {
            user->incrementErrorCount();
            ret = 1;
         }
         else
         {
            /* send a disconnect ack message back to user */
            nemsgDisconnectAck disconnectAckMsg(user->getSocket());
            user->lockWriteLock();
            if (disconnectAckMsg.send(g_config.getGoodbyeMessageFile()))
            {
               user->unlockWriteLock();
               dprintf("disconnectAckMsg send FAILED (user %x)\n",user);
               user->incrementErrorCount();
               ret = 1;
            }
            else
            {
               dprintf("disconnectAckMsg sent OK (user %x)\n",user);
               user->getSocket()->flush();
               user->unlockWriteLock();
               userManager->removeUser(user);
               iprintf("User %x removed normally.  (%lu remaining).\n",
                       userSymbol,userManager->getNumUsers());
            }
         }
         return ret;
      }

      int handleEntrySetStart(neUser *user, nemsgEntrySetStart *msg)
      {
         int ret = 0;
         unsigned long entryCount = 0;
         neUserManager *userManager = (neUserManager *)0;

         assert(user);
         assert(msg);

         userManager = user->getUserManager();
         assert(userManager);

         /* 
            for now ignore the entryCount, although later
            the server admin should be able to limit this
            number to a configurable max number in which
            case we would have to disconnect the user forcefully.
         */
         if (msg->format(&entryCount))
         {
            user->incrementErrorCount();
            ret = 1;
         }
         else
         {
            /* 
               flush out all user entries and remove all
               owned keywords from the global keyword map.
               This message tells us to make room for a new
               list of entries for a particular user (such as
               is the case at the start of a submit or resubmit
               file list action).
            */
            dprintf("neServerUtils::handleEntrySetStart | entrySetStart "
                    "says %u entries.\n",entryCount);

            if (userManager->removeEntriesFromUser(user))
            {
               eprintf("neServerUtils::handleEntrySetStart | "
                       "removeEntriesFromUser failed.\n");
               user->incrementErrorCount();
               ret = 1;
            }
         }
         return ret;
      }

      int handleEntry(neUser *user, nemsgEntry *msg)
      {
         int ret = 0;
         unsigned long filesize = 0;
         char filename[NE_MSG_MAX_DATA_LEN];
         neUserManager *userManager = (neUserManager *)0;

         assert(user);
         assert(msg);

         userManager = user->getUserManager();
         assert(userManager);

         /* receive the entry (filename) */
         if (msg->format(filename,NE_MSG_MAX_DATA_LEN,&filesize))
         {
            user->incrementErrorCount();
            ret = 1;
         }
         else
         {
            /* add the entry to the user's filelist (if it's not empty) */
            if (filename &&
                userManager->addEntryToUser(user,filename,filesize))
            {
               user->incrementErrorCount();
               ret = 1;
            }
         }
         return ret;
      }

      int handleEntrySetEnd(neUser *user, nemsgEntrySetEnd *msg)
      {
         int ret = 0;
         unsigned long entryCount = 0;
         neUserManager *userManager = (neUserManager *)0;

         assert(user);
         assert(msg);

         userManager = user->getUserManager();
         assert(userManager);

         /* 
            for now ignore the entryCount, although later
            the server admin should be able to limit this
            number to a configurable max number in which
            case we would have to disconnect the user forcefully.
         */
         if (msg->format(&entryCount))
         {
            user->incrementErrorCount();
            ret = 1;
         }
         else
         {
            /* 
               once we're told there are no more incoming entries
               for this user, merge the current user entries into
               the 'global' keyword map if they've satisfied the
               server configured limit (i.e. min num MB) for sharing.
            */
            dprintf("neServerUtils::handleEntrySetEnd | "
                    "entrySetEnd says %u entries.\n",entryCount);
            ret = userManager->mergeEntriesToKeywordMap(user);
            if (ret == 1)
            {
               eprintf("neServerUtils::handleEntrySetEnd | "
                       "mergeEntriesToKeywordMap failed.\n");
               user->incrementErrorCount();
               ret = 1;
            }
            else if (ret == -1)
            {
               /*
                 disconnect the user with a message saying that they
                 are not sharing enough files currently and to share
                 more, etc
               */
               char buf[NE_MSG_MAX_DATA_LEN];
               snprintf(buf,NE_MSG_MAX_DATA_LEN,"You were disconnected "
                        "because this server requires that at least %uMB "
                        "of files are shared to participate.",
                        g_config.getMinUserSharedFileLimit());
               iprintf("neServerUtils::handleEntrySetEnd | User %x "
                       "is below the minimum shared file limit.\n",user);
               nemsgError errorMsg(user->getSocket());
               user->lockWriteLock();
               errorMsg.send(buf);
               user->getSocket()->flush();
               user->unlockWriteLock();
               ret = 1;
            }
         }
         return ret;
      }

      int handleForcedDisconnect(neUser *user, nemsgForcedDisconnect *msg)
      {
         int ret = 0;
         unsigned long userSymbol = (unsigned long)user;
         neUserManager *userManager = (neUserManager *)0;

         assert(user);

         userManager = user->getUserManager();
         assert(userManager);

         if (user->getSocket())
         {
            nemsgForcedDisconnect forcedDisconnectMsg(user->getSocket());
            user->lockWriteLock();
            forcedDisconnectMsg.send(g_config.getInvalidMessageTypeFile());
            user->getSocket()->flush();
            user->unlockWriteLock();
         }
         userManager->removeUser(user);
         iprintf("User %x removed forcefully.  (%d remaining).\n",
                 userSymbol,userManager->getNumUsers());
         return ret;
      }

      int handlePong(neUser *user, nemsgPong *msg)
      {
         int ret = 0;

         assert(user);
         assert(msg);

         /* receive the pong */
         if (msg->format())
         {
            user->incrementErrorCount();
            ret = 1;
         }
         return ret;
      }

      int handleSearchQuery(neUser *user, nemsgSearchQuery *msg)
      {
         int ret = 1;
         unsigned long typeflag = 0;
         unsigned long numKeywords = 0;
         char keyword[NE_MSG_MAX_DATA_LEN];
         neUserSearchQuery userSearchQuery;
         neUserManager *userManager = (neUserManager *)0;

         assert(user);
         assert(msg);

         userManager = user->getUserManager();
         assert(userManager);

         /* read the search query msg from the user */
         if (msg->getNumKeywords(&numKeywords))
         {
            eprintf("neServerUtils::handleSearchQuery | "
                    "getNumKeywords failed.\n");
            user->incrementErrorCount();
            return ret;
         }
         iprintf("neServerUtils::handleSearchQuery | got "
                 "search query (%u keywords)\n",numKeywords);

         /* and retrieve all of the keywords */
         for(unsigned long i = 0; i < numKeywords; i++)
         {
            memset(keyword,0,NE_MSG_MAX_DATA_LEN);
            
            if (msg->getNextKeyword(&typeflag,keyword,NE_MSG_MAX_DATA_LEN))
            {
               eprintf("neServerUtils::handleSearchQuery | "
                       "getNextKeyword failed (%d).\n",i);
               user->incrementErrorCount();
               return ret;
            }

            iprintf("neServerUtils::handleSearchQuery | Keyword: %s and "
                    "typeFlag %d\n",keyword,typeflag);

            /* otherwise, add the information to the USER_SEARCH_QUERY */
            userSearchQuery.addKeyword(keyword);
            userSearchQuery.addTypeFlag(typeflag);
            dprintf("neServerUtils::handleSearchQuery | searching "
                    "for %s\n",keyword);
         }

         /* now, perform the search */
         if (userManager->performSearchQuery(&userSearchQuery,user))
         {
            user->incrementErrorCount();
            eprintf("neServerUtils::handleSearchQuery | Error "
                    "performing search query.\n");
            eprintf("neServerUtils::handleSearchQuery | Sending "
                    "an error message.\n");
            nemsgError errorMsg(user->getSocket());
            user->lockWriteLock();
            if (errorMsg.send("Error performing search "
                              "query.  Try again later.\n"))
            {
               user->unlockWriteLock();
               eprintf("neServerUtils::handleSearchQuery | Error "
                       "message send failed.\n");
               user->incrementErrorCount();
               return ret;
            }
            user->getSocket()->flush();
            user->unlockWriteLock();
            return 0;
         }
         else
         {
            iprintf("neServerUtils::handleSearchQuery | completed "
                    "successfully.\n");
            ret = 0;
         }
         return ret;
      }

      int handlePushRequest(neUser *user, nemsgPushRequest *msg)
      {
         int ret = 0;
         neUser *targetUser = (neUser *)0;
         unsigned long ipAddr = 0;
         unsigned long ctrlPort = 0;
         unsigned long id = 0;
         char filename[NE_MSG_MAX_DATA_LEN];
         neUserManager *userManager = (neUserManager *)0;

         assert(user);
         assert(msg);

         userManager = user->getUserManager();
         assert(userManager);

         if (msg->format(&ipAddr,&ctrlPort,&id,filename,
                         NE_MSG_MAX_DATA_LEN) == 0)
         {
            /* if the push requester is also behind a firewall */
            if (user->getFirewallStatus())
            {
               /* return an error because a push is not possible */
               nemsgError errorMsg(user->getSocket());
               user->lockWriteLock();
               ret = errorMsg.send("Push Request is not possible since "
                                   "you are behind a firewall.");
               user->unlockWriteLock();
            }
            else
            {
               /* otherwise, pass this message along to the user specified */
               targetUser = userManager->findUser(ipAddr,ctrlPort,id);
               if (targetUser)
               {
                  /*
                    send a push request, with the Ip address of
                    the original push message sender
                  */
                  nemsgPushRequest targetPushRequestMsg
                    (targetUser->getSocket());
                  user->lockWriteLock();
                  ret = targetPushRequestMsg.send(
                      user->getSocket()->getIpAddr(),
                      ctrlPort,id,filename);
                  user->unlockWriteLock();
               }
               else
               {
                  iprintf("neServerUtils::handlePushRequest | invalid "
                          "user specified.\n");
               }
            }
         }
         else
         {
            eprintf("neServerUtils::handlePushRequest | failed to "
                    "recv push request.\n");
         }
         return ret;
      }

      int recvRawMessage(neThreadedHandlerManager *threadedHandlerManager,
                         int threadedHandlerIndex,
                         neUser *user,
                         unsigned long msgType)
      {
         int ret = 1;
         char buf[NE_MSG_MAX_ERROR_LEN];
         nemsgCommon *commonMsg = (nemsgCommon *)0;

         assert(threadedHandlerManager);
         assert(user);

         memset(buf,0,NE_MSG_MAX_ERROR_LEN);

         switch(msgType)
         {
         case NE_MSG_DISCONNECT:
            if ((commonMsg = new nemsgDisconnect(user->getSocket())))
            {
               sprintf(buf,"%s","Disconnect message handler failed.\n");
               ret =  ((nemsgDisconnect *)commonMsg)->recv();
            }
            break;
         case NE_MSG_ENTRY_SET_START:
            if ((commonMsg = new nemsgEntrySetStart(user->getSocket())))
            {
               sprintf(buf,"%s","EntrySetStart message handler failed.\n");
               ret = ((nemsgEntrySetStart *)commonMsg)->recv();
            }
            break;
         case NE_MSG_ENTRY:
            if ((commonMsg = new nemsgEntry(user->getSocket())))
            {
               sprintf(buf,"%s","Entry message handler failed.\n");
               ret = ((nemsgEntry *)commonMsg)->recv();
            }
            break;
         case NE_MSG_ENTRY_SET_END:
            if ((commonMsg = new nemsgEntrySetEnd(user->getSocket())))
            {
               sprintf(buf,"%s","EntrySetEnd message handler failed.\n");
               ret = ((nemsgEntrySetEnd *)commonMsg)->recv();
            }
            break;
         case NE_MSG_PONG:
            if ((commonMsg = new nemsgPong(user->getSocket())))
            {
               sprintf(buf,"%s","Pong message handler failed.\n");
               ret = ((nemsgPong *)commonMsg)->recv();
            }
            break;
         case NE_MSG_SEARCH_QUERY:
            if ((commonMsg = new nemsgSearchQuery(user->getSocket())))
            {
               sprintf(buf,"%s","SearchQuery message handler failed.\n");
               ret = ((nemsgSearchQuery *)commonMsg)->recv();
            }
            break;
         case NE_MSG_PUSH_REQUEST:
            if ((commonMsg = new nemsgPushRequest(user->getSocket())))
            {
               sprintf(buf,"%s","PushRequest message handler failed.\n");
               ret = ((nemsgPushRequest *)commonMsg)->recv();
            }
            break;
         default:
            dprintf("neServerUtils::recvRawMessage | invalid "
                    "message type received! (%x)\n",msgType);
#ifdef NESHARE_DEBUG
            /*
              pre-emptively reduce the totalBytesAllocated
              since we didn't allocate any (but will add below)
            */
            totalBytesAllocated -= sizeof(nemsgCommon);
#endif
         }

#ifdef NESHARE_DEBUG
         totalBytesAllocated += sizeof(nemsgCommon);
#endif

         if (ret == 0)
         {
            ret = threadedHandlerManager->addMsgToThreadHandlerQueue(
               threadedHandlerIndex,user,commonMsg,msgType,(char *)buf);
         }
         return ret;
      }
   }
}
