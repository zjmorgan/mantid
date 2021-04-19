
#ifndef MANTID_API_DLL_H
#define MANTID_API_DLL_H

#ifdef MANTID_API_STATIC_DEFINE
#  define MANTID_API_DLL
#  define MANTID_API_NO_EXPORT
#else
#  ifndef MANTID_API_DLL
#    ifdef API_EXPORTS
        /* We are building this library */
#      define MANTID_API_DLL __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MANTID_API_DLL __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MANTID_API_NO_EXPORT
#    define MANTID_API_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MANTID_API_DEPRECATED
#  define MANTID_API_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MANTID_API_DEPRECATED_EXPORT
#  define MANTID_API_DEPRECATED_EXPORT MANTID_API_DLL MANTID_API_DEPRECATED
#endif

#ifndef MANTID_API_DEPRECATED_NO_EXPORT
#  define MANTID_API_DEPRECATED_NO_EXPORT MANTID_API_NO_EXPORT MANTID_API_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MANTID_API_NO_DEPRECATED
#    define MANTID_API_NO_DEPRECATED
#  endif
#endif

#ifndef UNUSED_ARG
    #define UNUSED_ARG(x) (void) x;
#endif

#ifndef API_DEPRECATED
    #define API_DEPRECATED(func) MANTID_API_DEPRECATED func
#endif

// Use extern keyword in client code to suppress class template instantiation
#include "MantidKernel/System.h"

#ifdef API_EXPORTS
#define EXTERN_MANTID_API
#else
#define EXTERN_MANTID_API EXTERN_IMPORT
#endif /* API_EXPORTS*/
 
#endif
