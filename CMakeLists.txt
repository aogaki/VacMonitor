# Copyright 2017 MongoDB Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Demonstrates how to use the CMake 'find_package' mechanism to locate build against the libmongocxx
# shared library.

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if(POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif()

project(vac-mon LANGUAGES C CXX)

# Enforce the C++ standard, and disable extensions.
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#add_definitions("-std=c++11 -march=native -mtune=native")

set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif()

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{ROOTSYS}/ect/cmake")
find_package(ROOT REQUIRED COMPONENTS RIO Net)
include(${ROOT_USE_FILE})

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hpp)
add_executable(vac-mon ./main.cpp  ${sources} ${headers})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${ROOT_INCLUDE_DIR}
                    /usr/include/libserial/
                  )
link_directories(${ROOT_LIBRARY_DIR})

target_link_libraries(vac-mon
  ${ROOT_LIBRARIES}
  pthread
  serial
  RHTTP
)

add_custom_target(run
    COMMAND vac-mon
    DEPENDS vac-mon
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)
