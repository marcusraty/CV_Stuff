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
#include <string.h>

ML_EXTERN_C_BEGIN

/*!
  \defgroup PowerManager Power Manager
  \addtogroup PowerManager
  \sharedobject power_manager.magicleap
  \brief Power Manager provides a set of power management APIs that allow applications to
  receive callbacks when the power state, properties or error conditions of components change,
  as well as APIs to get device components power state and properties, and to set certain
  component power states.

  These APIs allow an application to function differently depending on its power needs. For
  example an application might use this API to put the controller in the idle state if the
  input mode is hand-tracking only.

  The API uses component specific handles, as such an application should create a handle
  for each kind of component it wishes to get/set power state or properties for.

  \{
*/

enum {
  /*! Defines the prefix for Power Manager return codes. */
  MLResultAPIPrefix_PowerManager = MLRESULT_PREFIX(0x4c8a),
};

/*!
  \brief Power Manager specific return codes.

  \apilevel 27
*/
typedef enum MLPowerManagerResult {
  /*! Indicates the component is not connected. */
  MLPowerManagerResult_NotConnected = MLResultAPIPrefix_PowerManager,

  /*! Indicates the component does not currently support transitioning to requested state. */
  MLPowerManagerResult_InvalidStateTransition,

  /*! Indicates the component does not currently support transitioning to a different state. */
  MLPowerManagerResult_StateTransitionsDisabled,

  /*! Indicates the component does not support the requested power state. */
  MLPowerManagerResult_UnsupportedState,

  /*! Ensure enum is represented as 32 bits. */
  MLPowerManagerResult_Ensure32Bits = 0x7FFFFFFF
} MLPowerManagerResult;

/*!
  \brief Represents the different components which can be accessed/controlled using the Power Manager.

  \apilevel 27
*/
typedef enum MLPowerManagerComponent {
  /*! Invalid or no component. */
  MLPowerManagerComponent_None = 0,

  /*! Controller. */
  MLPowerManagerComponent_Controller = 1,

  /*! Ensure enum is represented as 32 bits. */
  MLPowerManagerComponent_Ensure32Bits = 0x7FFFFFFF
} MLPowerManagerComponent;

/*!
  \brief Power Manager error codes.

  \apilevel 27
*/
typedef enum MLPowerManagerError {
  /*! Controller component could not be connected due to Invalid SKU. */
  MLPowerManagerError_InvalidSKU = 0,

  /*! Ensure enum is represented as 32 bits. */
  MLPowerManagerError_Ensure32Bits = 0x7FFFFFFF
} MLPowerManagerError;

/*!
  \brief Power states.
   Query #MLPowerManagerGetAvailablePowerStates() to determine which power state is available for each component.

  <table>
  <tr><th>              Component     <th>Power State               <th>Description
  <tr><td rowspan="48"> Controller    <td>Normal                    <td>Controller is active
  <tr>                                <td>Disabled While Charging   <td>Certain SKUs cannot use controller while charging
  <tr>                                <td>Standby                   <td>Controller is turned on but inactive, press home button to switch to active manually
  <tr>                                <td>Sleep                     <td>Not supported for this component
  </table>

  \apilevel 27
*/
typedef enum MLPowerManagerPowerState {
  /*! Invalid or no power state. */
  MLPowerManagerPowerState_None = 0,

  /*! Normal mode, this is the default or active state of the component. */
  MLPowerManagerPowerState_Normal = 1,

  /*! Charging only mode. When charging component it cannot be used. */
  MLPowerManagerPowerState_DisabledWhileCharging = 2,

  /*! Standby mode. */
  MLPowerManagerPowerState_Standby = 3,

  /*! Sleep mode. */
  MLPowerManagerPowerState_Sleep = 4,

  /*! Ensure enum is represented as 32 bits. */
  MLPowerManagerPowerState_Ensure32Bits = 0x7FFFFFFF
} MLPowerManagerPowerState;

/*!
  \brief Power Manager charging states.

  \apilevel 27

*/
typedef enum MLPowerManagerChargingState {

  /*! Not charging. */
  MLPowerManagerChargingState_NotCharging = 0,

  /*! Charging normally. */
  MLPowerManagerChargingState_ChargingNormally = 1,

  /*! Ensure enum is represented as 32 bits. */
  MLPowerManagerChargingState_Ensure32Bits = 0x7FFFFFFF
} MLPowerManagerChargingState;

/*!
  \brief Connection state of the component.

  \apilevel 27
*/
typedef enum MLPowerManagerConnectionState {
  /*! Component is connected. */
  MLPowerManagerConnectionState_Connected = 0,

  /*! Component is disconnected. */
  MLPowerManagerConnectionState_Disconnected = 1,

  /*! Ensure enum is represented as 32 bits. */
  MLPowerManagerConnectionState_Ensure32Bits = 0x7FFFFFFF
} MLPowerManagerConnectionState;

/*!
  \brief Battery info/warning codes.

  \apilevel 27
*/
typedef enum MLPowerManagerBatteryInfo {
  /*! No issues reported. */
  MLPowerManagerBatteryInfo_OK = 0,

  /*! Charge the component soon. */
  MLPowerManagerBatteryInfo_BatteryLow = 1,

  /*! Charge the component immediately. */
  MLPowerManagerBatteryInfo_BatteryCritical = 2,

  /*! Ensure enum is represented as 32 bits. */
  MLPowerManagerBatteryInfo_Ensure32Bits = 0x7FFFFFFF
} MLPowerManagerBatteryInfo;

/*!
  \brief Power Manager property types.
  Query #MLPowerManagerGetAvailableProperties() to determine which Power
  Manager properties are available for each component.

  \apilevel 27

*/
typedef enum MLPowerManagerPropertyType {

  /*! Extra info about battery, as represented by MLPowerManagerBatteryInfo. */
  MLPowerManagerPropertyType_BatteryInfo = 0,

  /*! Battery level. Range is between 0 and 100. */
  MLPowerManagerPropertyType_BatteryLevel = 1,

  /*! Charging state. */
  MLPowerManagerPropertyType_ChargingState = 2,

  /*! Connection state. */
  MLPowerManagerPropertyType_ConnectionState = 3,

  /*! Ensure enum is represented as 32 bits. */
  MLPowerManagerPropertyType_Ensure32Bits = 0x7FFFFFFF
} MLPowerManagerPropertyType;

/*!
  \brief A structure to encapsulate the data for each #MLPowerManagerPropertyType.

  This struct and union is used as a flexible way for each component to output
  an array containing distinct types of data.

  The below code shows how a Controller could output battery_info, battery_level,
  charging_state or connection_state. Where data is an instance of #MLPowerManagerPropertyData
  returned from #MLPowerManagerGetComponentProperties.
  \code
  auto properties = data.properties;
    for (int num = 0; num < data.size; num++) {
      switch (properties[num]->property_type) {
        case (MLPowerManagerPropertyType_BatteryInfo):
          // Process properties[num]->battery_info enum
          break;
        case (MLPowerManagerPropertyType_BatteryLevel):
          // Process properties[num]->battery_level as integer
          break;
        case (MLPowerManagerPropertyType_ChargingState):
          // Process properties[num]->charging_state as enum
          break;
        case (MLPowerManagerPropertyType_ConnectionState):
          // Process properties[num]->connection_state as enum
          break;
        default:
          // Error handle if default case
          break;
      }
    }
  \endcode

  \apilevel 27

*/
typedef struct MLPowerManagerComponentProperty {
  /*! The type of each property. */
  MLPowerManagerPropertyType property_type;

  union {
    /*! Extra info about battery. */
    MLPowerManagerBatteryInfo battery_info;

    /*! Battery level. Range is between 0 and 100. */
    uint8_t battery_level;

    /*! Charging state. */
    MLPowerManagerChargingState charging_state;

    /*! Connection state. */
    MLPowerManagerConnectionState connection_state;
  };
} MLPowerManagerComponentProperty;

/*!
  \brief A structure to encapsulate output data when getting the
  current properties.

  \apilevel 27
*/
typedef struct MLPowerManagerPropertyData {
  /*! Size of #properties array. */
  uint8_t size;

  /*! Array of #MLPowerManagerComponentProperty elements. */
  MLPowerManagerComponentProperty *properties;

} MLPowerManagerPropertyData;

/*!
  \brief A structure to encapsulate output data when getting a component's available property types.

  \apilevel 27
*/
typedef struct MLPowerManagerPropertyTypeData {
  /*! Size of #property_types array. */
  uint8_t size;

  /*! Array of #MLPowerManagerPropertyType elements. */
  MLPowerManagerPropertyType *property_types;

} MLPowerManagerPropertyTypeData;

/*!
  \brief A structure to encapsulate information used by the Power Manager
  when getting the available property types.

  This structure must be initialized by calling #MLPowerManagerPropertyTypeInfoInit
  before use.

  \apilevel 27
*/
typedef struct MLPowerManagerPropertyTypeInfo {
  /*! Version of this structure. */
  uint32_t version;

} MLPowerManagerPropertyTypeInfo;


/*!
  \brief Initializes the default values for MLPowerManagerPropertyTypeInfo.

  \apilevel 27

  \param[in,out] inout_property_type_info The object to initialize with default values.
*/
ML_STATIC_INLINE void MLPowerManagerPropertyTypeInfoInit(MLPowerManagerPropertyTypeInfo *inout_property_type_info) {
  if (inout_property_type_info) {
    memset(inout_property_type_info, 0, sizeof(MLPowerManagerPropertyTypeInfo));
    inout_property_type_info->version = 1u;
  }
}


/*!
  \brief A structure to encapsulate info data used by the Power Manager
  when getting the current properties.

  This structure must be initialized by calling #MLPowerManagerPropertyInfoInit
  before use.

  \apilevel 27
*/
typedef struct MLPowerManagerPropertyInfo {
  /*! Version of this structure. */
  uint32_t version;

} MLPowerManagerPropertyInfo;

/*!
  \brief Initializes the default values for MLPowerManagerPropertyInfo.

  \apilevel 27

  \param[in,out] inout_property_info The object to initialize with default values.
*/
ML_STATIC_INLINE void MLPowerManagerPropertyInfoInit(MLPowerManagerPropertyInfo *inout_property_info) {
  if (inout_property_info) {
    memset(inout_property_info, 0, sizeof(MLPowerManagerPropertyInfo));
    inout_property_info->version = 1u;
  }
}

/*!
  \brief A structure to encapsulate settings used by the Power Manager
  when requesting the power state to be changed.

  This structure must be initialized by calling #MLPowerManagerPowerStateSettingsInit
  before use.

  \apilevel 27
*/
typedef struct MLPowerManagerPowerStateSettings {
  /*! Version of this structure. */
  uint32_t version;

  /*! New power state to request. */
  MLPowerManagerPowerState power_state;

} MLPowerManagerPowerStateSettings;

/*!
  \brief Initializes the default values for MLPowerManagerPowerStateSettings.

  \apilevel 27

  \param[in,out] inout_power_settings The object to initialize with default values.
*/
ML_STATIC_INLINE void MLPowerManagerPowerStateSettingsInit(MLPowerManagerPowerStateSettings *inout_power_settings) {
  if (inout_power_settings) {
    memset(inout_power_settings, 0, sizeof(MLPowerManagerPowerStateSettings));
    inout_power_settings->version = 1u;
    inout_power_settings->power_state = MLPowerManagerPowerState_None;
  }
}

/*!
  \brief A structure to encapsulate info data used by the Power Manager
  when getting the current power state.

  This structure must be initialized by calling #MLPowerManagerPowerStateInfoInit
  before use.

  \apilevel 27
*/
typedef struct MLPowerManagerPowerStateInfo {
  /*! Version of this structure. */
  uint32_t version;

} MLPowerManagerPowerStateInfo;


/*!
  \brief Initializes the default values for MLPowerManagerPowerStateInfo.

  \apilevel 27

  \param[in,out] inout_power_info The object to initialize with default values.
*/
ML_STATIC_INLINE void MLPowerManagerPowerStateInfoInit(MLPowerManagerPowerStateInfo *inout_power_info) {
  if (inout_power_info) {
    memset(inout_power_info, 0, sizeof(MLPowerManagerPowerStateInfo));
    inout_power_info->version = 1u;
  }
}


/*!
  \brief A structure to encapsulate output data when either getting
  available power states, or the current power state.

  \apilevel 27
*/
typedef struct MLPowerManagerPowerStateData {
  /*! Size of #power_states array. */
  uint8_t size;

  /*! Array of #MLPowerManagerPowerState elements. */
  MLPowerManagerPowerState *power_states;

} MLPowerManagerPowerStateData;


/*!
  \brief A structure containing Power Manager callback events.
  Individual callbacks which are not required by the Power Manager can be NULL.

  \apilevel 27

*/
typedef struct MLPowerManagerCallbacks {
  /*! Version of this structure. */
  uint32_t version;

  /*!
  \brief This callback will be invoked when #MLPowerManagerPowerState changes.

  \param[in] state #MLPowerManagerPowerState representing the new power state.
  \param[in] user_data User data as passed to the #MLPowerManagerSetCallbacks.

  */
  void (*on_power_state_changed)(MLPowerManagerPowerState state, void *user_data);

  /*!
  \brief This callback will be invoked when #MLPowerManagerPropertyData of a component changes.
  Only the properties that have changed will be returned, the component may support additional
  properties which values were not returned.

  \param[in] properties #MLPowerManagerPropertyData struct encapsulating the properties changed.
  \param[in] user_data User data as passed to the #MLPowerManagerSetCallbacks.
  */
  void (*on_properties_changed)(const MLPowerManagerPropertyData *properties, void *user_data);

  /*!
  \brief This callback will be invoked when an #MLPowerManagerError occurs on one of the components.

  \param[in] error The error which has occurred.
  \param[in] user_data User data as passed to the #MLPowerManagerSetCallbacks.
  */
  void (*on_error_occurred)(MLPowerManagerError error, void *user_data);

} MLPowerManagerCallbacks;

/*!
  \brief Initializes the default values for MLPowerManagerCallbacks.

  \apilevel 27

  \param[in,out] inout_cb The object to initialize with default values.
*/
ML_STATIC_INLINE void MLPowerManagerCallbacksInit(MLPowerManagerCallbacks *inout_cb) {
  if (inout_cb) {
    memset(inout_cb, 0, sizeof(MLPowerManagerCallbacks));
    inout_cb->version = 1u;
    inout_cb->on_power_state_changed = NULL;
    inout_cb->on_properties_changed = NULL;
    inout_cb->on_error_occurred = NULL;
  }
}


/*!
  \brief Creates a Power Manager handle for a specified component.

  In a single application multiple calls to this API method, for the same component type
  will return a new handle each time. The handle is valid until #MLPowerManagerDestroy for that
  specific handle is called.

  \apilevel 27

  \param[in] component The component specific to the handle to be created.
  \param[out] out_handle The handle to be created.

  \retval MLResult_InvalidParam Failed due to an invalid parameter.
  \retval MlResult_HandleExists Handle for this component already exists.
  \retval MLResult_Ok Power Manager handle was successfully created.
  \retval MLResult_UnspecifiedFailure Power Manager handle failed to be created.

  \permissions none
*/
ML_API MLResult ML_CALL MLPowerManagerCreate(MLPowerManagerComponent component, MLHandle* out_handle);

/*!
  \brief Destroys a Power Manager handle.

  \apilevel 27

  \param[in] handle The Power Manager handle for a specific component to be destroyed.

  \retval MLResult_InvalidParam Passed handle was invalid.
  \retval MLResult_Ok Power Manager handle was successfully destroyed.

  \permissions None
*/
ML_API MLResult ML_CALL MLPowerManagerDestroy(MLHandle handle);

/*!
  \brief Register Power Manager callbacks for a specific handle.

  The #MLPowerManagerCallbacks structure can be set for each handle, whether those
  handles are for the same or different component types.

  \apilevel 27

  \param[in] handle Power Manager handle for component to set #MLPowerManagerCallbacks for.
  \param[in] cb Callbacks to receive Power Manager events. Set this to NULL to unregister callbacks.
  \param[in] user_data The caller can pass in user context data that will be
  returned in the callback (can be NULL).

  \retval MLResult_InvalidParam Failed due to an invalid parameter.
  \retval MLResult_Ok The callbacks have been registered.
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.

  \permissions None
*/
ML_API MLResult ML_CALL MLPowerManagerSetCallbacks(MLHandle handle, MLPowerManagerCallbacks *cb,
                                                   void *user_data);

/*!
  \brief Sets the power state of a component.
  The new power state of a component will persist if the application loses focus, or exits.

  \apilevel 27

  \param[in] handle Power Manager handle for component to set power state for.
  \param[in] settings Settings used by the Power Manager updating a component's power state.

  \retval MLResult_IllegalState The component does not support the requested power state.
  \retval MLResult_InvalidParam Failed due to an invalid parameter.
  \retval MLPowerManagerResult_InvalidStateTransition The component does not support
  transitioning to the requested state.
  \retval MLPowerManagerResult_NotConnected The component is not connected.
  \retval MLResult_Ok The power state of the controller was set
  successfully.
  \retval MLPowerManagerResult_StateTransitionsDisabled The component currently does
  not support transitioning to a different state.
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.
  \retval MLPowerManagerResult_UnsupportedState The component does not support the
  requested power state.

  \permissions None

*/

ML_API MLResult ML_CALL MLPowerManagerSetPowerState(MLHandle handle,
                                                    const MLPowerManagerPowerStateSettings *settings);

/*!
  \brief Gets the power manager properties of a component.

  \apilevel 27

  \param[in] handle Power Manager handle for component to get properties of.
  \param[in] in_info #MLPowerManagerPropertyInfo struct filled with information about
  the power manager properties of a component to request.
  \param[out] out_properties Information about the properties of a component. Must be
  released using #MLPowerManagerReleasePropertyData after each successful call.

  \retval MLResult_AllocFailed Failed due to memory allocation failure.
  \retval MLResult_InvalidParam Failed due to an invalid parameter.
  \retval MLResult_Ok The property of the controller was retrieved
  successfully.
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.

  \permissions None

*/

ML_API MLResult ML_CALL MLPowerManagerGetComponentProperties(MLHandle handle,
                                                             const MLPowerManagerPropertyInfo *in_info,
                                                             MLPowerManagerPropertyData *out_properties);


/*!
  \brief Releases specified #MLPowerManagerPropertyData.

  This function should be called exactly once for each successful call to
  #MLPowerManagerGetComponentProperties.

  \param[in] handle Power Manager handle for component relating to #MLPowerManagerPropertyData.
  \param[in] properties Pointer to a #MLPowerManagerPropertyData returned
  from #MLPowerManagerGetComponentProperties.

  \retval MLResult_Ok Successfully released #MLPowerManagerPropertyData.
  \retval MLResult_InvalidParam properties parameter was not valid (NULL).
  \retval MLResult_UnspecifiedFailure Failed due to internal error.

  \permissions None
*/
ML_API MLResult ML_CALL MLPowerManagerReleasePropertyData(MLHandle handle,
                                                          MLPowerManagerPropertyData *properties);

/*!
  \brief Query available power states for a component.

  \apilevel 27

  \param[in] handle Power Manager handle for component to get list of all available
  power states for.
  \param[in] in_info #MLPowerManagerPowerStateInfo struct filled with data
  to be used by the Power Manager when requesting/receiving all available power states.
  \param[out] out_states #MLPowerManagerPowerStateData holding list of available power states.
  Must be released using #MLPowerManagerReleasePowerStateData after each successful call.

  \retval MLResult_AllocFailed Failed due to memory allocation failure.
  \retval MLResult_InvalidParam Failed due to an invalid parameter.
  \retval MLResult_Ok Query completed and out_states has been populated.
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.

  \permissions None
*/
ML_API MLResult ML_CALL MLPowerManagerGetAvailablePowerStates(MLHandle handle,
                                                              const MLPowerManagerPowerStateInfo *in_info,
                                                              MLPowerManagerPowerStateData *out_states);

/*!
  \brief Gets the power state of a component.

  \apilevel 27

  \param[in] handle Power Manager handle for component to get power state from.
  \param[in] in_info #MLPowerManagerPowerStateInfo struct filled with data
  to be used by the Power Manager when requesting/receiving power state.
  \param[out] power_state #MLPowerManagerPowerStateData with the current power
  state. Must be released using #MLPowerManagerReleasePowerStateData
  after each successful call.

  \retval MLPowerManagerResult_NotConnected The component is not connected.
  \retval MLResult_InvalidParam Failed due to an invalid parameter.
  \retval MLResult_Ok The power state of the controller was retrieved
  successfully.
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.

  \permissions None

*/

ML_API MLResult ML_CALL MLPowerManagerGetPowerState(MLHandle handle,
                                                    const MLPowerManagerPowerStateInfo *in_info,
                                                    MLPowerManagerPowerStateData *out_state);

/*!
  \brief Releases specified #MLPowerManagerPowerStateData.

  This function should be called exactly once for each successful call to
  #MLPowerManagerGetAvailablePowerStates.

  \param[in] handle Power Manager handle for component relating to #MLPowerManagerPowerStateData.
  \param[in] power_states Pointer to a #MLPowerManagerPowerStateData.

  \retval MLResult_Ok Successfully released #MLPowerManagerPowerStateData.
  \retval MLResult_InvalidParam power_states parameter was not valid (NULL).
  \retval MLResult_UnspecifiedFailure Failed due to internal error.

  \permissions None
*/
ML_API MLResult ML_CALL MLPowerManagerReleasePowerStateData(MLHandle handle,
                                                            MLPowerManagerPowerStateData *power_states);

/*!
  \brief Request a list of the available #MLPowerManagerPropertyType.

  \apilevel 27

  \param[in] handle Power Manager handle for component to get properties from.
  \param[in] in_info #MLPowerManagerPropertyTypeInfo struct filled with data
  to be used by the Power Manager when requesting/receiving available property types.
  \param[out] out_properties Information about the properties of a component. Must be
  released using #MLPowerManagerReleasePropertyTypeData after each successful call.

  \retval MLResult_AllocFailed Failed due to memory allocation failure.
  \retval MLResult_InvalidParam Failed due to an invalid parameter.
  \retval MLResult_Ok Query completed and out_properties has been populated.
  \retval MLResult_UnspecifiedFailure The operation failed with an unspecified error.

  \permissions None
*/
ML_API MLResult ML_CALL MLPowerManagerGetAvailableProperties(MLHandle handle,
                                                             const MLPowerManagerPropertyTypeInfo *in_info,
                                                             MLPowerManagerPropertyTypeData *out_properties);

/*!
  \brief Releases specified #MLPowerManagerPropertyTypeData.

  This function should be called exactly once for each successful call to
  #MLPowerManagerGetAvailableProperties.

  \param[in] handle Power Manager handle for component relating to #MLPowerManagerPropertyTypeData.
  \param[in] properties Pointer to a #MLPowerManagerPropertyTypeData.

  \retval MLResult_Ok Successfully #MLPowerManagerPropertyTypeData.
  \retval MLResult_InvalidParam properties parameter was not valid (NULL).
  \retval MLResult_UnspecifiedFailure Failed due to internal error.

  \permissions None
*/
ML_API MLResult ML_CALL MLPowerManagerReleasePropertyTypeData(MLHandle handle,
                                                              MLPowerManagerPropertyTypeData *properties);

/*!
  \brief Returns an ASCII string for each result code.

  \apilevel 27

  \param[in] result_code #MLResult to select the result code.
  \retval ASCII string containing readable version of result code.

  \permissions None
*/
ML_API const char *ML_CALL MLPowerManagerGetResultString(MLResult result_code);

/*! \} */

ML_EXTERN_C_END

