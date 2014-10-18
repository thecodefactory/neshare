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

#ifndef __NEKEYWORDMANAGER_H
#define __NEKEYWORDMANAGER_H

class neKeywordManager
{
  public:
    neKeywordManager();
    ~neKeywordManager();

    /* returns the newly added keyword obj on success; NULL on error */
    neKeyword *addKeyword(std::string keyword);

    /* returns the corresponding keyword obj on success; NULL on error */
    neKeyword *getKeywordObj(std::string keyword);

    void removeKeyword(std::string keyword);

  private:
    ncMutex m_mutex;
    std::map< std::string, neKeyword > m_strToKeywordMap;
};

#endif /* __NEKEYWORDMANAGER_H */
