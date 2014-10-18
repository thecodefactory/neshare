ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = @CFLAGS@ @THREAD_CFLAGS@

AM_CPPFLAGS = @CPPFLAGS@ -I@NETCLASS_PATH@/include

lib_LTLIBRARIES = libnescommon.la

libnescommon_la_SOURCES = md5.c \
                          nemsgerror.cpp \
                          nemsglogin.cpp \
                          nemsgloginack.cpp \
                          nemsgdisconnect.cpp \
                          nemsgdisconnectack.cpp \
                          nemsgloginfailed.cpp \
                          nemsgentrysetstart.cpp \
                          nemsgentry.cpp \
                          nemsgentrysetend.cpp \
                          nemsgforceddisconnect.cpp \
                          nemsgping.cpp \
                          nemsgpong.cpp \
                          nemsgstatus.cpp \
                          nemsgsearchquery.cpp \
                          nemsgsearchresults.cpp \
                          nemsgqueryresult.cpp \
                          nemsgpeererror.cpp \
                          nemsgpeerlogin.cpp \
                          nemsgpeerloginack.cpp \
                          nemsgpeerdisconnect.cpp \
                          nemsgpeerloginfailed.cpp \
                          nemsgfilerequest.cpp \
                          nemsgfilerequestack.cpp \
                          nemsgpushrequest.cpp \
			  nemsgpushrequestack.cpp \
                          nemsgfiledatasend.cpp \
                          nemsgfiledatasendend.cpp \
                          nemsgfiledatacancel.cpp \
                          nemsgfileresume.cpp \
                          nemsgfileresumeack.cpp \
                          neutils.cpp \
                          neconfig.cpp \
                          nesyslog.cpp \
                          xprintf.cpp

include_HEADERS = md5.h \
                  nemsgerror.h \
                  nemsglogin.h \
                  nemsgloginack.h \
                  nemsgdisconnect.h \
                  nemsgdisconnectack.h \
                  nemsgloginfailed.h \
                  nemsgentrysetstart.h \
                  nemsgentry.h \
                  nemsgentrysetend.h \
                  nemsgforceddisconnect.h \
                  nemsgping.h \
                  nemsgpong.h \
                  nemsgstatus.h \
                  nemsgsearchquery.h \
                  nemsgsearchresults.h \
                  nemsgqueryresult.h \
                  nemsgpeererror.h \
                  nemsgpeerlogin.h \
                  nemsgpeerloginack.h \
                  nemsgpeerdisconnect.h \
                  nemsgpeerloginfailed.h \
                  nemsgfilerequest.h \
                  nemsgfilerequestack.h \
                  nemsgpushrequest.h \
                  nemsgpushrequestack.h \
                  nemsgfiledatasend.h \
                  nemsgfiledatasendend.h \
                  nemsgfiledatacancel.h \
                  nemsgfileresume.h \
                  nemsgfileresumeack.h \
                  neutils.h \
                  neconfig.h \
                  nesyslog.h \
                  xprintf.h \
                  necommonheaders.h \
                  nemsgconsts.h \
                  nemsgs.h \
                  nemsgcommon.h
