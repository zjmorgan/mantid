
#ifndef MANTID_INDEXING_DLL_H
#define MANTID_INDEXING_DLL_H

#ifdef MANTID_INDEXING_STATIC_DEFINE
#  define MANTID_INDEXING_DLL
#  define MANTID_INDEXING_NO_EXPORT
#else
#  ifndef MANTID_INDEXING_DLL
#    ifdef Indexing_EXPORTS
        /* We are building this library */
#      define MANTID_INDEXING_DLL __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MANTID_INDEXING_DLL __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MANTID_INDEXING_NO_EXPORT
#    define MANTID_INDEXING_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MANTID_INDEXING_DEPRECATED
#  define MANTID_INDEXING_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MANTID_INDEXING_DEPRECATED_EXPORT
#  define MANTID_INDEXING_DEPRECATED_EXPORT MANTID_INDEXING_DLL MANTID_INDEXING_DEPRECATED
#endif

#ifndef MANTID_INDEXING_DEPRECATED_NO_EXPORT
#  define MANTID_INDEXING_DEPRECATED_NO_EXPORT MANTID_INDEXING_NO_EXPORT MANTID_INDEXING_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MANTID_INDEXING_NO_DEPRECATED
#    define MANTID_INDEXING_NO_DEPRECATED
#  endif
#endif

#ifndef UNUSED_ARG
    #define UNUSED_ARG(x) (void) x;
#endif

#ifndef INDEXING_DEPRECATED
    #define INDEXING_DEPRECATED(func) MANTID_INDEXING_DEPRECATED func
#endif

// Use extern keyword in client code to suppress class template instantiation
#include "MantidKernel/System.h"

#ifdef INDEXING_EXPORTS
#define EXTERN_MANTID_INDEXING
#else
#define EXTERN_MANTID_INDEXING EXTERN_IMPORT
#endif /* INDEXING_EXPORTS*/
 
#endif
