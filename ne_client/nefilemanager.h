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

#ifndef __NEFILEMANAGER_H
#define __NEFILEMANAGER_H

/*
  This file, by the very nature of dealing with the filesystem
  will require porting to other platforms. -N.M.
*/

typedef struct
{
    char encodedFilename[NEFILE_MAX_NAME_LEN+1];
    char realFilename[NEFILE_MAX_NAME_LEN+1];
    unsigned long fileSize;
} NESHARE_FILE_OBJ;

class neFileManager
{
  public:
    neFileManager();
    ~neFileManager();

    /* clears all memory associated with
       storing files in this file manager */
    void reset();

    /* adds a directory of files to the file manager.
       Each file in the specified directory is encoded
       into the neshare file manager file name format.
       If recurse is non-zero, all subdirectories will be added also.
       returns 0 on success; 1 on failure */
    int addDirectory(char *directory, int recurse);
   
    /* given an neshare file manager file name, this
       function fills in the filename array with up
       to maxlen bytes of the actual system file name.
       If maxlen is shorter than the system file name
       mapped, a value of -1 is returned.  Otherwise,
       0 is returned on success and 1 on failure */
    int lookupEncodedName(char *encodedName, char *filename, int maxlen);
   
    /* used for forward iteration loops in conjunction with 
       getNextEncodedFileObj() below.  This must be called once
       before repeatedly calling getNextEncodedFilename */
    void resetEncodedFileObjPtr();
   
    /* used for forward iteration loops.  The char pointer returned
       should not be stored for later use or de-allocated by the
       caller. returns NULL when there are no more encoded file names */
    NESHARE_FILE_OBJ *getNextEncodedFileObj();

    /* returns the number of files that this file manager is managing */
    int getNumFiles();

    /* removes the specified file obj from the filemanager */
    void removeEncodedFile(NESHARE_FILE_OBJ *fileObj);
   
  private:
    std::string getEncoded(char *basePath, char *filename);
    std::string formatBasePath(char *directory);

    std::map< std::string, NESHARE_FILE_OBJ * > m_encodedFilenames;
    std::map< std::string, NESHARE_FILE_OBJ * >::iterator
        m_encodedFilenamesIter;
};

#endif /* __NEFILEMANAGER_H */
