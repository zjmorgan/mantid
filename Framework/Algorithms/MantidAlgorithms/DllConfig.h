
#ifndef MANTID_ALGORITHMS_DLL_H
#define MANTID_ALGORITHMS_DLL_H

#ifdef MANTID_ALGORITHMS_STATIC_DEFINE
#  define MANTID_ALGORITHMS_DLL
#  define MANTID_ALGORITHMS_NO_EXPORT
#else
#  ifndef MANTID_ALGORITHMS_DLL
#    ifdef Algorithms_EXPORTS
        /* We are building this library */
#      define MANTID_ALGORITHMS_DLL __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MANTID_ALGORITHMS_DLL __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MANTID_ALGORITHMS_NO_EXPORT
#    define MANTID_ALGORITHMS_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MANTID_ALGORITHMS_DEPRECATED
#  define MANTID_ALGORITHMS_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MANTID_ALGORITHMS_DEPRECATED_EXPORT
#  define MANTID_ALGORITHMS_DEPRECATED_EXPORT MANTID_ALGORITHMS_DLL MANTID_ALGORITHMS_DEPRECATED
#endif

#ifndef MANTID_ALGORITHMS_DEPRECATED_NO_EXPORT
#  define MANTID_ALGORITHMS_DEPRECATED_NO_EXPORT MANTID_ALGORITHMS_NO_EXPORT MANTID_ALGORITHMS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MANTID_ALGORITHMS_NO_DEPRECATED
#    define MANTID_ALGORITHMS_NO_DEPRECATED
#  endif
#endif

#ifndef UNUSED_ARG
    #define UNUSED_ARG(x) (void) x;
#endif

#ifndef ALGORITHMS_DEPRECATED
    #define ALGORITHMS_DEPRECATED(func) MANTID_ALGORITHMS_DEPRECATED func
#endif


#endif
