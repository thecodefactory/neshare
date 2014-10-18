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
extern int numNewUserFiles;
extern int numDeletedUserFiles;
extern unsigned long totalBytesAllocated;
extern unsigned long totalBytesFreed;
#endif

#include "neserverheaders.h"

extern neKeywordManager g_keywordManager;

neUserFile::neUserFile(neUser *user)
{
    m_filesize = 0;
    m_user = user;
    memset(m_filename,0,NE_MSG_MAX_DATA_LEN);

#ifdef NESHARE_DEBUG
    numNewUserFiles++;
    totalBytesAllocated += sizeof(neUserFile);
#endif
}

neUserFile::~neUserFile()
{
    m_filesize = 0;
    m_user = (neUser *)0;

#ifdef NESHARE_DEBUG
    numDeletedUserFiles++;
    totalBytesFreed += sizeof(neUserFile);
#endif

    std::vector<neKeyword *>::iterator iter;
    for(iter = m_keywords.begin(); iter != m_keywords.end(); iter++)
    {
        g_keywordManager.removeKeyword(*((*iter)->getKeyword()));
    }
    m_keywords.clear();
}

int neUserFile::initialize(char *filename, unsigned long filesize)
{
    int i = 0;
    int start = 0;
    int end = 0;
    int len = 0;
    char keyword[NE_MSG_MAX_DATA_LEN] = {0};
    char *ptrLimit = (char *)0;
    neKeyword *nKey = (neKeyword *)0;

    assert(filename);

    len = strlen(filename);
    m_filesize = filesize;
    memcpy(m_filename,filename,MIN((unsigned long)len,
                                   NE_MSG_MAX_DATA_LEN-1));
    ptrLimit = (char *)(filename + len);

    /* break up the string into keywords */
    for(char *ptr = filename; *ptr && (ptr < (char *)ptrLimit); ptr++)
    {
        if (isalnum(*ptr))
        {
            end++;
        }
        else
        {
            /* if at a delimiter, break up keyword */
            for(i = 0; i < (end-start); i++)
            {
                keyword[i] = toupper((int)filename[start+i]);
            }
            keyword[(end-start)] = '\0';

            if (strlen(keyword) > 0)
            {
                nKey = g_keywordManager.addKeyword(std::string(keyword));
                if (nKey)
                {
                    m_keywords.push_back(nKey);
                }
                else
                {
                    eprintf("neUserFile::initialize | Keyword \"%s\" "
                            "failed to be added\n", keyword);
                }
            }
            start = end;
            start++;
            end++;
        }
    }
    if (end-start)
    {
        /* grab the last keyword, if we skipped it before */
        for(i = 0; i < (end-start); i++)
        {
            keyword[i] = toupper((int)filename[start+i]);
        }
        keyword[(end-start)] = '\0';

        if (strlen(keyword) > 0)
        {
            nKey = g_keywordManager.addKeyword(std::string(keyword));
            if (nKey)
            {
                m_keywords.push_back(nKey);
            }
            else
            {
                eprintf("neUserFile::initialize | Keyword \"%s\" "
                        "failed to be added\n", keyword);
            }
        }
    }
    return ((m_keywords.size()) ? 0 : 1);
}

int neUserFile::getNumKeywords()
{
    return m_keywords.size();
}

std::string *neUserFile::getKeyword(int index)
{
    return (m_keywords[index])->getKeyword();
}

neKeyword *neUserFile::getKeywordObj(int index)
{
    return (m_keywords[index]);
}

char *neUserFile::getFilename()
{
    return m_filename;
}

unsigned long neUserFile::getFilesize()
{
    return m_filesize;
}

neUser *neUserFile::getOwner()
{
    return m_user;
}
