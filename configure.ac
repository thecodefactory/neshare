NE_MAJOR=0
NE_MINOR=3
NE_SUB=0
NE_PRE=""

AC_INIT([neshare], [0.3.0])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([ne_server/neshareserver.cpp])


dnl neshare 0.2.5 deprecates the use of autoconf 2.13
dnl in favor of autoconf 2.5x
AC_PREREQ(2.50)

NE_PROTOCOL_MAJOR=0
NE_PROTOCOL_MINOR=2
NE_PROTOCOL_SUB=1

AC_DEFINE_UNQUOTED(NE_PROTOCOL_VERSION,
        "$NE_PROTOCOL_MAJOR.$NE_PROTOCOL_MINOR.$NE_PROTOCOL_SUB")

AC_DEFINE_UNQUOTED(NE_VERSION,"$NE_MAJOR.$NE_MINOR.$NE_SUB")

AM_INIT_AUTOMAKE

dnl neshare 0.2.3 deprecates the use of ranlib in favor of libtool
dnl AC_PROG_RANLIB

AC_PROG_LIBTOOL
AC_STDC_HEADERS
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_CXX
AC_LANG_CPLUSPLUS

dnl
dnl Checks for typedefs, structures, and compiler characteristics.
dnl
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

AC_CHECK_SIZEOF(unsigned short, 2)
AC_CHECK_SIZEOF(unsigned int, 4)
AC_CHECK_SIZEOF(unsigned long, 4)


dnl ----------------------------------
dnl start netclass related checks here
dnl ----------------------------------
NETCLASS_PATH=""

AC_ARG_WITH(netclass,
            [  --with-netclass=DIR     path to installed netclass package],
            NETCLASS_PATH=$withval,
            NETCLASS_PATH="/usr/local")

AC_CHECK_FILE("$NETCLASS_PATH/include/netclass.h", ,
              AC_MSG_ERROR("cannot find netclass.h header file."))

AC_CHECK_FILE("$NETCLASS_PATH/lib/libnetclass.la", ,
             AC_MSG_ERROR("cannot find netclass library."))

AC_SUBST(NETCLASS_PATH)

dnl this is required if NetClass threads are desired
AC_DEFINE(HAVE_THREADS)

AC_CHECK_FUNCS(gethostbyname socket inet_aton inet_addr \
               inet_pton poll select strerror)
dnl ----------------------------------
dnl end netclass related checks here
dnl ----------------------------------

dnl
dnl Common cflags for all platforms
dnl
COMMON_CFLAGS="-D_REENTRANT"

dnl
dnl Check whether or not to use debugging
dnl
AC_ARG_ENABLE(debug,
        [  --enable-debug          Enable debug support in NEshare [default=no]],
        [ac_cv_enable_debug=$enableval], [ac_cv_enable_debug=no])
AC_MSG_CHECKING([whether to use debugging])
if test $ac_cv_enable_debug = yes; then
        AC_MSG_RESULT(yes)
        AC_DEFINE(NESHARE_DEBUG)
        CPPFLAGS="-ggdb $CPPFLAGS"
else
        AC_MSG_RESULT(no)
        CPPFLAGS="$CPPFLAGS"
fi

CFLAGS="$COMMON_FLAGS $CFLAGS"
CPPFLAGS="$COMMON_FLAGS $CPPFLAGS"

dnl
dnl pthread support
dnl
AC_CHECK_LIB(pthread, pthread_create,
             THREAD_LIBS="-lpthread",
             CC='cc -pthread'
             THREAD_LIBS="-L/usr/local/lib -pthread"
             THREAD_CFLAGS="-I/usr/local/include -D_THREAD_SAFE"
             CFLAGS="$CFLAGS -L/usr/local/lib $THREAD_CFLAGS"
             CPPFLAGS="$CPPFLAGS -I/usr/local/include -L/usr/local/lib"
             AC_MSG_RESULT("                               ")
             AC_MSG_RESULT("                               ")
             AC_MSG_RESULT("WARNING: Assuming host is a BSD variant")
             AC_MSG_RESULT("WARNING: Using -L/usr/local/lib -pthread")
             AC_MSG_RESULT("                               ")
             AC_MSG_RESULT("                               ")
             AC_MSG_RESULT("REMINDER: Use gmake to compile!")
             AC_MSG_RESULT("                               ")
             AC_MSG_RESULT("                               ")
)

AC_SUBST(THREAD_CFLAGS)
AC_SUBST(THREAD_LIBS)
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)


AC_MSG_CHECKING([for the answer to life the universe and everything])
AC_MSG_RESULT(42)


AC_OUTPUT([
           Makefile
           ne_common/Makefile
           ne_server/Makefile
           ne_client/Makefile
           ne_clienttest/Makefile
          ])
