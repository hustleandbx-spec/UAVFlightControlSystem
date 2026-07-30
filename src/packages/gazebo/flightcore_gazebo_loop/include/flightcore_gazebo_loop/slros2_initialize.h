// Copyright 2022-2024 The MathWorks, Inc.
// Generated 29-Jul-2026 15:51:33
#ifndef _SLROS2_INITIALIZE_H_
#define _SLROS2_INITIALIZE_H_
#include "FlightCore_Gazebo_loop_types.h"
// Generic pub-sub header
#include "slros2_generic_pubsub.h"
#ifndef SET_QOS_VALUES
#define SET_QOS_VALUES(qosStruct, _history, _depth, _durability, _reliability, _deadline \
, _lifespan, _liveliness, _lease_duration, _avoid_ros_namespace_conventions)             \
    {                                                                                    \
        qosStruct.history = _history;                                                    \
        qosStruct.depth = _depth;                                                        \
        qosStruct.durability = _durability;                                              \
        qosStruct.reliability = _reliability;                                            \
        qosStruct.deadline.sec = _deadline.sec;                                          \
        qosStruct.deadline.nsec = _deadline.nsec;                                        \
        qosStruct.lifespan.sec = _lifespan.sec;                                          \
        qosStruct.lifespan.nsec = _lifespan.nsec;                                        \
        qosStruct.liveliness = _liveliness;                                              \
        qosStruct.liveliness_lease_duration.sec = _lease_duration.sec;                   \
        qosStruct.liveliness_lease_duration.nsec = _lease_duration.nsec;                 \
        qosStruct.avoid_ros_namespace_conventions = _avoid_ros_namespace_conventions;    \
    }
#endif
inline rclcpp::QoS getQOSSettingsFromRMW(const rmw_qos_profile_t& qosProfile) {
    rclcpp::QoS qos(rclcpp::QoSInitialization::from_rmw(qosProfile));
    if (RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL == qosProfile.durability) {
        qos.transient_local();
    } else {
        qos.durability_volatile();
    }
    if (RMW_QOS_POLICY_RELIABILITY_RELIABLE == qosProfile.reliability) {
        qos.reliable();
    } else {
        qos.best_effort();
    }
    return qos;
}
// FlightCore_Gazebo_loop/GazeboEscCmd/Gazebo_ESC_Publish
extern SimulinkPublisher<flightcore_gazebo_msgs::msg::ActuatorCommand,SL_Bus_flightcore_gazebo_msgs_ActuatorCommand> Pub_FlightCore_Gazebo_loop_42;
// FlightCore_Gazebo_loop/GazeboGpsSubscribe
extern SimulinkSubscriber<flightcore_msgs::msg::Gps,SL_Bus_flightcore_msgs_Gps> Sub_FlightCore_Gazebo_loop_189;
// FlightCore_Gazebo_loop/GazeboImuSubscribe
extern SimulinkSubscriber<flightcore_msgs::msg::Imu,SL_Bus_flightcore_msgs_Imu> Sub_FlightCore_Gazebo_loop_187;
#endif
