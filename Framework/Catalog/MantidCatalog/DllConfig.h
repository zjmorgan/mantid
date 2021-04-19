
#ifndef MANTID_CATALOG_DLL_H
#define MANTID_CATALOG_DLL_H

#ifdef MANTID_CATALOG_STATIC_DEFINE
#  define MANTID_CATALOG_DLL
#  define MANTID_CATALOG_NO_EXPORT
#else
#  ifndef MANTID_CATALOG_DLL
#    ifdef Catalog_EXPORTS
        /* We are building this library */
#      define MANTID_CATALOG_DLL __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MANTID_CATALOG_DLL __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MANTID_CATALOG_NO_EXPORT
#    define MANTID_CATALOG_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MANTID_CATALOG_DEPRECATED
#  define MANTID_CATALOG_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MANTID_CATALOG_DEPRECATED_EXPORT
#  define MANTID_CATALOG_DEPRECATED_EXPORT MANTID_CATALOG_DLL MANTID_CATALOG_DEPRECATED
#endif

#ifndef MANTID_CATALOG_DEPRECATED_NO_EXPORT
#  define MANTID_CATALOG_DEPRECATED_NO_EXPORT MANTID_CATALOG_NO_EXPORT MANTID_CATALOG_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MANTID_CATALOG_NO_DEPRECATED
#    define MANTID_CATALOG_NO_DEPRECATED
#  endif
#endif

#ifndef UNUSED_ARG
    #define UNUSED_ARG(x) (void) x;
#endif

#ifndef CATALOG_DEPRECATED
    #define CATALOG_DEPRECATED(func) MANTID_CATALOG_DEPRECATED func
#endif

// Use extern keyword in client code to suppress class template instantiation
#include "MantidKernel/System.h"

#ifdef CATALOG_EXPORTS
#define EXTERN_MANTID_CATALOG
#else
#define EXTERN_MANTID_CATALOG EXTERN_IMPORT
#endif /* CATALOG_EXPORTS*/
 
#endif
