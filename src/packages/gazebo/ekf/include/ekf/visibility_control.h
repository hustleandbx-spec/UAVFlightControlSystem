#ifndef EKF__VISIBILITY_CONTROL_H_
#define EKF__VISIBILITY_CONTROL_H_
#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define EKF_EXPORT __attribute__ ((dllexport))
    #define EKF_IMPORT __attribute__ ((dllimport))
  #else
    #define EKF_EXPORT __declspec(dllexport)
    #define EKF_IMPORT __declspec(dllimport)
  #endif
  #ifdef EKF_BUILDING_LIBRARY
    #define EKF_PUBLIC EKF_EXPORT
  #else
    #define EKF_PUBLIC EKF_IMPORT
  #endif
  #define EKF_PUBLIC_TYPE EKF_PUBLIC
  #define EKF_LOCAL
#else
  #define EKF_EXPORT __attribute__ ((visibility("default")))
  #define EKF_IMPORT
  #if __GNUC__ >= 4
    #define EKF_PUBLIC __attribute__ ((visibility("default")))
    #define EKF_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define EKF_PUBLIC
    #define EKF_LOCAL
  #endif
  #define EKF_PUBLIC_TYPE
#endif
#endif  // EKF__VISIBILITY_CONTROL_H_
// Generated 29-Jul-2026 15:50:49
 