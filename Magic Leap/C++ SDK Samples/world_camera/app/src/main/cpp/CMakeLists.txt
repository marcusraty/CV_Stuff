# %BANNER_BEGIN%
# ---------------------------------------------------------------------
# %COPYRIGHT_BEGIN%
# Copyright (c) 2022 Magic Leap, Inc. All Rights Reserved.
# Use of this file is governed by the Software License Agreement,
# located here: https://www.magicleap.com/software-license-agreement-ml2
# Terms and conditions applicable to third-party materials accompanying
# this distribution may also be found in the top-level NOTICE file
# appearing herein.
# %COPYRIGHT_END%
# ---------------------------------------------------------------------
# %BANNER_END%

cmake_minimum_required(VERSION 3.22.1)

project(world_camera)

file(TO_CMAKE_PATH "$ENV{MLSDK}" MLSDK)
list(APPEND CMAKE_MODULE_PATH "${MLSDK}/cmake" "${ANDROID_NDK}/../../mlsdk/cmake")

find_package(MagicLeap REQUIRED)
find_package(MagicLeapAppFramework REQUIRED)

add_library(world_camera SHARED main.cpp)

include(DeprecatedApiUsage)
use_deprecated_api(world_camera)

target_link_libraries(world_camera
    ML::app_framework
)

if (COMMAND copy_artifacts)
    copy_artifacts(world_camera)
endif()