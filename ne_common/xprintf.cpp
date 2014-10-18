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

extern neSysLog GLOBAL_SYSLOG_OBJ;
extern FILE    *GLOBAL_SYSLOG_FD;

#define MAX_LOGLINE_LEN  512

void dprintf(char *format, ...)
{
    char buf[MAX_LOGLINE_LEN] = {0};
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(buf, MAX_LOGLINE_LEN, format, argptr);
    va_end(argptr);

    GLOBAL_SYSLOG_OBJ.output(NESYSLOG_LEVEL_DEBUG, buf);
}

void iprintf(char *format, ...)
{
    char buf[MAX_LOGLINE_LEN] = {0};
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(buf, MAX_LOGLINE_LEN, format, argptr);
    va_end(argptr);

    GLOBAL_SYSLOG_OBJ.output(NESYSLOG_LEVEL_INFO, buf);
}

void eprintf(char *format, ...)
{
    char buf[MAX_LOGLINE_LEN] = {0};
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(buf, MAX_LOGLINE_LEN, format, argptr);
    va_end(argptr);

    GLOBAL_SYSLOG_OBJ.output(NESYSLOG_LEVEL_ERROR, buf);
}
