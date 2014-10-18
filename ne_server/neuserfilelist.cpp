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
extern unsigned long numNewUserFileLists;
extern unsigned long numDeletedUserFileLists;
extern unsigned long numNewUserFileVectors;
extern unsigned long numDeletedUserFileVectors;
extern unsigned long totalBytesAllocated;
extern unsigned long totalBytesFreed;
#endif

#include "neserverheaders.h"

extern neDiskMemAllocator g_userFileAllocator;
extern neKeywordManager g_keywordManager;

neUserFileList::neUserFileList()
{
    m_numUserFiles = 0;
    m_totalSizeofUserFiles = 0;

#ifdef NESHARE_DEBUG
    numNewUserFileLists++;
    totalBytesAllocated += sizeof(neUserFileList);
#endif
}

neUserFileList::~neUserFileList()
{
    empty();
#ifdef NESHARE_DEBUG
    numDeletedUserFileLists++;
    totalBytesFreed += sizeof(neUserFileList);
#endif
}

int neUserFileList::addUserFile(neUserFile *userFile)
{
    int ret = 1;
    int numKeywords = 0;
    std::map< neKeyword *, std::vector<neUserFile *> >::iterator iter;
    neKeyword *nKey = (neKeyword *)0;

    assert(userFile);
    numKeywords = userFile->getNumKeywords();

    /*
      for each keyword in the userFile,
      find the list of neUserFiles
    */
    for(int i = 0; i < numKeywords; i++)
    {
        nKey = userFile->getKeywordObj(i);
        assert(nKey);

        /*
          if there is no mapping from this keyword
          to a list of userFiles create one
        */
        if ((iter = m_keywordToUserFileMap.find(nKey)) ==
            m_keywordToUserFileMap.end())
        {
            m_keywordToUserFileMap[nKey] = std::vector<neUserFile *>();
            iter = m_keywordToUserFileMap.find(nKey);
            nKey->addRef();
        }
        if (iter != m_keywordToUserFileMap.end())
        {
            /*
              finally, add the userFile - thus creating the
              mapping from keyword to userFile
            */
            std::vector<neUserFile *> &userFileVector = (*iter).second;
            userFileVector.push_back(userFile);
            ret = 0;
        }
        else
        {
            eprintf("neUserFileList::addUserFile | added an entry that "
                    "can no longer be found! FIXME\n");
        }
    }

    if (ret == 0)
    {
        /* update the internal file statistics */
        int sizeofFileInMBytes =
            BYTES_TO_MBYTES(userFile->getFilesize());
        m_numUserFiles++;
        m_totalSizeofUserFiles += ((sizeofFileInMBytes == 0) ? 
                                   1 : sizeofFileInMBytes);
    }
    return ret;
}

void neUserFileList::empty()
{
    std::vector<neUserFile *>::iterator userFileIter;
    std::map< neKeyword *, std::vector<neUserFile *> >::iterator iter;
    std::map< neUserFile *, neUserFile * > markedDeleted;
    std::map< neUserFile *, neUserFile * >::iterator
        markedDeletedIter;
    neUserFile *userFile = (neUserFile *)0;
    neKeyword *nKey = (neKeyword *)0;

    /* must free all objects used for mapping keywords */
    for(iter = m_keywordToUserFileMap.begin();
        iter != m_keywordToUserFileMap.end(); iter++)
    {
        nKey = (*iter).first;
        assert(nKey);
        g_keywordManager.removeKeyword(*(nKey->getKeyword()));

        std::vector<neUserFile *> &userFileVector = ((*iter).second);

        for(userFileIter = userFileVector.begin();
            userFileIter != userFileVector.end();
            userFileIter++)
        {
            /* 
               mark each neUserFile object for deletion being careful not
               to mark the same object twice (using natural properties of
               a map). The neUserFile objects were originally allocated
               from neUserManager::addEntryToUser.
            */
            if (*userFileIter)
            {
                markedDeleted[(*userFileIter)] = *userFileIter;
            }
        }
        /* erase all elements and then delete the file vector */
        userFileVector.clear();
    }
    m_keywordToUserFileMap.clear();

    /*
      now free all memory from each neUserFile object that was
      marked for deletion.  There should be no duplicates here
    */
    for(markedDeletedIter  = markedDeleted.begin();
        markedDeletedIter != markedDeleted.end();
        markedDeletedIter++)
    {
        userFile = (*markedDeletedIter).first;
        assert(userFile == ((*markedDeletedIter).second));

        userFile->~neUserFile();
        g_userFileAllocator.free(userFile);
        //delete userFile;
        userFile = (neUserFile *)0;
    }
    markedDeleted.clear();
    m_numUserFiles = 0;
    m_totalSizeofUserFiles = 0;
}

unsigned long neUserFileList::getNumUserFiles()
{
    return m_numUserFiles;
}

unsigned long neUserFileList::getTotalSizeofUserFiles()
{
    return m_totalSizeofUserFiles;
}

std::vector<neUserFile *> *neUserFileList::getUserFilesFromKeyword(
    char *keyword)
{
    std::vector<neUserFile *> *ret = (std::vector<neUserFile *>*)0;
    std::map< neKeyword *, std::vector<neUserFile *> >::iterator iter;
    neKeyword *nKey = (neKeyword *)0;

    if (keyword)
    {
        nKey = g_keywordManager.getKeywordObj(std::string(keyword));
        if (nKey)
        {
            iter = m_keywordToUserFileMap.find(nKey);
            if (iter != m_keywordToUserFileMap.end())
            {
                ret = &((*iter).second);
            }
        }
    }
    return ret;
}

void neUserFileList::resetKeywordPtr()
{
    m_keywordToUserFileMapIter = m_keywordToUserFileMap.begin();
}

char *neUserFileList::getNextKeyword()
{
    char *retStr = (char *)0;
    if (m_keywordToUserFileMapIter != m_keywordToUserFileMap.end())
    {
        retStr = (char *)
            (((*m_keywordToUserFileMapIter).first)->getKeyword()->c_str());
        m_keywordToUserFileMapIter++;
    }
    return retStr;
}

int neUserFileList::getNumKeywords()
{
    return (int)(m_keywordToUserFileMap.size());
}
