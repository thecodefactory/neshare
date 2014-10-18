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

#ifndef __NEUSERMANAGERMANAGER_H
#define __NEUSERMANAGERMANAGER_H

class neUserManagerManager
{
  public:
    neUserManagerManager();
    ~neUserManagerManager();

    /*
      attempts to start numUserManagers user manager threads.
      returns the number of user manager threads actually created
    */
    int startManagers(int numUserManagers);

    int addUserToUserManager(int managerIndex, neUser *user);

    /* attempts to gracefully stop all managers */
    void stopManagers();

    /*
      forcefully terminates all manager threads and frees
      any claimed resources used by the manager threads
    */
    void cancelManagers();

    /*
      removes a user from the specified manager
      located at managerIndex
    */
    void removeUserFromUserManager(int managerIndex, neUser *user);

    /* returns the user manager at specified index */
    neUserManager *getUserManager(int managerIndex);

  private:
    int m_numManagers;
    std::vector<neUserManager *> m_threadedManagers;
    neServerStatus m_serverStatus;
};

#endif /* __NEUSERMANAGERMANAGER_H */
