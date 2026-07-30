cmake_minimum_required(VERSION 3.5)
project(flightcore)
set(CMAKE_VERBOSE_MAKEFILE ON)
# Set CMAKE_BUILD_TYPE
set(CMAKE_BUILD_TYPE Release)
# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()
# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(ALIAS_ROS2_TF2)
  add_definitions(-Dtf2=ros2_tf2)
  add_definitions(-Dtf2_ros=ros2_tf2_ros)
  add_definitions(-Dmessage_filters=ros2_message_filters)
endif()
# Check the ROS 2 distribution and define a macro
if("$ENV{ROS_DISTRO}" STREQUAL "humble")
    message(STATUS "Building for ROS 2 Humble")
    add_definitions(-DROS2_DISTRO_HUMBLE)
elseif("$ENV{ROS_DISTRO}" STREQUAL "jazzy")
    message(STATUS "Building for ROS 2 Jazzy")
    add_definitions(-DROS2_DISTRO_JAZZY)
else()
    message(WARNING "ROS 2 distribution $ENV{ROS_DISTRO} is not recommended for this MATLAB version. Use 'ROS 2 Jazzy' for this MATLAB version.")
    message(WARNING "Building for ROS 2 Jazzy")
    add_definitions(-DROS2_DISTRO_JAZZY)
endif()
# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)
find_package( ekf REQUIRED)
find_package( rclcpp REQUIRED)
find_package( uav_flightcontrol REQUIRED)
# Add package include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/flightcore")
set (CMAKE_SKIP_BUILD_RPATH false)
set (CMAKE_BUILD_WITH_INSTALL_RPATH true)
set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# Library
add_library(
    FlightCore 
    STATIC
    src/slros2_multi_threaded_executor.cpp
    src/FlightCore.cpp
    src/slros2_initialize.cpp
    src/const_params.cpp
    src/norm_94qjDDKI.cpp
    src/rtGetNaN.cpp
    src/rt_nonfinite.cpp
    src/rt_zcfcn.cpp
    src/const_params.cpp
    src/norm_94qjDDKI.cpp
    src/rtGetNaN.cpp
    src/rt_nonfinite.cpp
    src/rt_zcfcn.cpp
)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
target_compile_options(FlightCore PUBLIC 
  $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:C>>:
  -Wall -Wextra -Wpedantic
  >)
endif()
target_compile_options(FlightCore PRIVATE 
  $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:C>>:
  -DMODEL=FlightCore
  -DNUMST=4
  -DNCSTATES=0
  -DHAVESTDIO
  -DMODEL_HAS_DYNAMICALLY_LOADED_SFCNS=0
  -DCLASSIC_INTERFACE=0
  -DALLOCATIONFCN=0
  -DTID01EQ=1
  -DTERMFCN=1
  -DONESTEPFCN=1
  -DMAT_FILE=0
  -DMULTI_INSTANCE_CODE=0
  -DINTEGER_CODE=0
  -DMT=0
  -DROS2_PROJECT
  -DSTACK_SIZE=64
  -D__MW_TARGET_USE_HARDWARE_RESOURCES_H__
  -DRT
  -fpermissive  >
)
target_include_directories(FlightCore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/flightcore>
  $<INSTALL_INTERFACE:include>)
ament_target_dependencies(
  FlightCore
  "ekf"
  "rclcpp"
  "uav_flightcontrol"
)
# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(FlightCore PRIVATE "FLIGHTCORE_BUILDING_LIBRARY")
install(
  DIRECTORY include/
  DESTINATION include
)
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(
  ekf
  rclcpp
  uav_flightcontrol
)
install(
  TARGETS FlightCore
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
ament_export_include_directories(
  include
)
ament_export_libraries(
  FlightCore
)
foreach(file_i ${CUDA_BINS})
add_custom_command(
                   TARGET 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND}
                   ARGS -E copy ${PROJECT_SOURCE_DIR}/src/${file_i} ${CMAKE_INSTALL_PREFIX}/_ert_rtw/${file_i}
				   )
add_custom_command(
                   TARGET 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND}
                   ARGS -E copy ${PROJECT_SOURCE_DIR}/src/${file_i} ${CMAKE_INSTALL_PREFIX}/codegen/exe//${file_i}
				   )				   
endforeach(file_i)
ament_package()
# Generated on Wed Jul 29 15:51:13 2026
