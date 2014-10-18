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

neKeywordManager::neKeywordManager()
{
    assert(m_strToKeywordMap.empty());
}

neKeywordManager::~neKeywordManager()
{
    m_strToKeywordMap.clear();
}

neKeyword *neKeywordManager::addKeyword(std::string keyword)
{
    neKeyword *ret = (neKeyword *)NULL;
    std::map< std::string, neKeyword >::iterator iter;

    m_mutex.lock();
    if ((iter = m_strToKeywordMap.find(keyword)) !=
        m_strToKeywordMap.end())
    {
        /* if keyword is in map, add a reference and return it */
        ret = &((*iter).second);
        assert(ret);
        ret->addRef();
    }
    else
    {
        /* otherwise, create a new keyword obj and return it */
        neKeyword newKeyword;

        /* for map insertion, the keyword string is copied */
        m_strToKeywordMap[keyword] = newKeyword;

        /*
          thus, to save space, set the keyword obj's std::string ptr
          to point to this copy instead of having a separate
          internal copy
        */
        if ((iter = m_strToKeywordMap.find(keyword)) !=
             m_strToKeywordMap.end())
        {
            ret = &((*iter).second);
            assert(ret);
            ret->initialize((std::string *)(&((*iter).first)));
            ret->addRef();
        }
        else
        {
            eprintf((char *)"neKeywordManager::addKeyword | Cannot find new "
                    "inserted keyword! FIXME!\n");
        }
    }
    m_mutex.unlock();
    return ret;
}

neKeyword *neKeywordManager::getKeywordObj(std::string keyword)
{
    neKeyword *ret = (neKeyword *)NULL;
    std::map< std::string, neKeyword >::iterator iter;

    m_mutex.lock();
    if ((iter = m_strToKeywordMap.find(keyword)) !=
        m_strToKeywordMap.end())
    {
        ret = &((*iter).second);
        assert(ret);
    }
    m_mutex.unlock();
    return ret;
}

void neKeywordManager::removeKeyword(std::string keyword)
{
    neKeyword *val = (neKeyword *)NULL;
    std::map< std::string, neKeyword >::iterator iter;

    m_mutex.lock();
    if ((iter = m_strToKeywordMap.find(keyword)) !=
        m_strToKeywordMap.end())
    {
        val = &((*iter).second);
        assert(val);

        /*
          if this is the last reference to this word, clear it from
          internal book keeping (i.e. reclaim map space)
        */
        if (val->dropRef() == 0)
        {
            //dprintf("Removing keyword %s\n",(char *)(val->getKeyword()->c_str()));
            m_strToKeywordMap.erase(iter);
        }
    }
    else
    {
        eprintf("neKeywordManager::removeKeyword | cannot find keyword "
                "%s in internal book keeping!\n",(char *)keyword.c_str());
    }
    m_mutex.unlock();
}
