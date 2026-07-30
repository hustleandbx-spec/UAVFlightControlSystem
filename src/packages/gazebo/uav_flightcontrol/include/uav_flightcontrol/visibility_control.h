#ifndef UAV_FLIGHTCONTROL__VISIBILITY_CONTROL_H_
#define UAV_FLIGHTCONTROL__VISIBILITY_CONTROL_H_
#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define UAV_FLIGHTCONTROL_EXPORT __attribute__ ((dllexport))
    #define UAV_FLIGHTCONTROL_IMPORT __attribute__ ((dllimport))
  #else
    #define UAV_FLIGHTCONTROL_EXPORT __declspec(dllexport)
    #define UAV_FLIGHTCONTROL_IMPORT __declspec(dllimport)
  #endif
  #ifdef UAV_FLIGHTCONTROL_BUILDING_LIBRARY
    #define UAV_FLIGHTCONTROL_PUBLIC UAV_FLIGHTCONTROL_EXPORT
  #else
    #define UAV_FLIGHTCONTROL_PUBLIC UAV_FLIGHTCONTROL_IMPORT
  #endif
  #define UAV_FLIGHTCONTROL_PUBLIC_TYPE UAV_FLIGHTCONTROL_PUBLIC
  #define UAV_FLIGHTCONTROL_LOCAL
#else
  #define UAV_FLIGHTCONTROL_EXPORT __attribute__ ((visibility("default")))
  #define UAV_FLIGHTCONTROL_IMPORT
  #if __GNUC__ >= 4
    #define UAV_FLIGHTCONTROL_PUBLIC __attribute__ ((visibility("default")))
    #define UAV_FLIGHTCONTROL_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define UAV_FLIGHTCONTROL_PUBLIC
    #define UAV_FLIGHTCONTROL_LOCAL
  #endif
  #define UAV_FLIGHTCONTROL_PUBLIC_TYPE
#endif
#endif  // UAV_FLIGHTCONTROL__VISIBILITY_CONTROL_H_
// Generated 29-Jul-2026 15:51:01
 