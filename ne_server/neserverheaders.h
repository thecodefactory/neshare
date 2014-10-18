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

#ifndef __NESERVERHEADERS_H
#define __NESERVERHEADERS_H

/*
  all headers used for server files are placed here to clean up the
  original messy and scattered header situation
*/

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <iterator>
#include <algorithm>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "necommonheaders.h"
#include "nekeyword.h"
#include "nekeywordmanager.h"
#include "nediskmemallocator.h"
#include "neserverstatus.h"
#include "neusersearchquery.h"
class neUser;
class neUserFile;
class neUserFileList;
#include "neusermanager.h"
#include "neusermanagermanager.h"
#include "neuser.h"
#include "neuserfile.h"
#include "neuserfilelist.h"
#include "neusermsgobj.h"
#include "nequeryresult.h"
#include "nethreadedhandlerobj.h"
#include "nethreadedhandlermanager.h"
#include "nequeryresultmanager.h"
#include "neshareserverthreads.h"

#define CONFIG_MAX_USER_ERROR_COUNT  5

/* customizeable default messages */
#define CONFIG_DEFAULT_GREETING_MSG "Welcome to our NEshare server!\n"


#endif /* __NESERVERHEADERS_H */
