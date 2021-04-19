
#ifndef MANTID_ICAT_DLL_H
#define MANTID_ICAT_DLL_H

#ifdef MANTID_ICAT_STATIC_DEFINE
#  define MANTID_ICAT_DLL
#  define MANTID_ICAT_NO_EXPORT
#else
#  ifndef MANTID_ICAT_DLL
#    ifdef ICat_EXPORTS
        /* We are building this library */
#      define MANTID_ICAT_DLL __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MANTID_ICAT_DLL __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MANTID_ICAT_NO_EXPORT
#    define MANTID_ICAT_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MANTID_ICAT_DEPRECATED
#  define MANTID_ICAT_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MANTID_ICAT_DEPRECATED_EXPORT
#  define MANTID_ICAT_DEPRECATED_EXPORT MANTID_ICAT_DLL MANTID_ICAT_DEPRECATED
#endif

#ifndef MANTID_ICAT_DEPRECATED_NO_EXPORT
#  define MANTID_ICAT_DEPRECATED_NO_EXPORT MANTID_ICAT_NO_EXPORT MANTID_ICAT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MANTID_ICAT_NO_DEPRECATED
#    define MANTID_ICAT_NO_DEPRECATED
#  endif
#endif

#ifndef UNUSED_ARG
    #define UNUSED_ARG(x) (void) x;
#endif

#ifndef ICAT_DEPRECATED
    #define ICAT_DEPRECATED(func) MANTID_ICAT_DEPRECATED func
#endif

// Use extern keyword in client code to suppress class template instantiation
#include "MantidKernel/System.h"

#ifdef ICAT_EXPORTS
#define EXTERN_MANTID_ICAT
#else
#define EXTERN_MANTID_ICAT EXTERN_IMPORT
#endif /* ICAT_EXPORTS*/
 
#endif
