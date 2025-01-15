# Install script for directory: /home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "fmt-core" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libfmtd.so.11.1.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libfmtd.so.11"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/cmake-build-debug/3rd-party/fmt/libfmtd.so.11.1.2"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/cmake-build-debug/3rd-party/fmt/libfmtd.so.11"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libfmtd.so.11.1.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libfmtd.so.11"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "fmt-core" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/eden/LearningInfiniTensor/TinyInfiniTensor/cmake-build-debug/3rd-party/fmt/libfmtd.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "fmt-core" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/fmt" TYPE FILE FILES
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/args.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/base.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/chrono.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/color.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/compile.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/core.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/format.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/format-inl.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/os.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/ostream.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/printf.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/ranges.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/std.h"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/3rd-party/fmt/include/fmt/xchar.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "fmt-core" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/cmake-build-debug/3rd-party/fmt/fmt-config.cmake"
    "/home/eden/LearningInfiniTensor/TinyInfiniTensor/cmake-build-debug/3rd-party/fmt/fmt-config-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "fmt-core" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake"
         "/home/eden/LearningInfiniTensor/TinyInfiniTensor/cmake-build-debug/3rd-party/fmt/CMakeFiles/Export/b834597d9b1628ff12ae4314c3a2e4b8/fmt-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES "/home/eden/LearningInfiniTensor/TinyInfiniTensor/cmake-build-debug/3rd-party/fmt/CMakeFiles/Export/b834597d9b1628ff12ae4314c3a2e4b8/fmt-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES "/home/eden/LearningInfiniTensor/TinyInfiniTensor/cmake-build-debug/3rd-party/fmt/CMakeFiles/Export/b834597d9b1628ff12ae4314c3a2e4b8/fmt-targets-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "fmt-core" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/eden/LearningInfiniTensor/TinyInfiniTensor/cmake-build-debug/3rd-party/fmt/fmt.pc")
endif()

