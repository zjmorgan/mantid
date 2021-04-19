
#ifndef MANTID_CURVEFITTING_DLL_H
#define MANTID_CURVEFITTING_DLL_H

#ifdef MANTID_CURVEFITTING_STATIC_DEFINE
#  define MANTID_CURVEFITTING_DLL
#  define MANTID_CURVEFITTING_NO_EXPORT
#else
#  ifndef MANTID_CURVEFITTING_DLL
#    ifdef CurveFitting_EXPORTS
        /* We are building this library */
#      define MANTID_CURVEFITTING_DLL __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MANTID_CURVEFITTING_DLL __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MANTID_CURVEFITTING_NO_EXPORT
#    define MANTID_CURVEFITTING_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MANTID_CURVEFITTING_DEPRECATED
#  define MANTID_CURVEFITTING_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MANTID_CURVEFITTING_DEPRECATED_EXPORT
#  define MANTID_CURVEFITTING_DEPRECATED_EXPORT MANTID_CURVEFITTING_DLL MANTID_CURVEFITTING_DEPRECATED
#endif

#ifndef MANTID_CURVEFITTING_DEPRECATED_NO_EXPORT
#  define MANTID_CURVEFITTING_DEPRECATED_NO_EXPORT MANTID_CURVEFITTING_NO_EXPORT MANTID_CURVEFITTING_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MANTID_CURVEFITTING_NO_DEPRECATED
#    define MANTID_CURVEFITTING_NO_DEPRECATED
#  endif
#endif

#ifndef UNUSED_ARG
    #define UNUSED_ARG(x) (void) x;
#endif

#ifndef CURVEFITTING_DEPRECATED
    #define CURVEFITTING_DEPRECATED(func) MANTID_CURVEFITTING_DEPRECATED func
#endif


#endif
