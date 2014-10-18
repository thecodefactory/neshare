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

#include "necommonheaders.h"

static unsigned long s_logLevelList[NESYSLOG_LEVEL_COUNT] = 
{
    0x00000000,
    NESYSLOG_LEVEL_DEBUG,
    NESYSLOG_LEVEL_INFO,
    0x00000000,
    NESYSLOG_LEVEL_ERROR
};

neSysLog::neSysLog()
{
    std::vector<ACTION_FUNC> *debugVec = new std::vector<ACTION_FUNC>();
    std::vector<ACTION_FUNC> *infoVec = new std::vector<ACTION_FUNC>();
    std::vector<ACTION_FUNC> *errorVec = new std::vector<ACTION_FUNC>();

    assert(debugVec);
    assert(infoVec);
    assert(errorVec);

    m_callbackMap[0] = NULL;
    m_callbackMap[NESYSLOG_LEVEL_DEBUG] = debugVec;
    m_callbackMap[NESYSLOG_LEVEL_INFO]  = infoVec;
    m_callbackMap[3] = NULL;
    m_callbackMap[NESYSLOG_LEVEL_ERROR] = errorVec;
}

neSysLog::~neSysLog()
{
    std::vector<ACTION_FUNC> *vec = NULL;

    for(unsigned long i = 0; i < NESYSLOG_LEVEL_COUNT; i++)
    {
        vec = m_callbackMap[i];
        if (vec)
        {
            vec->clear();
            delete vec;
            vec = NULL;
        }
        m_callbackMap[i] = NULL;
    }
}

int neSysLog::registerListener(unsigned long levels,
                               actionFunc callback,
                               void *data)
{
    int ret = 1;
    ACTION_FUNC af;
    std::vector<ACTION_FUNC> *vec = NULL;

    if (callback)
    {
        for(unsigned long i = 0; i < NESYSLOG_LEVEL_COUNT; i++)
        {
            if (levels & s_logLevelList[i])
            {
                vec = m_callbackMap[i];
                assert(vec);

                af.cb = callback;
                af.data = data;
                vec->push_back(af);
            }
        }
        ret = 0;
    }
    return ret;
}

void neSysLog::unregisterListener(unsigned long levels,
                                  actionFunc callback,
                                  void *data)
{
    if (callback)
    {
        std::vector<ACTION_FUNC> *vec = NULL;
        std::vector<ACTION_FUNC>::iterator vecIter;
        std::vector<ACTION_FUNC>::iterator tmpVecIter;

        for(unsigned long i = 0; i < NESYSLOG_LEVEL_COUNT; i++)
        {
            if (levels & s_logLevelList[i])
            {
                vec = m_callbackMap[i];
                assert(vec);

                for(vecIter = vec->begin(); vecIter != vec->end(); )
                {
                    if ((*vecIter).cb == callback)
                    {
                        /*
                          if data is specified, it must match; if not the
                          match on the callback is enough for id purposes
                        */
                        if (!data || ((*vecIter).data == data))
                        {
                            tmpVecIter = vecIter++;
                            vec->erase(tmpVecIter);

                            /*
                              once we've found the matching one and removed
                              it, continue to next
                            */
                            break;
                        }
                    }
                    else
                    {
                        vecIter++;
                    }
                }
            }
        }
    }
}

void neSysLog::output(unsigned long levels, char *logmessage)
{
    std::vector<ACTION_FUNC> *vec = NULL;
    std::vector<ACTION_FUNC>::iterator vecIter;

    for(unsigned long i = 0; i < NESYSLOG_LEVEL_COUNT; i++)
    {
        if (levels & s_logLevelList[i])
        {
            vec = m_callbackMap[i];
            assert(vec);

            for(vecIter = vec->begin(); vecIter != vec->end(); vecIter++)
            {
                (*((*vecIter).cb))((*vecIter).data, logmessage);
            }
        }
    }
}
