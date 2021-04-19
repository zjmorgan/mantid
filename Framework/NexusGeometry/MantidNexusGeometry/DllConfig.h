
#ifndef MANTID_NEXUSGEOMETRY_DLL_H
#define MANTID_NEXUSGEOMETRY_DLL_H

#ifdef MANTID_NEXUSGEOMETRY_STATIC_DEFINE
#  define MANTID_NEXUSGEOMETRY_DLL
#  define MANTID_NEXUSGEOMETRY_NO_EXPORT
#else
#  ifndef MANTID_NEXUSGEOMETRY_DLL
#    ifdef NexusGeometry_EXPORTS
        /* We are building this library */
#      define MANTID_NEXUSGEOMETRY_DLL __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MANTID_NEXUSGEOMETRY_DLL __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MANTID_NEXUSGEOMETRY_NO_EXPORT
#    define MANTID_NEXUSGEOMETRY_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MANTID_NEXUSGEOMETRY_DEPRECATED
#  define MANTID_NEXUSGEOMETRY_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MANTID_NEXUSGEOMETRY_DEPRECATED_EXPORT
#  define MANTID_NEXUSGEOMETRY_DEPRECATED_EXPORT MANTID_NEXUSGEOMETRY_DLL MANTID_NEXUSGEOMETRY_DEPRECATED
#endif

#ifndef MANTID_NEXUSGEOMETRY_DEPRECATED_NO_EXPORT
#  define MANTID_NEXUSGEOMETRY_DEPRECATED_NO_EXPORT MANTID_NEXUSGEOMETRY_NO_EXPORT MANTID_NEXUSGEOMETRY_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MANTID_NEXUSGEOMETRY_NO_DEPRECATED
#    define MANTID_NEXUSGEOMETRY_NO_DEPRECATED
#  endif
#endif

#ifndef UNUSED_ARG
    #define UNUSED_ARG(x) (void) x;
#endif

#ifndef NEXUSGEOMETRY_DEPRECATED
    #define NEXUSGEOMETRY_DEPRECATED(func) MANTID_NEXUSGEOMETRY_DEPRECATED func
#endif

// Use extern keyword in client code to suppress class template instantiation
#include "MantidKernel/System.h"

#ifdef NEXUSGEOMETRY_EXPORTS
#define EXTERN_MANTID_NEXUSGEOMETRY
#else
#define EXTERN_MANTID_NEXUSGEOMETRY EXTERN_IMPORT
#endif /* NEXUSGEOMETRY_EXPORTS*/
 
#endif
