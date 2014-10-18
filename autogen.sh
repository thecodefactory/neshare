#!/bin/sh

export WANT_AUTOCONF_2_5="1"

LIBTOOL=`which libtoolize`
if [ -z "${LIBTOOL}" ]; then
    echo "Please make sure libtool is installed."
    exit 1
fi

aclocal
#CMD="libtoolize -i -c --force --copy 2&>/dev/null"
CMD=$(libtoolize -i -c --force --copy >/dev/null)

if [ ! -z "${CMD}" ]; then
   echo "If you see an error that looks like:"
   echo "... AM_PROG_LIBTOOL not found in library ..."
   echo "Please make sure libtool is installed."
   echo "If you're running OpenBSD, install /usr/ports/devel/libtool"
   exit 1
fi

autoreconf -i && \
automake --add-missing --copy && \
autoconf && \
./configure $@
