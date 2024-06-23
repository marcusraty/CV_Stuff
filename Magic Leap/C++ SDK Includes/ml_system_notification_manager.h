// %BANNER_BEGIN%
// ---------------------------------------------------------------------
// %COPYRIGHT_BEGIN%
// Copyright (c) 2023 Magic Leap, Inc. All Rights Reserved.
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

ML_EXTERN_C_BEGIN

/*!
  \defgroup SystemNotificationManager System Notification Manager
  \sharedobject system_notification_manager.magicleap
  \brief Systems Notification Manager allows toggling the system notifications. This API is not threadsafe.

  \{
*/

/*!
  \brief Creates a System Notification Manager handle.

  Multiple calls to this API method from the same applicaiton will return the same handle. The
  handle is valid for the lifecycle of the application.

  \apilevel 24

  \param[out] out_handle The handle to be created.

  \retval MLResult_InvalidParam Failed due to an invalid parameter.
  \retval MLResult_Ok System Notification Manager handle was successfully created.
  \retval MLResult_PermissionDenied Necessary permission is missing.
  \retval MLResult_UnspecifiedFailure System Notification Manager handle failed to be created.

  \permissions com.magicleap.permission.SYSTEM_NOTIFICATION (protection level: normal)
*/
ML_API MLResult ML_CALL MLSystemNotificationManagerCreate(MLHandle* out_handle);

/*!
  \brief Destroys a System Notification Manager handle.

  \apilevel 24

  \param[in] handle The handle to be destroyed.

  \retval MLResult_InvalidParam The handle passed in was not valid.
  \retval MLResult_Ok System Notification Manager handle was successfully destroyed.

  \permissions None
*/
ML_API MLResult ML_CALL MLSystemNotificationManagerDestroy(MLHandle handle);

/*!
  \brief Request suppression/unsuppression of system notifications.

  Requests the system to unsuppress/suppress all notifications. This includes notifications,
  dialogs and alarms from being displayed. Once suppressed, notifications remain suppressed
  even if the application requesting suppression loses focus (ie: if the user navigates away
  from the application).

  If the calling app is closed for any reason (ie. closed by user action, voice command,
  terminal command, or crashed) before notifications were unsuppressed the System
  Notification Manager will automatically unsuppress all notficiations (unless another
  application was currently suppressing notifications).

  \apilevel 24

  \param[in] handle Handle to System Notification Manager.
  \param[in] suppress True to suppress all notifications, false to unsuppress all notifications.

  \retval MLResult_IncompatibleSKU Failed due to feature not being supported on current device
  version.
  \retval MLResult_InvalidParam The handle passed in was not valid.
  \retval MLResult_Ok All system notifications were successfully suppressed/unsuppressed.
  \retval MLResult_UnspecifiedFailure Suppression/Unsuppression of system notifications failed.

  \permissions None
*/
ML_API MLResult ML_CALL MLSystemNotificationManagerSetNotifications(MLHandle handle, bool suppress);

/*!  /} */
ML_EXTERN_C_END
