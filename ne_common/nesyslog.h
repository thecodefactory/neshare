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

#ifndef __NESYSLOG_H
#define __NESYSLOG_H

const unsigned long NESYSLOG_MAX_LINELEN             = 0x00000200;

/* valid logging levels */
const unsigned long NESYSLOG_LEVEL_DEBUG             = 0x00000001;
const unsigned long NESYSLOG_LEVEL_INFO              = 0x00000002;
const unsigned long NESYSLOG_LEVEL_ERROR             = 0x00000004;
const unsigned long NESYSLOG_LEVEL_COUNT             = 0x00000005;

typedef void (*actionFunc)(void *data, char *msg);

typedef struct
{
    actionFunc cb;
    void *data;
} ACTION_FUNC;


class neSysLog
{
  public:
    neSysLog();
    ~neSysLog();

    /*
      registers a callback which is called when a
      message is output at the specified level(s).
      the syslog levels can be ORed together which allows the
      same callback to be called for multiple levels of
      outputted messages.  data may be any data pointer and
      it will be passed to the callback each time it is called.
      returns 1 on error; 0 on success
    */
    int registerListener(unsigned long levels,
                         actionFunc callback,
                         void *data);

    /*
      unregisters a callback previously registered that matches
      both the registered callback and the associated data
    */
    void unregisterListener(unsigned long levels,
                            actionFunc callback,
                            void *data);

    /*
      calls all callbacks on all listeners at the specified levels
      with the log message specified.
    */
    void output(unsigned long levels, char *logmessage);

  private:
    std::vector<ACTION_FUNC> *m_callbackMap[NESYSLOG_LEVEL_COUNT];
};

#endif /* __NESYSLOG_H */
