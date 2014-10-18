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

#ifndef __NEQUERYRESULT_H
#define __NEQUERYRESULT_H

#ifdef NESHARE_DEBUG
extern unsigned long numNewQueryResults;
extern unsigned long numDeletedQueryResults;
extern unsigned long totalBytesAllocated;
extern unsigned long totalBytesFreed;
#endif


class neQueryResult
{
  public:
    neQueryResult()
    {
        files.clear();
        assert(files.empty());
#ifdef NESHARE_DEBUG
        numNewQueryResults++;
        totalBytesAllocated += sizeof(neQueryResult);
#endif
    }

    ~neQueryResult()
    {
        files.resize(0);
        assert(files.empty());
#ifdef NESHARE_DEBUG
        numDeletedQueryResults++;
        totalBytesFreed += sizeof(neQueryResult);
#endif
    }

    int addUserFile(neUserFile *userFile)
    {
        int ret = 1;
        if (userFile)
        {
            files.push_back(userFile);
            ret = 0;
        }
        return ret;
    }

    std::vector<neUserFile *> *getFiles()
    {
        return &files;
    }

  private:
    std::vector<neUserFile *>files;
};

#endif /* __NEQUERYRESULT_H */
