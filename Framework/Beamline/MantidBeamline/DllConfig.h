
#ifndef MANTID_BEAMLINE_DLL_H
#define MANTID_BEAMLINE_DLL_H

#ifdef MANTID_BEAMLINE_STATIC_DEFINE
#  define MANTID_BEAMLINE_DLL
#  define MANTID_BEAMLINE_NO_EXPORT
#else
#  ifndef MANTID_BEAMLINE_DLL
#    ifdef Beamline_EXPORTS
        /* We are building this library */
#      define MANTID_BEAMLINE_DLL __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define MANTID_BEAMLINE_DLL __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef MANTID_BEAMLINE_NO_EXPORT
#    define MANTID_BEAMLINE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef MANTID_BEAMLINE_DEPRECATED
#  define MANTID_BEAMLINE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MANTID_BEAMLINE_DEPRECATED_EXPORT
#  define MANTID_BEAMLINE_DEPRECATED_EXPORT MANTID_BEAMLINE_DLL MANTID_BEAMLINE_DEPRECATED
#endif

#ifndef MANTID_BEAMLINE_DEPRECATED_NO_EXPORT
#  define MANTID_BEAMLINE_DEPRECATED_NO_EXPORT MANTID_BEAMLINE_NO_EXPORT MANTID_BEAMLINE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MANTID_BEAMLINE_NO_DEPRECATED
#    define MANTID_BEAMLINE_NO_DEPRECATED
#  endif
#endif

#ifndef UNUSED_ARG
    #define UNUSED_ARG(x) (void) x;
#endif

#ifndef BEAMLINE_DEPRECATED
    #define BEAMLINE_DEPRECATED(func) MANTID_BEAMLINE_DEPRECATED func
#endif


#endif
