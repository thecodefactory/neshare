#ifndef __CONFIG_H
#define __CONFIG_H
@TOP@

#undef NE_MAJOR
#undef NE_MINOR
#undef NE_SUB
#undef NE_PRE

#undef NE_PROTOCOL_MAJOR
#undef NE_PROTOCOL_MINOR

#undef NESHARE_VERSION
#undef NESHARE_PROTOCOL_VERSION

#undef HAVE_THREADS

#undef NESHARE_DEBUG

/* Define this if you're running x86 architecture */
#undef __i386__

/* Define this if you're running x86 architecture */
#undef ARCH_X86

/* Define this if you're running Alpha architecture */
#undef __alpha__

/* Define this if you're running PowerPC architecture */
#undef ARCH_PPC

/* Define this if you're running Sparc architecture */
#undef __sparc__ 

/* Define this if you're running Mips architecture */
#undef __mips__ 

@BOTTOM@
/* Disable GCC compiler extensions, if gcc is not in use */
#ifndef __GNUC__
#define __attribute__(x)        /*empty*/
#endif

#endif /* __CONFIG_H */
