#ifndef ROS2_TIME_STEPPING_PROBE__VISIBILITY_CONTROL_H_
#define ROS2_TIME_STEPPING_PROBE__VISIBILITY_CONTROL_H_
#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define ROS2_TIME_STEPPING_PROBE_EXPORT __attribute__ ((dllexport))
    #define ROS2_TIME_STEPPING_PROBE_IMPORT __attribute__ ((dllimport))
  #else
    #define ROS2_TIME_STEPPING_PROBE_EXPORT __declspec(dllexport)
    #define ROS2_TIME_STEPPING_PROBE_IMPORT __declspec(dllimport)
  #endif
  #ifdef ROS2_TIME_STEPPING_PROBE_BUILDING_LIBRARY
    #define ROS2_TIME_STEPPING_PROBE_PUBLIC ROS2_TIME_STEPPING_PROBE_EXPORT
  #else
    #define ROS2_TIME_STEPPING_PROBE_PUBLIC ROS2_TIME_STEPPING_PROBE_IMPORT
  #endif
  #define ROS2_TIME_STEPPING_PROBE_PUBLIC_TYPE ROS2_TIME_STEPPING_PROBE_PUBLIC
  #define ROS2_TIME_STEPPING_PROBE_LOCAL
#else
  #define ROS2_TIME_STEPPING_PROBE_EXPORT __attribute__ ((visibility("default")))
  #define ROS2_TIME_STEPPING_PROBE_IMPORT
  #if __GNUC__ >= 4
    #define ROS2_TIME_STEPPING_PROBE_PUBLIC __attribute__ ((visibility("default")))
    #define ROS2_TIME_STEPPING_PROBE_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define ROS2_TIME_STEPPING_PROBE_PUBLIC
    #define ROS2_TIME_STEPPING_PROBE_LOCAL
  #endif
  #define ROS2_TIME_STEPPING_PROBE_PUBLIC_TYPE
#endif
#endif  // ROS2_TIME_STEPPING_PROBE__VISIBILITY_CONTROL_H_
// Generated 28-Jul-2026 16:51:12
 