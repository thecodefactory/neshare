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

neUserSearchQuery::neUserSearchQuery()
{
}

neUserSearchQuery::~neUserSearchQuery()
{
    std::vector<char *>::iterator iter;
    for(iter = m_keywords.begin(); iter != m_keywords.end(); iter++)
    {
        assert(*iter);
        free(*iter);
        (*iter) = (char *)0;
    }
    m_keywords.clear();
    m_typeFlags.clear();
}

void neUserSearchQuery::addKeyword(char *keyword)
{
    if (keyword)
    {
        m_keywords.push_back(strdup(keyword));
    }
}

void neUserSearchQuery::addTypeFlag(unsigned long typeFlag)
{
    m_typeFlags.push_back(typeFlag);
}

int neUserSearchQuery::isValidQuery()
{
    return (m_keywords.size() == m_typeFlags.size());
}

std::vector<char *> *neUserSearchQuery::getKeywords()
{
    return &(m_keywords);
}

std::vector<unsigned long> *neUserSearchQuery::getTypeFlags()
{
    return &(m_typeFlags);
}
