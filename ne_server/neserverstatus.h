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

#ifndef __NESERVERSTATUS_H
#define __NESERVERSTATUS_H

class neServerStatus
{
  public:
    neServerStatus();
    ~neServerStatus();

    unsigned long getNumUsers();
    unsigned long getNumUserFiles();
    unsigned long getTotalSizeofFilesInMB();

    void incNumUsers(int numUsers);
    void incNumUserFiles(int numUserFiles);
    void incTotalSizeofFilesInMB(int numMB);

    void decNumUsers(int numUsers);
    void decNumUserFiles(int numUserFiles);
    void decTotalSizeofFilesInMB(int numMB);

  private:

    ncMutex m_mutex;
    unsigned long m_totalNumUsers;
    unsigned long m_totalNumUserFiles;
    unsigned long m_totalSizeofFilesInMB;
};

#endif /* __NESERVERSTATUS_H */
