#ifndef FLIGHTCORE_GAZEBO_LOOP__VISIBILITY_CONTROL_H_
#define FLIGHTCORE_GAZEBO_LOOP__VISIBILITY_CONTROL_H_
#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define FLIGHTCORE_GAZEBO_LOOP_EXPORT __attribute__ ((dllexport))
    #define FLIGHTCORE_GAZEBO_LOOP_IMPORT __attribute__ ((dllimport))
  #else
    #define FLIGHTCORE_GAZEBO_LOOP_EXPORT __declspec(dllexport)
    #define FLIGHTCORE_GAZEBO_LOOP_IMPORT __declspec(dllimport)
  #endif
  #ifdef FLIGHTCORE_GAZEBO_LOOP_BUILDING_LIBRARY
    #define FLIGHTCORE_GAZEBO_LOOP_PUBLIC FLIGHTCORE_GAZEBO_LOOP_EXPORT
  #else
    #define FLIGHTCORE_GAZEBO_LOOP_PUBLIC FLIGHTCORE_GAZEBO_LOOP_IMPORT
  #endif
  #define FLIGHTCORE_GAZEBO_LOOP_PUBLIC_TYPE FLIGHTCORE_GAZEBO_LOOP_PUBLIC
  #define FLIGHTCORE_GAZEBO_LOOP_LOCAL
#else
  #define FLIGHTCORE_GAZEBO_LOOP_EXPORT __attribute__ ((visibility("default")))
  #define FLIGHTCORE_GAZEBO_LOOP_IMPORT
  #if __GNUC__ >= 4
    #define FLIGHTCORE_GAZEBO_LOOP_PUBLIC __attribute__ ((visibility("default")))
    #define FLIGHTCORE_GAZEBO_LOOP_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define FLIGHTCORE_GAZEBO_LOOP_PUBLIC
    #define FLIGHTCORE_GAZEBO_LOOP_LOCAL
  #endif
  #define FLIGHTCORE_GAZEBO_LOOP_PUBLIC_TYPE
#endif
#endif  // FLIGHTCORE_GAZEBO_LOOP__VISIBILITY_CONTROL_H_
// Generated 29-Jul-2026 15:51:40
 