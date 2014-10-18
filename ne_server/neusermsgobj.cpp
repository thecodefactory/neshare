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

#ifdef NESHARE_DEBUG
extern unsigned long numNewMsgObjs;
extern unsigned long numDeletedMsgObjs;
extern unsigned long totalBytesAllocated;
extern unsigned long totalBytesFreed;
#endif

#include "neserverheaders.h"

neUserMsgObj::neUserMsgObj()
{
    m_user = (neUser *)0;
    m_commonMsg = (nemsgCommon *)0;
    m_msgType = 0;
    memset(m_errorMsg,0,NE_MSG_MAX_ERROR_LEN);

#ifdef NESHARE_DEBUG
    numNewMsgObjs++;
    totalBytesAllocated += sizeof(neUserMsgObj);
#endif
}

neUserMsgObj::~neUserMsgObj()
{
    delete m_commonMsg;

#ifdef NESHARE_DEBUG
    numDeletedMsgObjs++;
    totalBytesFreed += sizeof(nemsgCommon);
    totalBytesFreed += sizeof(neUserMsgObj);
#endif

    m_user = (neUser *)0;
    m_msgType = 0;
    m_errorMsg[0] = '\0';
}

int neUserMsgObj::initialize(neUser *user,
                             nemsgCommon *commonMsg,
                             unsigned long msgType,
                             char *errorMsg)
{
    int ret = 1;
    if (user)
    {
        m_user = user;
        m_commonMsg = commonMsg;
        m_msgType = msgType;

        if (errorMsg)
        {
            memcpy(m_errorMsg,errorMsg,
                   MIN(NE_MSG_MAX_ERROR_LEN-1,strlen(errorMsg)));
        }
        ret = 0;
    }
    return ret;
}

neUser *neUserMsgObj::getUser()
{
    return m_user;
}

nemsgCommon *neUserMsgObj::getCommonMsg()
{
    return m_commonMsg;
}

unsigned long neUserMsgObj::getMsgType()
{
    return m_msgType;
}

char *neUserMsgObj::getErrorMsg()
{
    return m_errorMsg;
}
