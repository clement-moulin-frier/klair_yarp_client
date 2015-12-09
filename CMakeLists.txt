# Copyright: (C) 2010 RobotCub Consortium
# Author: Paul Fitzpatrick
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
# YARP needs CMake 2.8.9 or greater
cmake_minimum_required(VERSION 2.8.9)

set(PROJECTNAME klair_yarp_client)
project(${PROJECTNAME})

# find YARP
find_package(YARP REQUIRED)

file(GLOB folder_source ./*.cpp ../Klair/rpc/*.c)
file(GLOB folder_header ../Klair/rpc/*.h )

source_group("Header Files" FILES ${folder_header})
source_group("Source Files" FILES ${folder_source})


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Klair/rpc ${YARP_INCLUDE_DIRS})

add_executable(${PROJECTNAME} ${folder_header} ${folder_source})

target_link_libraries(${PROJECTNAME} rpcrt4 ${YARP_LIBRARIES})

# SET_TARGET_PROPERTIES(${PROJECTNAME} PROPERTIES COMPILER_FLAGS "/EHsc")
