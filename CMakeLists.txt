# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.22)
project(kwin-touch-screen-improvements VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)

# KWin's exported target links these but its CMake config doesn't find them
find_package(Qt6 REQUIRED COMPONENTS Core Gui DBus Quick Widgets)
find_package(KF6 REQUIRED COMPONENTS Config CoreAddons KCMUtils)
# This effect uses KWin's internal C++ API, which has no stable ABI.
# Rebuild it whenever KWin is updated.
find_package(KWin REQUIRED)

add_subdirectory(src)
