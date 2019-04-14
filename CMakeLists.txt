cmake_minimum_required (VERSION 2.8.12.2)

# Change project name to desired name
set (PROJECT_NAME "camera_module")

project (${PROJECT_NAME})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

# include directory for headers
include_directories(include)

file(GLOB SOURCES "src/*.cpp")

# Create camera module
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories (${PROJECT_NAME} PUBLIC include src)
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION 0)

# Change name of executable to desired name
#add_executable(${PROJECT_NAME} ${SOURCES})

find_library(UTILITY NAMES Utility HINTS "/usr/local/lib/OctopOS")
find_library(PUBLISHER NAMES Publisher HINTS "/usr/local/lib/OctopOS")
find_library(SUBSCRIBER NAMES Subscriber HINTS "/usr/local/lib/OctopOS")

# Check if pthreads.h is being used
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(PUBLIC ${PROJECT_NAME} "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(${PROJECT_NAME} "${CMAKE_THREAD_LIBS_INIT}"
      "${UTILITY}"
      "${PUBLISHER}"
      "${SUBSCRIBER}")
endif()
