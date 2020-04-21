cmake_minimum_required(VERSION 3.7)
project(rvnsyncpoint)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

option(BUILD_SHARED_LIBS "Set to ON to build shared libraries; OFF for static libraries." OFF)
option(WARNING_AS_ERROR "Set to ON to build with -Werror" ON)

option(BUILD_TEST_COVERAGE "Set to ON to build while generating coverage information. Will put source on the build directory." OFF)

add_library(rvnsyncpoint
  src/streamable_file.cpp
  src/streamable_outfile.cpp
  src/sync_event.cpp
  src/sync_point.cpp
  src/sync_file.cpp
  src/io_file.cpp
  src/hardware_file.cpp
  src/hardware_access.cpp
)

target_compile_options(rvnsyncpoint PRIVATE -W -Wall -Wextra -Wmissing-include-dirs -Wunknown-pragmas -Wpointer-arith -Wmissing-field-initializers -Wno-multichar -Wreturn-type)

if(WARNING_AS_ERROR)
  target_compile_options(rvnsyncpoint PRIVATE -Werror)
endif()

if(BUILD_TEST_COVERAGE)
  target_compile_options(rvnsyncpoint PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
  target_link_libraries(rvnsyncpoint PRIVATE gcov)
endif()

target_include_directories(rvnsyncpoint
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set(PUBLIC_HEADERS
  include/device.h
  include/hardware_access.h
  include/hardware_file.h
  include/io_file.h
  include/streamable_file.h
  include/streamable_outfile.h
  include/sync_event.h
  include/sync_file.h
  include/sync_point.h
)

set_target_properties(rvnsyncpoint PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  POSITION_INDEPENDENT_CODE ON
)

include(GNUInstallDirs)
install(TARGETS rvnsyncpoint
  EXPORT rvnsyncpoint-export
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

install(EXPORT rvnsyncpoint-export
  FILE
    rvnsyncpoint-config.cmake
  DESTINATION
    ${CMAKE_INSTALL_DATADIR}/cmake/rvnsyncpoint
)

add_subdirectory(bin)
