# Install script for directory: /home/llama/Documents/Light_Files/Code/KDE/Plasma Keyboard/kwin-touch-screen-improvements/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kwin-touch-screen-improvements" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/plugins/touchscreenimprovements.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/plugins/touchscreenimprovements.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/plugins/touchscreenimprovements.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/plugins" TYPE MODULE FILES "/home/llama/Documents/Light_Files/Code/KDE/Plasma Keyboard/kwin-touch-screen-improvements/build/bin/kwin/effects/plugins/touchscreenimprovements.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/plugins/touchscreenimprovements.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/plugins/touchscreenimprovements.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/plugins/touchscreenimprovements.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kwin-touch-screen-improvements" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/llama/Documents/Light_Files/Code/KDE/Plasma Keyboard/kwin-touch-screen-improvements/build/src/CMakeFiles/touchscreenimprovements.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kwin-touch-screen-improvements" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/configs/kwin_touchscreenimprovements_config.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/configs/kwin_touchscreenimprovements_config.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/configs/kwin_touchscreenimprovements_config.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/configs" TYPE MODULE FILES "/home/llama/Documents/Light_Files/Code/KDE/Plasma Keyboard/kwin-touch-screen-improvements/build/bin/kwin/effects/configs/kwin_touchscreenimprovements_config.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/configs/kwin_touchscreenimprovements_config.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/configs/kwin_touchscreenimprovements_config.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qt6/plugins/kwin/effects/configs/kwin_touchscreenimprovements_config.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "kwin-touch-screen-improvements" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/llama/Documents/Light_Files/Code/KDE/Plasma Keyboard/kwin-touch-screen-improvements/build/src/CMakeFiles/kwin_touchscreenimprovements_config.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/llama/Documents/Light_Files/Code/KDE/Plasma Keyboard/kwin-touch-screen-improvements/build/src/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
