ACLOCAL_AMFLAGS = -I m4

noinst_PROGRAMS = neshare_client

neshare_client_SOURCES = neshare_client.cpp

neshare_client_FLAGS = ($CXXFLAGS)

AM_CFLAGS = @CFLAGS@ @THREAD_CFLAGS@

AM_CPPFLAGS = @CPPFLAGS@ -I@NETCLASS_PATH@/include \
           -I../ne_common \
           -I../ne_client

neshare_client_LDADD = -L../ne_client -lnesclient @THREAD_LIBS@ \
                       -L@NETCLASS_PATH@/lib -lnetclass \
                       -L../ne_common -lnescommon

