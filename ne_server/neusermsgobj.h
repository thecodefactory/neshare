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

#ifndef __NEUSERMSGOBJ_H
#define __NEUSERMSGOBJ_H

class neUserMsgObj
{
  public:
    neUserMsgObj();
    ~neUserMsgObj();

    /*
      initializes this msgObj; on delete the msgCommon
      specified will be freed. returns 0 on success; 1 on failure
    */
    int initialize(neUser *user,
                   nemsgCommon *msgCommon,
                   unsigned long msgType,
                   char *errorMsg);

    neUser *getUser();
    nemsgCommon *getCommonMsg();
    unsigned long getMsgType();
    char *getErrorMsg();

  private:
    neUser *m_user;
    nemsgCommon *m_commonMsg;
    unsigned long m_msgType;
    char m_errorMsg[NE_MSG_MAX_ERROR_LEN];
};

#endif /* __NEUSERMSGOBJ_H */
