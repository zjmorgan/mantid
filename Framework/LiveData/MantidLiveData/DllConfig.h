
#ifndef MANTID_LIVEDATA_DLL_H
#define MANTID_LIVEDATA_DLL_H

#ifdef MANTID_LIVEDATA_STATIC_DEFINE
#  define MANTID_LIVEDATA_DLL
#  define MANTID_LIVEDATA_NO_EXPORT
#else
#  ifndef MANTID_LIVEDATA_DLL
#    ifdef LiveData_EXPORTS
        /* We are building this library */
#      define MANTID_LIVEDATA_DLL __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MANTID_LIVEDATA_DLL __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MANTID_LIVEDATA_NO_EXPORT
#    define MANTID_LIVEDATA_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MANTID_LIVEDATA_DEPRECATED
#  define MANTID_LIVEDATA_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MANTID_LIVEDATA_DEPRECATED_EXPORT
#  define MANTID_LIVEDATA_DEPRECATED_EXPORT MANTID_LIVEDATA_DLL MANTID_LIVEDATA_DEPRECATED
#endif

#ifndef MANTID_LIVEDATA_DEPRECATED_NO_EXPORT
#  define MANTID_LIVEDATA_DEPRECATED_NO_EXPORT MANTID_LIVEDATA_NO_EXPORT MANTID_LIVEDATA_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MANTID_LIVEDATA_NO_DEPRECATED
#    define MANTID_LIVEDATA_NO_DEPRECATED
#  endif
#endif

#ifndef UNUSED_ARG
    #define UNUSED_ARG(x) (void) x;
#endif

#ifndef LIVEDATA_DEPRECATED
    #define LIVEDATA_DEPRECATED(func) MANTID_LIVEDATA_DEPRECATED func
#endif


#endif
