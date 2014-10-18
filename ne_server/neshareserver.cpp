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

#include "neserverheaders.h"
#include <signal.h>

/* globally defined constants */
const char *NESHARE_SERVER_LOGFILE = "neshare.log";

/* global variables used for this server implementation */
neConfig g_config;
neSysLog g_syslog;
neDiskMemAllocator g_userFileAllocator;
neKeywordManager g_keywordManager;
neUserManager *g_userManager = (neUserManager *)0;
neQueryResultManager *g_queryResultManager = (neQueryResultManager *)0;
ncSocketListener *g_nsl = (ncSocketListener *)0;
static pid_t parentPid = getpid();
FILE *g_logfd = (FILE *)0;
ncThread g_cleanupThread;
static ncMutex g_cleanedSyncMutex;

/* Debugging related variables here */
#ifdef NESHARE_DEBUG
unsigned long numNewMsgObjs = 0;
unsigned long numDeletedMsgObjs = 0;
unsigned long numNewUserFiles = 0;
unsigned long numDeletedUserFiles = 0;
unsigned long numNewUserFileLists = 0;
unsigned long numDeletedUserFileLists = 0;
unsigned long numNewUserFileVectors = 0;
unsigned long numDeletedUserFileVectors = 0;
unsigned long numNewUsers = 0;
unsigned long numDeletedUsers = 0;
unsigned long numNewUserManagers = 0;
unsigned long numDeletedUserManagers = 0;
unsigned long numNewThreadedHandlerObjs = 0;
unsigned long numDeletedThreadedHandlerObjs = 0;
unsigned long numNewQueryResults = 0;
unsigned long numDeletedQueryResults = 0;
unsigned long numNewQueryResultManagers = 0;
unsigned long numDeletedQueryResultManagers = 0;
unsigned long numNewUserManagerManagers = 0;
unsigned long numDeletedUserManagerManagers = 0;

unsigned long totalBytesAllocated = 0;
unsigned long totalBytesFreed = 0;
#endif

/* signal handlers */
void sigint_handler(int signo)
{
   if (getpid() == parentPid)
   {
      if (signo == SIGINT)
      {
         /* make sure this is only ever run once */
         if (g_cleanedSyncMutex.trylock() == NC_OK)
         {
            iprintf("***\tServer received SIGINT.  Shutting down.\n");
            iprintf("***\tStopping incoming connection listener.\n");

            g_nsl->stopListening();

            iprintf("***\tStopping all server threads.\n");

            neShareServerThreads::stopThreads();

            iprintf("***\tServer Terminating.\n");

            delete g_nsl;
            g_nsl = (ncSocketListener *)0;

#ifdef NESHARE_DEBUG
            totalBytesFreed += sizeof(ncSocketListener);
#endif

#ifdef NESHARE_DEBUG
            dprintf("MEM STAT -- User Objects     : "
                    "Allocated %lu | Freed %lu\n",
                    numNewUsers,numDeletedUsers);
            dprintf("MEM STAT -- User Msgs        : "
                    "Allocated %lu | Freed %lu\n",
                    numNewMsgObjs,numDeletedMsgObjs);
            dprintf("MEM STAT -- User Files       : "
                    "Allocated %lu | Freed %lu\n",
                    numNewUserFiles,numDeletedUserFiles);
            dprintf("MEM STAT -- User File Lists  : "
                    "Allocated %lu | Freed %lu\n",
                    numNewUserFileLists,numDeletedUserFileLists);
            dprintf("MEM STAT -- User File Vectors: "
                    "Allocated %lu | Freed %lu\n",
                    numNewUserFileVectors,numDeletedUserFileVectors);
            dprintf("MEM STAT -- Query Results    : "
                    "Allocated %lu | Freed %lu\n",
                    numNewQueryResults,numDeletedQueryResults);
            dprintf("MEM STAT -- User Managers    : "
                    "Allocated %lu | Freed %lu\n",
                    numNewUserManagers,numDeletedUserManagers);
            dprintf("MEM STAT -- Threaded Handlers: "
                    "Allocated %lu | Freed %lu\n",
                    numNewThreadedHandlerObjs,
                    numDeletedThreadedHandlerObjs);
            dprintf("MEM STAT -- Approx Total Byte: "
                    "Allocated %lu | Freed %lu\n",
                    totalBytesAllocated,totalBytesFreed);
#endif
            fflush(g_logfd);
            fclose(g_logfd);

            exit(0);
         }
      }
   }
}

/* syslog callbacks/handlers */
void printDebug(void *data, char *msg)
{
#ifdef NESHARE_DEBUG
   printf("(D) %s",msg);
#endif
}

void printInfo(void *data, char *msg)
{
   char timestr[16] = {0};
   neUtils::getFormattedTime(timestr);

   /* info messages go to logfile */
   fwrite(timestr, sizeof(char), strlen(timestr), g_logfd);
   fwrite(" (I) ", sizeof(char), 5, g_logfd);
   fwrite(msg, sizeof(char), strlen(msg), g_logfd);
   fflush(g_logfd);
}

void printError(void *data, char *msg)
{
   char timestr[16] = {0};
   neUtils::getFormattedTime(timestr);

   /* error messages go to logfile */
   fwrite(timestr, sizeof(char), strlen(timestr), g_logfd);
   fwrite(" (E) ", sizeof(char), 5, g_logfd);
   fwrite(msg, sizeof(char), strlen(msg), g_logfd);
   fflush(g_logfd);

   /* mirror the same messages to stderr */
   fprintf(stderr,"%s (E) %s",timestr,msg);
}


/* entry point of the server */
int main(int argc, char **argv)
{
   /* parse command line options */


   /*
     parse config file -- FIXME: config location precedence should be:
     1) command line option, 2) environment variable, 3 default
   */
   char *configfilename = (getenv("NESHARESERVERCONFIG") ?
                           getenv("NESHARESERVERCONFIG") :
                           (char *)"config");
   printf("***\tReading configuration file: %s\n",configfilename);
   if (g_config.readConfigFile(configfilename))
   {
      fprintf(stderr,"Fatal Error: cannot find config file.\n");

      g_config.writeConfigFile(configfilename,NECFG_SERVER_FIELDS);
      fprintf(stderr,"A default config file has been written to \"%s\",\n"
              "however you should edit it to make sure it makes sense.\n",
              configfilename);
      fprintf(stderr,"Program terminating.\n");
      exit(1);
   }
   
   /* open the logfile for append */
   char *logfilename = (g_config.getServerLog() ?
                        g_config.getServerLog() :
                        (char *)NESHARE_SERVER_LOGFILE);
   printf("***\tOpening log file: %s\n",logfilename);
   if ((g_logfd = fopen(logfilename, "a")) == NULL)
   {
      fprintf(stderr, "Cannot open specified logfile %s."
              "Program terminating. \n", logfilename);
      exit(1);
   }
   printf("***\tRedirecting output to log file\n");

   /* hook up the syslog callbacks */
   printf("***\tHooking up syslog callbacks\n");
   g_syslog.registerListener(NESYSLOG_LEVEL_DEBUG,printDebug,
                             (void *)0x00000000);
   g_syslog.registerListener(NESYSLOG_LEVEL_INFO,printInfo,
                             (void *)0x00000000);
   g_syslog.registerListener(NESYSLOG_LEVEL_ERROR,printError,
                             (void *)0x00000000);

   /* set up signal handler */
   iprintf("***\tInstalling signal handler\n");
   if (signal(SIGINT,sigint_handler) == SIG_ERR)
   {
      eprintf("Warning: signal handler cannot be set.\n");
      eprintf("Graceful shutdown on signaled termination "
              "is out of the question.\n");
   }

   /* prepare to start listening for incoming connections */
   iprintf("***\tOpening listening server socket\n");
   g_nsl = new ncSocketListener(g_config.getServerAcceptPort(),
                                SOCKTYPE_TCPIP);
   if (!g_nsl)
   {
      eprintf("Fatal error: Cannot allocate required socket listener.\n");
      exit(1);
   }
#ifdef NESHARE_DEBUG
   totalBytesAllocated += sizeof(ncSocketListener);
#endif

   /* initialize server objects and start server threads */
   iprintf("***\tstarting server threads\n");
   neShareServerThreads::startThreads();

   struct timeval tv = {0,0};
   gettimeofday(&tv, 0);
   iprintf("NEshare Server version %s is running normally\n\t\t"
           "-- started on %s",NE_VERSION,
           ctime((time_t *)&tv.tv_sec));
   iprintf("NEshare Protocol version %s\n",NE_PROTOCOL_VERSION);

   if (g_nsl->startListening(neShareServerThreads::processLoginMessage, 
                             NC_NONTHREADED,
                             NC_REUSEADDR) != NC_OK)
   {
      eprintf("The NEshare Server has "
              "mysteriously stopped running.\nNo more incoming "
              "connections are allowed.\nServer terminating.\n");
#ifdef NESHARE_DEBUG
      totalBytesFreed += sizeof(ncSocketListener);
#endif
      delete g_nsl;
      g_nsl = (ncSocketListener *)0;
   }
   return 0;
}
