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
extern int numNewQueryResultManagers;
extern int numDeletedQueryResultManagers;
extern unsigned long totalBytesAllocated;
extern unsigned long totalBytesFreed;
#endif

#include "neserverheaders.h"

extern neConfig g_config;
extern neKeywordManager g_keywordManager;

neQueryResultManager::neQueryResultManager()
{
    assert(m_keywordMap.empty());
#ifdef NESHARE_DEBUG
    numNewQueryResultManagers++;
    totalBytesAllocated += sizeof(neQueryResultManager);
#endif
}

neQueryResultManager::~neQueryResultManager()
{
    m_keywordMap.clear();

#ifdef NESHARE_DEBUG
    numDeletedQueryResultManagers++;
    totalBytesFreed += sizeof(neQueryResultManager);
#endif
}

int neQueryResultManager::mergeEntriesToKeywordMap
(neUser *user,
 neUserFileList *userFileList)
{
    int ret = 0;
    int numKeywords = 0;
    char *keyword = (char *)0;
    std::vector<neUserFile *> *vectorPtr = (std::vector<neUserFile *> *)0;
    neKeyword *nKey = (neKeyword *)0;

    assert(user);
    assert(userFileList);

    numKeywords = userFileList->getNumKeywords();

    /* add all keyword entries at once */
    m_keywordMapMutex.lock();

    /* for each keyword in the user file list */
    userFileList->resetKeywordPtr();
    while((keyword = userFileList->getNextKeyword()))
    {
        /*
          add the associated vector ptr of neUserFile objects already
          built in the neUserFileList object to the keywordMap
        */
        vectorPtr = userFileList->getUserFilesFromKeyword(keyword);
        if (vectorPtr)
        {
            /*
              if there is no second level vector,
              this syntax will create one
            */
            nKey = g_keywordManager.getKeywordObj(std::string(keyword));
            if (nKey)
            {
                m_keywordMap[nKey].push_back(vectorPtr);
                nKey->addRef();
                ret++;
            }
        }
        else
        {
            eprintf("neUserManager::mergeEntriesToKeywordMap | "
                    "Error: Keyword (%s) found with no associated "
                    "vector of userFile objects\n",keyword);
        }
    }
    dprintf("neUserManager::mergeEntriesToKeywordMap | GLOBAL "
            "KEYWORD MAP HAS %d ENTRIES\n",(int)m_keywordMap.size());
    m_keywordMapMutex.unlock();
    return ((ret == numKeywords) ? 0 : 1);
}

int neQueryResultManager::removeEntriesFromUser
(neUser *user,neUserFileList *userFileList)
{
    int ret = 0;
    char *keyword = (char *)0;
    std::map< neKeyword *,
        std::vector< std::vector<neUserFile *> * > >::iterator
        keywordMapIter;
    std::vector<neUserFile *> *
        userFilesVectorPtr = (std::vector<neUserFile *> *)0;
    std::vector< std::vector<neUserFile *> *>::iterator
        vectorOfVectorPtrsIter;
    neKeyword *nKey = (neKeyword *)0;

    assert(user);
    assert(userFileList);

    /* remove all keyword entries at once */
    m_keywordMapMutex.lock();

    userFileList->resetKeywordPtr();
    while((keyword = userFileList->getNextKeyword()))
    {
        nKey = g_keywordManager.getKeywordObj(std::string(keyword));
        if (!nKey)
        {
            continue;
        }

        /* find keyword in keyword map and get resulting vector */
        if ((keywordMapIter = m_keywordMap.find(nKey)) !=
            m_keywordMap.end())
        {
            userFilesVectorPtr =
                userFileList->getUserFilesFromKeyword(keyword);
            if (userFilesVectorPtr)
            {
                if ((vectorOfVectorPtrsIter =
                     find(((*keywordMapIter).second).begin(),
                          ((*keywordMapIter).second).end(),
                          userFilesVectorPtr)) != 
                    ((*keywordMapIter).second).end())
                {
                    /*
                      remove the vector pointer of userFiles from
                      the vector of vectors of userFiles
                    */
                    ((*keywordMapIter).second).erase(
                        vectorOfVectorPtrsIter);

                    /*
                      check if this was the last vector pointer
                      in the vector, and if so, remove self
                    */
                    if (((*keywordMapIter).second).empty())
                    {
                        g_keywordManager.removeKeyword(
                            *(nKey->getKeyword()));
                        m_keywordMap.erase(keywordMapIter);
                    }
                }
            }
        }
    }
    dprintf("neQueryResultManager::removeEntriesFromUser | "
            "GLOBAL KEYWORD MAP HAS %d ENTRIES\n",
            (int)m_keywordMap.size());
    m_keywordMapMutex.unlock();
    return ret;
}

neQueryResult *neQueryResultManager::performSearchQuery
(neUserSearchQuery *query, neUser *targetUser)
{
    std::vector<char *>::iterator keywordIter;
    std::vector<unsigned long>::iterator typeflagIter;
    std::vector< std::vector<neUserFile *> * > *vectorOfVectorsPtr = 0;
    neQueryResult *result = (neQueryResult *)0;
    std::map< neKeyword *,
        std::vector< std::vector<neUserFile * > *> >::iterator
        keywordMapIter;
    neKeyword *nKey = (neKeyword *)0;

    assert(query);
    assert(targetUser);

    /* look up all keywords in the input search query */
    for(keywordIter = query->getKeywords()->begin(),
            typeflagIter = query->getTypeFlags()->begin();
        keywordIter != query->getKeywords()->end() &&
            typeflagIter != query->getTypeFlags()->end();
        keywordIter++, typeflagIter++)
    {
        nKey = g_keywordManager.getKeywordObj(std::string(*keywordIter));
        if (!nKey)
        {
            continue;
        }

        m_keywordMapMutex.lock();
        if ((keywordMapIter =
             m_keywordMap.find(nKey)) !=
            m_keywordMap.end())
        {
            /* if we have a keyword match */
            vectorOfVectorsPtr = &((*keywordMapIter).second);

            /* build up the results to return */
            result = buildQueryResult(vectorOfVectorsPtr,
                                      *(nKey->getKeyword()));
        }
        m_keywordMapMutex.unlock();
    }
    return result;
}

int neQueryResultManager::sendQueryResults(neQueryResult *result,
                                           neUser *user)
{
    int ret = 1;
    int numResults = 0;
    int i = 0;
    neUserFile *userFile = (neUserFile *)0;

    assert(user);
    assert(user->getSocket());

    if (result)
    {
        assert(result);
        assert(result->getFiles());
        numResults = result->getFiles()->size();
    }

    iprintf("neQueryResultManager::sendQueryResults | search went "
            "ok. (%u results for target user %x)\n",numResults,user);

    nemsgSearchResults searchResultsMsg(user->getSocket());
    user->lockWriteLock();
    if (searchResultsMsg.send(numResults) == 0)
    {
        user->unlockWriteLock();
        nemsgQueryResult queryResultMsg(user->getSocket());

        for(i = 0; i < numResults; i++)
        {
            userFile = (*(result->getFiles()))[i];
            assert(userFile);

            /*
              if the userfile was invalidated by another thread,
              send a fake result back in place of the original.
              FIXME: we must send a fake since the protocol does
              not allow a way for telling the client that the number
              of results it will be getting back is different from
              the num results we just sent it above.  this should be
              fixed in the future.
            */
            if (!userFile->getOwner() ||
                userFile->getOwner()->isDisconnecting() ||
                !userFile->getOwner()->isFileListValid() ||
                user->isDisconnecting())
            {
                dprintf("nequeryresultmanager::sendQueryResults | "
                        "Result invalidated...sending fake result\n");
                user->lockWriteLock();
                queryResultMsg.send(0,0,0,0,0,0,"Invalid");
                user->unlockWriteLock();
                continue;
            }
            user->lockWriteLock();
            if (queryResultMsg.send(
                    userFile->getOwner()->getSocket()->getIpAddr(),
                    (unsigned long)
                    userFile->getOwner()->getConnectionSpeed(),
                    userFile->getOwner()->getFirewallStatus(),
                    userFile->getFilesize(),
                    userFile->getOwner()->getControlPort(),
                    userFile->getOwner()->getSocket()->getSockfd(),
                    userFile->getFilename()))
            {
                user->unlockWriteLock();
                eprintf("neQueryResultManager::sendQueryResults | Error "
                        "sending search result %d to user %x\n",
                        i,user);
                break;
            }
            user->unlockWriteLock();
        }
        ret = ((i == numResults) ? 0 : 1);
    }
    else
    {
        user->unlockWriteLock();
    }
    return ret;
}

neQueryResult *neQueryResultManager::buildQueryResult
(std::vector< std::vector<neUserFile * > * > *vectorOfVectorsPtr,
 std::string keyword)
{
    int count = 0;
    int maxResults = g_config.getMaxResults()-1;
    std::vector<neUserFile *>::iterator userFileIter;
    std::vector< std::vector<neUserFile *> * >::iterator
        vectorOfVectorsIter;
    neQueryResult *result = new neQueryResult();

    if (result && vectorOfVectorsPtr)
    {
        /* add every single neUserFile to result object */
        for(vectorOfVectorsIter = vectorOfVectorsPtr->begin();
            vectorOfVectorsIter != vectorOfVectorsPtr->end(); 
            vectorOfVectorsIter++)
        {
            for(userFileIter = (*vectorOfVectorsIter)->begin();
                userFileIter != (*vectorOfVectorsIter)->end();
                userFileIter++)
            {
                /* do not add an exact duplicate to the result list */
                if (find(result->getFiles()->begin(),
                         result->getFiles()->end(),
                         *userFileIter) == result->getFiles()->end())
                {
                    assert(*userFileIter);
                    assert((*userFileIter)->getOwner());
                    assert((*userFileIter)->getOwner()->getSocket());
                    assert((*userFileIter)->getFilename());

                    result->getFiles()->push_back(*userFileIter);
                    if (++count > maxResults)
                    {
                        break;
                    }
                }
            }
            if (count > maxResults)
            {
                break;
            }
        }
    }
    else
    {
        delete result;
        result = (neQueryResult *)0;
    }
    return result;
}
