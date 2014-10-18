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

#ifndef __NECLIENTHEADERS_H
#define __NECLIENTHEADERS_H

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <dirent.h>
#include <string>
#include <stdlib.h>
#include <pthread.h>

#include <vector>
#include <map>
#include <iterator>
#include <algorithm>

#ifdef VERSION
#define NESHARE_CLIENT_VERSION        VERSION
#else
#define NESHARE_CLIENT_VERSION       "Unknown"
#endif

#define CONFIG_MAX_PEER_ERROR_COUNT       5

/*
  limitation: no file length stored in this system plus the
  length of the absolute path can be more than 511 bytes long
*/ 
#define NEFILE_MAX_NAME_LEN      0x000001FF

/* valid return values for neClientConnection::initialize(...) */
#define NE_INIT_OK                        0
#define NE_INIT_FAILED                    1
#define NE_RESOURCE_INIT_FAILED           2
#define NE_CONFIG_NOT_FOUND               3

/* server/peer sync constants */
#define NE_SYNC_LOGIN                     0
#define NE_SYNC_DISCONNECT                1
#define NE_SYNC_TIMEOUT                   2
#define NE_SYNC_PEER_LOGIN                3
#define NE_SYNC_FILE_REQUEST_ACK          4

const unsigned long INVALID_FILE_REQUEST = 0xFFFFFFFF;

#include "necommonheaders.h"
#include "nemsgconsts.h"
#include "nemsgs.h"
#include "nefilemanager.h"
#include "nepeer.h"
#include "nepeerdownloadstatus.h"
#include "nepeerdownloadmanager.h"
#include "nepeeruploadstatus.h"
#include "nepeeruploadmanager.h"
#include "nepeermanager.h"
#include "neshareclientthreads.h"
#include "neclientconnection.h"


#endif /* __NECLIENTHEADERS_H */
