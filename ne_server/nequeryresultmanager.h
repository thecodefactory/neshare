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

#ifndef __NEQUERYRESULTMANAGER_H
#define __NEQUERYRESULTMANAGER_H

class neQueryResultManager
{
  public:
    neQueryResultManager();
    ~neQueryResultManager();

    /*
      updates the keyword map with the entries of the given user.
      this function returns 0 on success and 1 on failure.
    */
    int mergeEntriesToKeywordMap(neUser *user,
                                 neUserFileList *userFileList);

    /*
      removes all entries from the user and empties 
      all owned keywords from the keyword map
    */
    int removeEntriesFromUser(neUser *user, neUserFileList *userFileList);

    /*
      performs a search query based on the query
      and returns a newly allocated neQueryResult object
      on success.  NULL is returned on error.  The caller
      MUST deallocate this neQueryResult object.
    */
    neQueryResult *performSearchQuery(neUserSearchQuery *query,
                                      neUser *targetUser);

    /*
      send each file in the result struct to the specified user
      returns 0 on success; 1 on failure
    */
    int sendQueryResults(neQueryResult *result, neUser *user);

  private:

    neQueryResult *buildQueryResult
        (std::vector< std::vector<neUserFile * > * > *vectorOfVectorsIter,
         std::string keyword);

    ncMutex m_keywordMapMutex;

    /*
      this data structure is a map that maps strings (keywords) to a
      vector of vector pointers.  Each sub-vector is a pointer to a 
      vector that already exists inside of the userFileList objects.
    */
    std::map< neKeyword *, std::vector< std::vector<neUserFile * > * > >
        m_keywordMap;
};

#endif /* __NEQUERYRESULTMANAGER_H */
