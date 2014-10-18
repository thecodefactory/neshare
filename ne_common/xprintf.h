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

#ifndef __XPRINTF_H
#define __XPRINTF_H

/* assumes these symbols are available somewhere */
#define GLOBAL_SYSLOG_OBJ g_syslog
#define GLOBAL_SYSLOG_FD  g_logfd

/*
  helper functions for logging (allows a printf style interface
  for generic log messages).
*/
void dprintf(char *format, ...);
void iprintf(char *format, ...);
void eprintf(char *format, ...);

#endif /* __XPRINTF_H */
