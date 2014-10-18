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

#ifndef __NEUSERSEARCHQUERY_H
#define __NEUSERSEARCHQUERY_H

class neUserSearchQuery
{
  public:
    neUserSearchQuery();
    ~neUserSearchQuery();

    void addKeyword(char *keyword);
    void addTypeFlag(unsigned long typeFlag);

    int isValidQuery();

    std::vector<char *> *getKeywords();
    std::vector<unsigned long> *getTypeFlags();

  private:
    std::vector<char *> m_keywords;
    std::vector<unsigned long> m_typeFlags;
};

#endif /* __NEUSERSEARCHQUERY_H */
