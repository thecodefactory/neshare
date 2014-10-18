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

#ifndef __NEUSERFILELIST_H
#define __NEUSERFILELIST_H

class neUserFileList
{
  public:
    neUserFileList();
    ~neUserFileList();

    /*
      files can only be added; never removed - it's better 
      to just flush the list for the specified user.
      returns 0 on success; 1 on failure
    */
    int addUserFile(neUserFile *userFile);

    /* empties all files in the file list */
    void empty();

    /* returns the number of user files that are in this list */
    unsigned long getNumUserFiles();

    /* returns the size of user files that are in this list */
    unsigned long getTotalSizeofUserFiles();

    /*
      returns the vector pointer of userFile objects that is mapped
      to the specified keyword.  Returns NULL on error.
    */
    std::vector<neUserFile *> *getUserFilesFromKeyword(char *keyword);

    /*
      resets the internal keyword map iterator.  Used in conjunction with
      the getNextKeyword method for forward iteration loops
    */
    void resetKeywordPtr();

    /*
      returns the keyword string pointed to by m_keywordToUserFileMapIter.
      returns NULL when there are no more strings
    */
    char *getNextKeyword();

    /* returns the number of keywords in the user file list map */
    int getNumKeywords();

  private:
    unsigned long m_numUserFiles;
    unsigned long m_totalSizeofUserFiles;

    /*
      emulates a multimap by mapping a single
      neKeyword obj to a list of userFile objects
    */
    std::map< neKeyword *, std::vector<neUserFile *> >
        m_keywordToUserFileMap;

    /* an iterator used for the getNextKeyword method */
    std::map< neKeyword *, std::vector<neUserFile *> >::iterator
        m_keywordToUserFileMapIter;
};

#endif /* __NEUSERFILELIST_H */
