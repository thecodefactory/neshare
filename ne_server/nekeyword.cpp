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

neKeyword::neKeyword()
{
    m_rcount = 0;
    m_value = (std::string *)0;
}

neKeyword::~neKeyword()
{
    m_rcount = 0;
    m_value = (std::string *)0;
}

void neKeyword::initialize(std::string *keyword)
{
    m_value = keyword;
}

std::string *neKeyword::getKeyword()
{
    return m_value;
}

void neKeyword::addRef()
{
    ++m_rcount;
}

int neKeyword::dropRef()
{
    return --m_rcount;
}
