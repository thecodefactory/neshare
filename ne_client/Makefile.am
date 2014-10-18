ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = @CFLAGS@ @THREAD_CFLAGS@

AM_CPPFLAGS = @CPPFLAGS@ -I@NETCLASS_PATH@/include \
           -I../ne_common/


lib_LTLIBRARIES = libnesclient.la

libnesclient_la_SOURCES = nepeer.cpp \
                          nefilemanager.cpp \
                          nepeerdownloadmanager.cpp \
                          nepeerdownloadstatus.cpp \
                          nepeeruploadmanager.cpp \
                          nepeeruploadstatus.cpp \
                          nepeermanager.cpp \
                          neclientconnection.cpp \
                          neshareclientthreads.cpp \
                          neclientutils.cpp

include_HEADERS = nepeer.h \
                  nefilemanager.h \
                  nepeerdownloadmanager.h \
                  nepeerdownloadstatus.h \
                  nepeeruploadmanager.h \
                  nepeeruploadstatus.h \
                  nepeermanager.h \
                  neclientconnection.h \
                  neshareclientthreads.h \
                  neclientheaders.h
