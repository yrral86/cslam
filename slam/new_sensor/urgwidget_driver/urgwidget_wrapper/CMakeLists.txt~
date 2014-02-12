cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#set(ROS_BUILD_TYPE RelWithDebInfo)

rosbuild_init()

#set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#uncomment if you have defined messages
#rosbuild_genmsg()
#uncomment if you have defined services
#rosbuild_gensrv()

rosbuild_add_library(urg_ring_buffer src/urg_ring_buffer.c)

rosbuild_add_library(urg_serial src/urg_serial.c)
target_link_libraries(urg_serial urg_ring_buffer)

rosbuild_add_library(urg_tcpclient src/urg_tcpclient.c)
target_link_libraries(urg_tcpclient urg_ring_buffer)

rosbuild_add_library(urg_connection src/urg_connection.c)
target_link_libraries(urg_connection urg_serial urg_tcpclient)

rosbuild_add_library(urg_utils src/urg_utils.c)

rosbuild_add_library(urgwidget src/urg_sensor.c)
target_link_libraries(urgwidget urg_connection)

rosbuild_add_executable(urg_node ros_src/urg_node.cpp)
target_link_libraries(urg_node urgwidget urg_utils)


#common commands for building c++ executables and libraries
#rosbuild_add_library(${PROJECT_NAME} src/example.cpp)
#target_link_libraries(${PROJECT_NAME} another_library)
#rosbuild_add_boost_directories()
#rosbuild_link_boost(${PROJECT_NAME} thread)
#rosbuild_add_executable(example examples/example.cpp)
#target_link_libraries(example ${PROJECT_NAME})
