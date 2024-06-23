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

#define ALOG_TAG "com.magicleap.capi.sample.system_notifications"
#define SYS_NUM_EVENTS 10

#include <app_framework/application.h>
#include <app_framework/gui.h>
#include <app_framework/components/light_component.h>
#include <app_framework/logging.h>
#include <app_framework/ml_macros.h>
#include <app_framework/toolset.h>
#include <app_framework/version.h>
#include <utility>
#include <ml_audio.h>
#include <ml_system_notification_manager.h>
#include <ml_head_tracking.h>
#include <ml_power_manager.h>


using namespace ml::app_framework;

namespace {
    std::string GetMLHeadTrackingErrorString(uint32_t error) {
      std::string result;
      if (error & MLHeadTrackingErrorFlag_NotEnoughFeatures) result += "MLHeadTrackingErrorFlag_NotEnoughFeatures\n";
      if (error & MLHeadTrackingErrorFlag_LowLight) result += "MLHeadTrackingErrorFlag_LowLight\n";
      if (error & MLHeadTrackingErrorFlag_ExcessiveMotion) result += "MLHeadTrackingErrorFlag_ExcessiveMotion\n";
      if (error & MLHeadTrackingErrorFlag_Unknown) result += "MLHeadTrackingErrorFlag_Unknown\n";
      if (error == MLHeadTrackingErrorFlag_None) result += "MLHeadTrackingErrorFlag_None\n";
      return result;
    }

  std::string GetMLPowerManagerErrorString(const MLPowerManagerError &error) {
    switch (error)
    {
      case MLPowerManagerError_InvalidSKU:
        return "Invalid SKU";
      default:
        return "Undefined error enum";
    }
  }

  std::string GetMLPowerManagerPowerStateString(const MLPowerManagerPowerState &state) {
    switch (state)
    {
      case MLPowerManagerPowerState_Normal:
        return "Normal power state.";
        break;
      case MLPowerManagerPowerState_DisabledWhileCharging:
        return "Component disabled while charging.";
        break;
      case MLPowerManagerPowerState_Standby:
        return "Standby power state.";
        break;
      case MLPowerManagerPowerState_Sleep:
        return "Sleep power state.";
        break;
      case MLPowerManagerPowerState_None:
      case MLPowerManagerPowerState_Ensure32Bits:
      default:
        return "Invalid power state.";
        break;
    }
  }

  std::string GetMLPowerManagerConnectionStateString(const MLPowerManagerConnectionState &state) {
    switch (state)
    {
      case MLPowerManagerConnectionState_Connected:
        return "Connected.";
        break;
      case MLPowerManagerConnectionState_Disconnected:
        return "Diconnected.";
        break;
      case MLPowerManagerConnectionState_Ensure32Bits:
      default:
        return "Invalid connection state.";
        break;
    }
  }
}

class SystemNotificationsApp : public Application {
 public:
  SystemNotificationsApp(struct android_app *state)
    : ml::app_framework::Application(state,USE_GUI),
      available_space_ratio_(0.0),
      compute_critical_(false),
      compute_pack_battery_level_(0),
      compute_pack_battery_temperature_(0.0),
      compute_pack_battery_temperature_warning_(false),
      controller_battery_level_(0),
      controller_critical_(false),
      controller_connection_state_{MLPowerManagerConnectionState_Connected},
      controller_power_state_{MLPowerManagerPowerState_Normal},
      head_tracker_(ML_INVALID_HANDLE),
      head_tracker_error_(MLHeadTrackingErrorFlag_None),
      internet_connection_(false),
      network_connection_(false),
      power_manager_handle_(ML_INVALID_HANDLE),
      space_warning_(false),
      system_ui_comms_suppressed_(false),
      system_ui_tracker_(ML_INVALID_HANDLE),
      volume_warning_(false) {
    events_.reserve(SYS_NUM_EVENTS);
  }

  void OnResume() override {
    if (ArePermissionsGranted()) {
      UNWRAP_MLRESULT(MLHeadTrackingCreate(&head_tracker_));
      using namespace ml::app_framework;
      std::shared_ptr<Node> light_node = std::make_shared<ml::app_framework::Node>();
      std::shared_ptr<LightComponent> light_component = std::make_shared<LightComponent>();
      light_node->AddComponent(light_component);
      GetRoot()->AddChild(light_node);
      GetGui().Show();
    }
  }

  void OnCreate(const void*, size_t) override {
    UNWRAP_MLRESULT(MLSystemNotificationManagerCreate(&system_ui_tracker_));
    UNWRAP_MLRESULT(MLPowerManagerCreate(MLPowerManagerComponent_Controller, &power_manager_handle_));
    //set initial states;
    network_connection_ = IsNetworkConnected();
    internet_connection_ =  IsInternetAvailable();

    // Set up callbacks for controller connection/status errors
    MLPowerManagerCallbacks callbacks = {};
    MLPowerManagerCallbacksInit(&callbacks);

    callbacks.on_error_occurred = &SystemNotificationsApp::OnControllerError;
    callbacks.on_power_state_changed = &SystemNotificationsApp::OnControllerPowerStateChange;
    callbacks.on_properties_changed = &SystemNotificationsApp::OnControllerPropertiesChange;

    UNWRAP_MLRESULT(MLPowerManagerSetCallbacks(power_manager_handle_, &callbacks, this));

    // Get initial states for power manager
    MLPowerManagerPowerStateInfo controller_power_state_info;
    MLPowerManagerPowerStateInfoInit(&controller_power_state_info);
    MLPowerManagerPowerStateData controller_power_state_data;
    auto result = MLPowerManagerGetPowerState(power_manager_handle_, &controller_power_state_info, &controller_power_state_data);
    if (result != MLResult_Ok) {
      ALOGE("ERROR: could not set initial power state: %s", MLGlobalGetResultString(result));
    }
    else {
      controller_power_state_ = *controller_power_state_data.power_states;
    }
    MLPowerManagerReleasePowerStateData(power_manager_handle_, &controller_power_state_data);

    MLPowerManagerPropertyInfo controller_properties_info;
    MLPowerManagerPropertyInfoInit(&controller_properties_info);
    MLPowerManagerPropertyData controller_property_data;
    MLPowerManagerGetComponentProperties(power_manager_handle_, &controller_properties_info, &controller_property_data);
    if (result != MLResult_Ok) {
      ALOGE("ERROR: could not set initial properties: %s", MLGlobalGetResultString(result));
    }
    else {
      auto properties = controller_property_data.properties;
      for (int num = 0 ; num < controller_property_data.size ; num++) {
        if (properties->property_type == MLPowerManagerPropertyType_ConnectionState) {
          controller_connection_state_ = properties->connection_state;
          break;
        }
        properties++;
      }
    }
  }

  void OnStop() override {
    if (MLHandleIsValid(head_tracker_)) {
      UNWRAP_MLRESULT(MLHeadTrackingDestroy(head_tracker_));
    }
  }

  void OnDestroy() override {
    if (MLHandleIsValid(system_ui_tracker_)) {
      UNWRAP_MLRESULT(MLSystemNotificationManagerDestroy(system_ui_tracker_));
    }
    if (MLHandleIsValid(power_manager_handle_)) {
      UNWRAP_MLRESULT(MLPowerManagerDestroy(power_manager_handle_));
    }
  }

  void OnLowMemory() override {
    AddEvent("Memory Warning: APP_CMD_LOW_MEMORY lifecycle event occurred.\n");
  }

  static void OnControllerError(MLPowerManagerError error, void *context) {
    SystemNotificationsApp *app = static_cast<SystemNotificationsApp *>(context);
    if (app) {
      if (error == MLPowerManagerError_InvalidSKU) {
        app->AddEvent("Incompatible charger: cannot use this controller SKU with this compute pack SKU.\n");
      } else {
        ALOGE("ERROR: Unknown MLPowerManagerError: %s\n",
              GetMLPowerManagerErrorString(error).c_str());
      }
    }
    else {
        ALOGE("ERROR: Unable to set event string in OnControllerError");
    }
  }

  static void OnControllerPowerStateChange(MLPowerManagerPowerState state, void *context) {
    SystemNotificationsApp *app = static_cast<SystemNotificationsApp *>(context);
    if (app) {
      switch (state) {
        case MLPowerManagerPowerState_Normal:
          app->AddEvent("Controller entered normal power state.\n");
          app->controller_power_state_ = MLPowerManagerPowerState_Normal;
          break;
        case MLPowerManagerPowerState_DisabledWhileCharging:
          app->AddEvent("Controller cannot be used while connected to charging for this SKU.\n");
          app->controller_power_state_ = MLPowerManagerPowerState_DisabledWhileCharging;
          break;
        case MLPowerManagerPowerState_Standby:
          app->AddEvent("Controller entered standby power state.\n");
          app->controller_power_state_ = MLPowerManagerPowerState_Standby;
          break;
        case MLPowerManagerPowerState_None:
        case MLPowerManagerPowerState_Sleep:
        case MLPowerManagerPowerState_Ensure32Bits:
        default:
          app->AddEvent("Invalid power state detected for controller.\n");
          break;
      }
    }
    else {
      ALOGE("ERROR: Unable to set event string in OnControllerError");
    }
  }

  static void OnControllerPropertiesChange(const MLPowerManagerPropertyData *property_data, void *context) {
    SystemNotificationsApp *app = static_cast<SystemNotificationsApp *>(context);
    if (app) {
      auto properties = property_data->properties;
      for (int num = 0 ; num < property_data->size ; num++) {
        if (properties->property_type == MLPowerManagerPropertyType_ConnectionState) {
          switch (properties->connection_state) {
            case MLPowerManagerConnectionState_Connected:
              app->AddEvent("Controller has been disconnected.\n");
              app->controller_connection_state_ = MLPowerManagerConnectionState_Connected;
              break;
            case MLPowerManagerConnectionState_Disconnected:
              app->AddEvent("Controller has been connected.\n");
              app->controller_connection_state_ = MLPowerManagerConnectionState_Disconnected;
              break;
            default:
              ALOGW("WARNING: unexpected property found: %d", properties->connection_state);
              break;
          }
        }
        properties++;
      }
    }
    else {
      ALOGE("ERROR: Unable to set event string in OnControllerError");
    }
  }

  void OnUpdate(float) override {
    CheckSystemEvents();
    auto & gui = GetGui();
    bool continue_running = true;
    gui.BeginUpdate();
    gui.BeginDialog("System Notification Manager Sample Application", &continue_running);
    ImGui::Text("The System Notification Manager is only available on certain device SKUs.");
    ImGui::NewLine();
    ImGui::Text("Actions:");
    bool value = system_ui_comms_suppressed_;
    if (ImGui::Checkbox("Suppress System Notifications", &value)) {
      SuppressSysUiComms(value);
    }
    ImGui::NewLine();
    ImGui::Text("System Status:");
    ImGui::Text("Internet: %s", internet_connection_ ? "connected" : "disconnected");
    ImGui::Text("Network: %s", network_connection_ ? "connected" : "disconnected");
    ImGui::Text("Controller Battery Percentage: %s ", IsControllerPresent() ? std::to_string(controller_battery_level_).c_str() : "not connected");
    ImGui::Text("Compute Pack Battery Percentage: %d ", compute_pack_battery_level_);
    ImGui::Text("Compute Pack Battery Temperature: %f ", compute_pack_battery_temperature_);
    ImGui::Text("Available Disk Space Free Ratio: %f (status: %s)", available_space_ratio_, available_space_ratio_ <= 0.1 ? "Critical" : "OK");
    ImGui::Text("Head Tracking Status: %s", GetMLHeadTrackingErrorString(head_tracker_error_).c_str());
    ImGui::Text("Controller Power State: %s", GetMLPowerManagerPowerStateString(controller_power_state_).c_str());
    ImGui::Text("Controller Connection State: %s", GetMLPowerManagerConnectionStateString(controller_connection_state_).c_str());

    ImGui::NewLine();
    if (ImGui::Button("Clear Event Stream")) {
      events_.clear();
    }
    ImGui::NewLine();
    ImGui::Text("System Notification Event Stream:");
    
    for (const auto& event : events_) {
      ImGui::Text("%s", event.c_str());
    }
    gui.EndDialog();
    gui.EndUpdate();

    if (!continue_running) {
        FinishActivity();
    }
  }

  void AddEvent(const std::string& event) {
    if ((int)events_.size() > SYS_NUM_EVENTS) {
      events_.erase(events_.begin());
    }
    events_.push_back(event);
  }
  
private:
  void SuppressSysUiComms(const bool suppress) {
    MLResult result = MLSystemNotificationManagerSetNotifications(system_ui_tracker_, suppress);
    if (result != MLResult_Ok) {
      ALOGE("failed to %s System UI Comms; got %s from suppression request",
            suppress ? "suppress" : "unsuppress",
            MLGetResultString(result));
      return;
    }
    system_ui_comms_suppressed_ = suppress;
    ALOGI("successfully %s System UI comms with return code: %s",
          suppress ? "suppressed" : "unsuppressed",
          MLGetResultString(result));
  }

  void CheckSystemEvents() {
    bool previous_network_connection = network_connection_;
    network_connection_ = IsNetworkConnected();
    if (previous_network_connection != network_connection_) {
      if (network_connection_) {
        AddEvent("Network Connected.\n");
      }
      else {
        AddEvent("Network Disconnected.\n");
      }
    }

    bool previous_internet_connection = internet_connection_;
    internet_connection_ = IsInternetAvailable();
    if (previous_internet_connection != internet_connection_) {
      if (internet_connection_) {
        AddEvent("Internet Connected.\n");
      }
      else {
        AddEvent("Internet Disconnected.\n");
      }
    }

    compute_pack_battery_level_ = GetComputePackBatteryLevel();
    if (compute_pack_battery_level_ <= 5 && !compute_critical_) {
      AddEvent("Compute Pack Battery Critically Low (less than %5).\n");
      compute_critical_ = true;
    }
    if (compute_critical_ && compute_pack_battery_level_ > 5) {
      compute_critical_ = false;
    }

    if (IsControllerPresent()) {
      controller_battery_level_ = GetControllerBatteryLevel();
      if (controller_battery_level_ <= 5 && !controller_critical_) {
        AddEvent("Controller Battery Critically Low (less than %5).\n");
        controller_critical_ = true;
      }
      if (controller_critical_ && controller_battery_level_ > 5) {
        controller_critical_ = false;
      }
    }

    available_space_ratio_ = float(GetAvailableDiskBytes()+GetAvailableExternalBytes())/float(GetTotalDiskBytes()+GetTotalExternalBytes());
    if (available_space_ratio_ <= 0.1 && !space_warning_) {
      AddEvent("Available space is critically low (less than 10%).\n");
      space_warning_ = true;
    }
    if (space_warning_ && available_space_ratio_ > 0.1) {
      space_warning_ = false;
    }

    float audio_volume;
    MLAudioGetMasterVolume(&audio_volume);
    if ((audio_volume >= 75.0) && !volume_warning_) {
      AddEvent("High volume warning: consider lowering volume. \n");
      volume_warning_ = true;
    }
    if (volume_warning_ && audio_volume < 75.0) {
      volume_warning_ = false;
    }

    compute_pack_battery_temperature_ = GetComputePackBatteryTemperature();
    if (!compute_pack_battery_temperature_warning_ & (compute_pack_battery_temperature_>=40.0)) {
      AddEvent("Compute Pack Temperature Warning: greater than 40 degrees Celsius.\n");
      compute_pack_battery_temperature_warning_ = true;
    }

    if (compute_pack_battery_temperature_warning_ & (compute_pack_battery_temperature_ < 40.0)) {
      compute_pack_battery_temperature_warning_ = false;
    }

    MLHeadTrackingStateEx cur_state;
    UNWRAP_MLRESULT(MLHeadTrackingGetStateEx(head_tracker_, &cur_state));
    uint32_t previous_head_tracker_error = head_tracker_error_;
    head_tracker_error_ = cur_state.error;
    if (previous_head_tracker_error != head_tracker_error_) {
      if (head_tracker_error_ & MLHeadTrackingErrorFlag_LowLight) {
        AddEvent("Head tracking lost due to low light conditions.\n");
      }
      if (head_tracker_error_ & MLHeadTrackingErrorFlag_NotEnoughFeatures) {
        AddEvent("Head tracking lost because there are not enough features.\n");
      }
      if (head_tracker_error_ & MLHeadTrackingErrorFlag_ExcessiveMotion) {
        AddEvent("Head tracking lost because of excessive motion.\n");
      }
      if (head_tracker_error_ & MLHeadTrackingErrorFlag_Unknown) {
        AddEvent("Head tracking lost due to unknown error.\n");
      }
      if (head_tracker_error_ == MLHeadTrackingError_None) {
        AddEvent("Head tracking restored.\n");
      }
    }

    int previous_memory_trim_level_ = memory_trim_level_;
    memory_trim_level_ = GetLastTrimLevel();
    if (previous_memory_trim_level_!=memory_trim_level_)
    {
      if (memory_trim_level_ == 10) {
        AddEvent("Memory warning: memory running low.\n");
      }
      if (memory_trim_level_ == 15) {
        AddEvent("Memory warning: memory running critically low.\n");
      }
    }
  }

  float available_space_ratio_;
  bool compute_critical_;
  int compute_pack_battery_level_;
  float compute_pack_battery_temperature_;
  bool compute_pack_battery_temperature_warning_;
  int controller_battery_level_;
  bool controller_critical_;
  MLPowerManagerConnectionState controller_connection_state_;
  MLPowerManagerPowerState controller_power_state_;
  std::vector<std::string> events_;
  MLHandle head_tracker_;
  uint32_t head_tracker_error_;
  bool internet_connection_;
  bool network_connection_;
  MLHandle power_manager_handle_;
  bool space_warning_;
  bool system_ui_comms_suppressed_;
  MLHandle system_ui_tracker_;
  int memory_trim_level_;
  bool volume_warning_;


};

void android_main(struct android_app *state) {
#ifndef ML_LUMIN
  ALOGE("This app is not supported on app simulator.");
#else
  SystemNotificationsApp app(state);
  ALOGI("%s built against app_framework %s", ALOG_TAG, ML_APP_FRAMEWORK_VERSION_NAME);
  app.RunApp();
#endif
}
