
#ifndef MANTID_MUON_DLL_H
#define MANTID_MUON_DLL_H

#ifdef MANTID_MUON_STATIC_DEFINE
#  define MANTID_MUON_DLL
#  define MANTID_MUON_NO_EXPORT
#else
#  ifndef MANTID_MUON_DLL
#    ifdef Muon_EXPORTS
        /* We are building this library */
#      define MANTID_MUON_DLL __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MANTID_MUON_DLL __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MANTID_MUON_NO_EXPORT
#    define MANTID_MUON_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MANTID_MUON_DEPRECATED
#  define MANTID_MUON_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MANTID_MUON_DEPRECATED_EXPORT
#  define MANTID_MUON_DEPRECATED_EXPORT MANTID_MUON_DLL MANTID_MUON_DEPRECATED
#endif

#ifndef MANTID_MUON_DEPRECATED_NO_EXPORT
#  define MANTID_MUON_DEPRECATED_NO_EXPORT MANTID_MUON_NO_EXPORT MANTID_MUON_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MANTID_MUON_NO_DEPRECATED
#    define MANTID_MUON_NO_DEPRECATED
#  endif
#endif

#ifndef UNUSED_ARG
    #define UNUSED_ARG(x) (void) x;
#endif

#ifndef MUON_DEPRECATED
    #define MUON_DEPRECATED(func) MANTID_MUON_DEPRECATED func
#endif


#endif
