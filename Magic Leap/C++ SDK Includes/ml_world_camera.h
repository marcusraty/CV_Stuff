// %BANNER_BEGIN%
// ---------------------------------------------------------------------
// %COPYRIGHT_BEGIN%
// Copyright (c) 2022 Magic Leap, Inc. All Rights Reserved.
// Use of this file is governed by the Software License Agreement,
// located here: https://www.magicleap.com/software-license-agreement-ml2
// Terms and conditions applicable to third-party materials accompanying
// this distribution may also be found in the top-level NOTICE file
// appearing herein.
// %COPYRIGHT_END%
// ---------------------------------------------------------------------
// %BANNER_END%

#pragma once

#include "ml_api.h"
#include "ml_types.h"

#include <string.h>

ML_EXTERN_C_BEGIN

/*!
  \addtogroup PixelSensors Pixel Sensors
  \sharedobject perception.magicleap
  \brief APIs to access pixel sensors data.
*/

/*!
  \addtogroup WCam World Camera
  \ingroup PixelSensors
  \brief APIs to access the world camera data.

  Magic Leap 2 has three world cameras which it uses for environment tracking.
  The three cameras area located on the left, center, and right side of the
  headset. This API will provide a way to query for the frames from these world
  cameras, at this point the app will not be able to configure the world camera
  parameters.

  \experimental This is an experimental API which may be modified or removed without
                any prior notice.

  \{
*/

enum {
  /*! Default radial distortion vector size. */
  MLWorldCameraIntrinsics_MaxRadialDistortionCoefficients = 4,
  /*! Default tangential distortion vector size. */
  MLWorldCameraIntrinsics_MaxTangentialDistortionCoefficients = 2
};

/*!
  \brief Camera Identifier.

  Enumeration of all the available world camera sensors.
*/
typedef enum MLWorldCameraIdentifier{
  /*! None. */
  MLWorldCameraIdentifier_None = 0,
  /*! Left World camera. */
  MLWorldCameraIdentifier_Left = 1 << 0,
  /*! Right World camera. */
  MLWorldCameraIdentifier_Right = 1 << 1,
  /*! Center World camera. */
  MLWorldCameraIdentifier_Center = 1 << 2,
  /*! All World cameras. */
  MLWorldCameraIdentifier_All = MLWorldCameraIdentifier_Left |
                                MLWorldCameraIdentifier_Right |
                                MLWorldCameraIdentifier_Center,
  /*! Ensure enum is represented as 32 bits. */
  MLWorldCameraIdentifier_Ensure32Bits = 0x7FFFFFFF
} MLWorldCameraIdentifier;

/*!
  \brief Enumeration of world camera modes.
*/
typedef enum MLWorldCameraMode {
  /*! None. */
  MLWorldCameraMode_Unknown = 0,
  /*!
    \brief Low exposure mode.

    This mode is currently only available when the controller is being tracked.
  */
  MLWorldCameraMode_LowExposure = 1 << 0,
  /*!
    \brief Normal exposure mode.
  */
  MLWorldCameraMode_NormalExposure = 1 << 1,
  /*! Ensure enum is represented as 32 bits. */
  MLWorldCameraMode_Ensure32Bits = 0x7FFFFFFF
} MLWorldCameraMode;

/*!
  \brief Enumeration of camera mode used when capturing a frame.
*/
typedef enum MLWorldCameraFrameType {
  /*! None. */
  MLWorldCameraFrameType_Unknown = 0,
  /*!
    \brief Frame captured using #MLWorldCameraMode_LowExposure mode.
  */
  MLWorldCameraFrameType_LowExposure = 1,
  /*!
    \brief Frame captured using #MLWorldCameraMode_NormalExposure mode.
  */
  MLWorldCameraFrameType_NormalExposure = 2,
  /*! Ensure enum is represented as 32 bits. */
  MLWorldCameraFrameType_Ensure32Bits = 0x7FFFFFFF
} MLWorldCameraFrameType;

/*!
  \brief A structure to encapsulate the camera settings.

  This structure must be initialized by calling #MLWorldCameraSettingsInit
  before use.

  \apilevel 23
*/
typedef struct MLWorldCameraSettings {
  /*! Version of this structure. */
  uint32_t version;
  /*!
    \brief World camera mode.

     See #MLWorldCameraMode for more details. If you want to request frames
     from different camera modes then "OR" the modes of interest to the app.
     The mode will apply for all the cameras.

     \note The system may not be able to service all the requested camera modes.
           This parameter is treated as a hint and data will be provided for
           the requested camera modes when available.
  */
  uint32_t mode;
  /*!
    \brief World cameras that need to be enabled.

     See #MLWorldCameraIdentifier for more details. If you want to request
     frames from different world camera then "OR" the cameras of interest to
     the app.

     \note The system may not be able to service all the requested cameras.
           This parameter is treated as a hint and data will be provided from
           the requested world cameras when available.
  */
  uint32_t cameras;
} MLWorldCameraSettings;

/*!
  \brief Initialize the camera settings structure.
  Shall be called before calling MLWorldCameraConnect().

  \apilevel 23

  \param[in,out] inout_handle MLWorldCameraSettings structure to initialize.

  \permissions None
*/
ML_STATIC_INLINE void MLWorldCameraSettingsInit(MLWorldCameraSettings *inout_handle) {
  if (inout_handle) {
    memset(inout_handle, 0, sizeof(MLWorldCameraSettings));
    inout_handle->version = 1;
    inout_handle->mode = MLWorldCameraMode_NormalExposure;
    inout_handle->cameras = MLWorldCameraIdentifier_All;
  }
}

/*!
  \brief World camera intrinsic parameters.

  \apilevel 23
*/
typedef struct MLWorldCameraIntrinsics {
  /*! Camera width. */
  uint32_t width;
  /*! Camera height. */
  uint32_t height;
  /*! Camera focal length. */
  MLVec2f focal_length;
  /*! Camera principal point. */
  MLVec2f principal_point;
  /*! Field of view in degrees. */
  float fov;
  /*!
    \brief Radial distortion vector.
    The radial distortion co-efficients are in the following order: [k1, k2, k3, k4].
   */
  double radial_distortion[MLWorldCameraIntrinsics_MaxRadialDistortionCoefficients];
  /*!
    \brief Tangential distortion vector.
    The tangential distortion co-efficients are in the following order: [p1, p2].
  */
  double tangential_distortion[MLWorldCameraIntrinsics_MaxTangentialDistortionCoefficients];
} MLWorldCameraIntrinsics;

/*!
  \brief A structure to encapsulate per plane info for each camera frame.

  \apilevel 23
*/
typedef struct MLWorldCameraFrameBuffer {
  /*! Width of the frame in pixels. */
  uint32_t width;
  /*! Height of the frame in pixels. */
  uint32_t height;
  /*! Stride of the frame in bytes. */
  uint32_t stride;
  /*! Number of bytes used to represent a single value. */
  uint32_t bytes_per_pixel;
  /*! Number of bytes in the frame. */
  uint32_t size;
  /*! Buffer data. */
  uint8_t *data;
} MLWorldCameraFrameBuffer;


/*!
  \brief A structure to encapsulate output data for each camera sensor.

  \apilevel 23
*/
typedef struct MLWorldCameraFrame {
  /*! Camera Identifier specifies which camera is associated with this frame. */
  MLWorldCameraIdentifier id;
  /*! A 64bit integer to index the frame number associated with this frame. */
  int64_t frame_number;
  /*! Frame timestamp specifies the time at which the frame was captured. */
  MLTime timestamp;
  /*! Camera intrinsic parameters. */
  MLWorldCameraIntrinsics intrinsics;
  /*! World camera pose in the world co-ordinate system. */
  MLTransform camera_pose;
  /*! Frame buffer data. */
  MLWorldCameraFrameBuffer frame_buffer;
  /*! World camera mode used for capturing the camera frames. */
  MLWorldCameraFrameType frame_type;
} MLWorldCameraFrame;


/*!
  \brief A structure to encapsulate output data for each camera sensor.

  This structure must be initialized by calling #MLWorldCameraDataInit
  before use.

  \apilevel 23
*/
typedef struct MLWorldCameraData {
  /*! Version of this structure. */
  uint32_t version;
  /*! Number of camera frames populated. */
  uint8_t frame_count;
  /*! Camera frame data. The number of frames is specified by frame_count. */
  MLWorldCameraFrame *frames;
} MLWorldCameraData;

/*!
  \brief Initialize MLWorldCameraData with version.

  \apilevel 23

  \param[in,out] inout_world_camera_data Set up the version for inout_world_camera_data.
*/
ML_STATIC_INLINE void MLWorldCameraDataInit(MLWorldCameraData *inout_world_camera_data) {
  if (inout_world_camera_data) {
    memset(inout_world_camera_data, 0, sizeof(MLWorldCameraData));
    inout_world_camera_data->version = 1;
  }
}

/*!
  \brief Connect to world cameras.

  \apilevel 23

  \param[in] settings A pointer to MLWorldCameraSettings structure.
  \param[out] out_handle A pointer to camera handle to be used in later APIs.

  \retval MLResult_InvalidParam One of the parameters is invalid.
  \retval MLResult_Ok Connected to camera device(s) successfully.
  \retval MLResult_PermissionDenied Necessary permission is missing.
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.

  \permissions android.permission.CAMERA (protection level: dangerous)
*/
ML_API MLResult ML_CALL MLWorldCameraConnect(const MLWorldCameraSettings *settings, MLHandle *out_handle);

/*!
  \brief Update the world camera settings.

  \apilevel 23

  \param[in] handle Camera handle obtained from #MLWorldCameraConnect.
  \param[in] settings Pointer to #MLWorldCameraSettings.

  \retval MLResult_InvalidParam Invalid handle.
  \retval MLResult_Ok Settings updated successfully.
  \retval MLResult_UnspecifiedFailure Failed due to internal error.

  \permissions None
*/
ML_API MLResult ML_CALL MLWorldCameraUpdateSettings(MLHandle handle, const MLWorldCameraSettings *settings);

/*!
  \brief Poll for Frames.

  Returns #MLWorldCameraData with this latest data when available. The memory is
  owned by the system. Application should copy the data it needs to cache and
  release the memory by calling #MLWorldCameraReleaseCameraData.

  This is a blocking call. API is not thread safe.

  If there are no new camera frames within the timeout_ms duration then the
  API will return MLResult_Timeout.

  \apilevel 23

  \param[in] handle Camera handle obtained from #MLWorldCameraConnect.
  \param[in] timeout_ms Timeout in milliseconds.
  \param[out] out_data World camera data. Will be set to NULL if no valid data is
              available at this time.

  \retval MLResult_InvalidParam Invalid handle.
  \retval MLResult_Ok World camera data fetched successfully.
  \retval MLResult_Timeout Returned because no new frame available at this time.
  \retval MLResult_UnspecifiedFailure Failed due to internal error.

  \permissions None
*/
ML_API MLResult ML_CALL MLWorldCameraGetLatestWorldCameraData(MLHandle handle, uint64_t timeout_ms, MLWorldCameraData **out_data);

/*!
  \brief Releases specified #MLWorldCameraData object.

  This function should be called exactly once for each successfull call to
  #MLWorldCameraGetLatestCameraData.

  \param[in] handle Camera handle obtained from #MLWorldCameraConnect.
  \param[in] world_camera_data  Pointer to a valid #MLWorldCameraData object.

  \retval MLResult_Ok Successfully released world camera data.
  \retval MLResult_InvalidParam world_camera_data  parameter was not valid (NULL).
  \retval MLResult_UnspecifiedFailure Failed due to internal error.

  \permissions None
*/
ML_API MLResult ML_CALL MLWorldCameraReleaseCameraData(MLHandle handle, MLWorldCameraData *world_camera_data);

/*!
  \brief Disconnect from world camera.

  This will disconnect from all the world camera currently connected.

  \apilevel 23

  \param[in] handle Camera handle obtained from #MLWorldCameraConnect.

  \retval MLResult_InvalidParam Invalid handle.
  \retval MLResult_Ok Disconnected camera successfully.
  \retval MLResult_UnspecifiedFailure Failed to disconnect camera.

  \permissions None
*/
ML_API MLResult ML_CALL MLWorldCameraDisconnect(MLHandle handle);

/*! \} */

ML_EXTERN_C_END

