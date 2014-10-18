ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = @CFLAGS@ @THREAD_CFLAGS@
AM_CPPFLAGS = -I@NETCLASS_PATH@/include \
           -I../ne_common/ @CPPFLAGS@

bin_PROGRAMS = neshareserver

EXTRA_DIST = config

neshareserver_FLAGS = ($CXXFLAGS)

neshareserver_SOURCES = nekeyword.cpp \
                        nekeywordmanager.cpp \
                        neuser.cpp \
                        neuserfile.cpp \
                        neuserfilelist.cpp \
                        nediskmemallocator.cpp \
			neserverstatus.cpp \
			neusersearchquery.cpp \
                        neusermanager.cpp \
			neusermanagermanager.cpp \
                        neusermsgobj.cpp \
                        nequeryresultmanager.cpp \
                        nethreadedhandlerobj.cpp \
                        nethreadedhandlermanager.cpp \
                        neserverutils.cpp \
                        neshareserverthreads.cpp \
                        neshareserver.cpp

include_HEADERS  = neuser.h \
                   neuserfile.h \
                   neuserfilelist.h \
		   neserverstatus.h \
                   neusermanager.h \
		   neusermanagermanager.h \
                   neusermsgobj.h \
                   nequeryresult.h \
                   nequeryresultmanager.h \
                   nethreadedhandlerobj.h \
                   nethreadedhandlermanager.h \
                   neshareserverthreads.h \
                   neserverheaders.h \
		   nekeyword.h \
		   nekeywordmanager.h \
		   nediskmemallocator.h \
		   neusersearchquery.h


neshareserver_LDADD = -L@NETCLASS_PATH@/lib -lnetclass @THREAD_LIBS@\
                      -L../ne_common/ -lnescommon
