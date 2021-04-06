
#ifndef MANTID_CRYSTAL_DLL_H
#define MANTID_CRYSTAL_DLL_H

#ifdef MANTID_CRYSTAL_STATIC_DEFINE
#  define MANTID_CRYSTAL_DLL
#  define MANTID_CRYSTAL_NO_EXPORT
#else
#  ifndef MANTID_CRYSTAL_DLL
#    ifdef Crystal_EXPORTS
        /* We are building this library */
#      define MANTID_CRYSTAL_DLL __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MANTID_CRYSTAL_DLL __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MANTID_CRYSTAL_NO_EXPORT
#    define MANTID_CRYSTAL_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MANTID_CRYSTAL_DEPRECATED
#  define MANTID_CRYSTAL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MANTID_CRYSTAL_DEPRECATED_EXPORT
#  define MANTID_CRYSTAL_DEPRECATED_EXPORT MANTID_CRYSTAL_DLL MANTID_CRYSTAL_DEPRECATED
#endif

#ifndef MANTID_CRYSTAL_DEPRECATED_NO_EXPORT
#  define MANTID_CRYSTAL_DEPRECATED_NO_EXPORT MANTID_CRYSTAL_NO_EXPORT MANTID_CRYSTAL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MANTID_CRYSTAL_NO_DEPRECATED
#    define MANTID_CRYSTAL_NO_DEPRECATED
#  endif
#endif

#ifndef UNUSED_ARG
    #define UNUSED_ARG(x) (void) x;
#endif

#ifndef CRYSTAL_DEPRECATED
    #define CRYSTAL_DEPRECATED(func) MANTID_CRYSTAL_DEPRECATED func
#endif


#endif
