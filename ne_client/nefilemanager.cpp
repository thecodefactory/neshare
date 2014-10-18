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

#include "neclientheaders.h"

neFileManager::neFileManager()
{
}

neFileManager::~neFileManager()
{
    reset();
}

void neFileManager::reset()
{
    /*
      delete any remaining NESHARE_FILE_OBJ
      pointers in the filename map
    */
    NESHARE_FILE_OBJ *fileObj = (NESHARE_FILE_OBJ *)0;
    std::map< std::string, NESHARE_FILE_OBJ * >::iterator iter;
    for(iter = m_encodedFilenames.begin(); 
        iter != m_encodedFilenames.end(); iter++)
    {
        fileObj = (*iter).second;
        m_encodedFilenames.erase(iter);
        delete fileObj;
    }
    m_encodedFilenames.clear();
}

/*
  write the platform-dependent iteration through a directory.
  Basic Algorithm:
  For each element in the directory
  if the element is a directory (and recurse is non-zero)
  recursively call addDirectory
  if the element is a file
  stat the file and allocate a new NESHARE_FILE_OBJ
  encode the filename and fill in the OBJ
*/
int neFileManager::addDirectory(char *directory, int recurse)
{
    int ret = 1;
    int count = 0;
   
    /* get the base path directory (and format properly) */
    std::string basePath = formatBasePath(directory);

    struct dirent *dentry = NULL;
    DIR *dir = opendir(basePath.c_str());
    if (dir)
    {
        while((dentry = readdir(dir)))
        {
            if (dentry->d_name[0] == '.')
            {
                continue;
            }

            /*
              stat the file to get the file size and
              distinguish b/w a dir and regular file
            */
            struct stat statbuf;
            memset(&statbuf,0,sizeof(statbuf));
            std::string direntry = basePath +
                std::string(dentry->d_name);

            if (stat((char *)direntry.c_str(),&statbuf))
            {
                continue;
            }

            /* handle directory entry */
            if (S_ISDIR(statbuf.st_mode))
            {
                if (recurse)
                {
                    /* adjust the basePath and recurse */
                    std::string newBasePath = basePath +
                        std::string(dentry->d_name);
                    ret = addDirectory((char *)newBasePath.c_str(),
                                       recurse);
                }

                /* count directories as files as well */
                count++;
            }
            else /* handle file entry */
            {
                /* format the filename */
                std::string filename = getEncoded(
                    (char *)basePath.c_str(),dentry->d_name);
         
                /* create a new file entry to map */
                NESHARE_FILE_OBJ *newFileObj = new NESHARE_FILE_OBJ();
                if ((basePath.length() + strlen(dentry->d_name)) <
                    NEFILE_MAX_NAME_LEN)
                {
                    newFileObj->fileSize = (unsigned long)statbuf.st_size;
                    strcpy(newFileObj->realFilename,basePath.c_str());
                    strcat(newFileObj->realFilename,dentry->d_name);
                    strcpy(newFileObj->encodedFilename,filename.c_str());

                    /* finally, insert the file into the map */
                    m_encodedFilenames[filename] = newFileObj;
                    count++;
                }
                else
                {
                    iprintf("Filename too long.  Skipping %s\n",
                            dentry->d_name);
                    delete newFileObj;
                    continue;
                }
            }
        }
        closedir(dir);
        ret = (((count > 0) || (ret == 0)) ? 0 : 1);
    }
    return ret;
}

int neFileManager::lookupEncodedName(char *encodedName, 
                                     char *filename, 
                                     int maxlen)
{
    int ret = 1;
    int len = MIN(maxlen,(int)NEFILE_MAX_NAME_LEN);
    std::map< std::string, NESHARE_FILE_OBJ * >::iterator iter;

    iter = m_encodedFilenames.find(std::string(encodedName));
    if (iter != m_encodedFilenames.end())
    {
        len = MIN(len,(int)strlen(((*iter).second)->realFilename));
        memcpy(filename,((*iter).second)->realFilename,
               (size_t)len*sizeof(char));
        filename[len] = '\0';
        ret = 0;
    }
    return ret;
}
    
void neFileManager::resetEncodedFileObjPtr()
{
    m_encodedFilenamesIter = m_encodedFilenames.begin();
}

NESHARE_FILE_OBJ *neFileManager::getNextEncodedFileObj()
{
    NESHARE_FILE_OBJ *ret = (NESHARE_FILE_OBJ *)0;
    if (m_encodedFilenamesIter != m_encodedFilenames.end())
    {
        ret = (*m_encodedFilenamesIter).second;
        m_encodedFilenamesIter++;
    }
    return ret;
}

/* 
   NOTE: When this is called, this function makes the following
   assumptions:

   0) "basePath" begins and ends with a slash (i.e. '/' or '\\')
   1) "filename" DOES NOT BEGIN with a slash (i.e. '/' or '\\')
   2) "filename" contains only the relative path name
   (i.e. absolute path minus the base path)
*/
std::string neFileManager::getEncoded(char *basePath, char *filename)
{
    return std::string("neshare:/" + std::string(basePath) +
                       std::string(filename));
}

/* 
   given a valid directory, this function has to make sure
   it is in the proper neshare format - which is:

   *NIX:  /basepath/ OR
   WIN32: \\basepath\\  (currently unsupported)

   (i.e. ensure the path begins and ends with a slash
*/
std::string neFileManager::formatBasePath(char *directory)
{
    std::string ret = std::string(directory);
    int len = strlen(directory);

    /* if no trailing slash is present, append one */
    if (directory[len-2] != '/')
    {
        ret += "/";
    }

    /* if no leading slash is present, prepend one */
    if (directory[0] != '/')
    {
        std::string tmp = ret;
        ret = "/" + tmp;
    }
    return ret;
}

int neFileManager::getNumFiles()
{
    return m_encodedFilenames.size();
}

void neFileManager::removeEncodedFile(NESHARE_FILE_OBJ *fileObj)
{
    std::map< std::string, NESHARE_FILE_OBJ * >::iterator iter;
    iter = m_encodedFilenames.find(std::string(fileObj->encodedFilename));
    if (iter != m_encodedFilenames.end())
    {
        iprintf("Found file: Removing %s",fileObj->encodedFilename);
        m_encodedFilenames.erase(iter);
        delete fileObj;
        fileObj = (NESHARE_FILE_OBJ *)0;
    }
}
