#ifndef FLIGHTCORE__VISIBILITY_CONTROL_H_
#define FLIGHTCORE__VISIBILITY_CONTROL_H_
#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define FLIGHTCORE_EXPORT __attribute__ ((dllexport))
    #define FLIGHTCORE_IMPORT __attribute__ ((dllimport))
  #else
    #define FLIGHTCORE_EXPORT __declspec(dllexport)
    #define FLIGHTCORE_IMPORT __declspec(dllimport)
  #endif
  #ifdef FLIGHTCORE_BUILDING_LIBRARY
    #define FLIGHTCORE_PUBLIC FLIGHTCORE_EXPORT
  #else
    #define FLIGHTCORE_PUBLIC FLIGHTCORE_IMPORT
  #endif
  #define FLIGHTCORE_PUBLIC_TYPE FLIGHTCORE_PUBLIC
  #define FLIGHTCORE_LOCAL
#else
  #define FLIGHTCORE_EXPORT __attribute__ ((visibility("default")))
  #define FLIGHTCORE_IMPORT
  #if __GNUC__ >= 4
    #define FLIGHTCORE_PUBLIC __attribute__ ((visibility("default")))
    #define FLIGHTCORE_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define FLIGHTCORE_PUBLIC
    #define FLIGHTCORE_LOCAL
  #endif
  #define FLIGHTCORE_PUBLIC_TYPE
#endif
#endif  // FLIGHTCORE__VISIBILITY_CONTROL_H_
// Generated 29-Jul-2026 15:51:13
 