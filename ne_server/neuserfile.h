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

#ifndef __NEUSERFILE_H
#define __NEUSERFILE_H

class neUserFile
{
  public:
    /* the user passed in is the 'owner' of this object */
    neUserFile(neUser *user);
    ~neUserFile();

    /*
      breaks the filename up into keywords;
      returns 0 on success; 1 on failure
    */
    int initialize(char *filename, unsigned long filesize);
 
    /* returns the number of keywords this file contains */
    int getNumKeywords();

    /* returns a keyword ptr at the specified index */
    std::string *getKeyword(int index);

    neKeyword *getKeywordObj(int index);

    /* returns the file name */
    char *getFilename();

    /* returns the file size */
    unsigned long getFilesize();

    /* returns the neUser object that owns this object */
    neUser *getOwner();

  private:
    unsigned long m_filesize;
    neUser *m_user;
    char m_filename[NE_MSG_MAX_DATA_LEN];
    std::vector<neKeyword *> m_keywords;    
};

#endif /* __NEUSERFILE_H */
