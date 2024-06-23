# %BANNER_BEGIN%
# ---------------------------------------------------------------------
# %COPYRIGHT_BEGIN%
#
# Copyright (c) 2023 Magic Leap, Inc. All Rights Reserved.
# Use of this file is governed by the Creator Agreement, located
# here: https://id.magicleap.com/creator-terms
#
# %COPYRIGHT_END%
# ---------------------------------------------------------------------
# %BANNER_END%

cmake_minimum_required(VERSION 3.22.1)

project(system_notifications)

file(TO_CMAKE_PATH "$ENV{MLSDK}" MLSDK)
list(APPEND CMAKE_MODULE_PATH "${MLSDK}/cmake" "${ANDROID_NDK}/../../mlsdk/cmake")

find_package(MagicLeap REQUIRED)
find_package(MagicLeapAppFramework REQUIRED)

add_library(system_notifications SHARED main.cpp)

include(DeprecatedApiUsage)
use_deprecated_api(system_notifications)

target_link_libraries(system_notifications
    ML::app_framework
    ML::audio
    ML::system_notification_manager
    ML::power_manager
    ML::capi
)

if (COMMAND copy_artifacts)
    copy_artifacts(system_notifications)
endif()