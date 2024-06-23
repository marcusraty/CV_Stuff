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

#define ALOG_TAG "com.magicleap.capi.sample.world_camera"

#include <app_framework/application.h>
#include <app_framework/components/renderable_component.h>
#include <app_framework/geometry/quad_mesh.h>
#include <app_framework/gui.h>
#include <app_framework/logging.h>
#include <app_framework/material/textured_grayscale_material.h>
#include <app_framework/registry.h>
#include <app_framework/toolset.h>

#include <map>
#include <unordered_map>
#include <utility>

#include <ml_perception.h>
#include <ml_time.h>
#include <ml_world_camera.h>


using namespace ml::app_framework;

namespace {

    typedef std::pair<MLWorldCameraIdentifier, MLWorldCameraFrameType> CameraIdModePair;

    const char *GetMLWorldCameraIdentifierString(const MLWorldCameraIdentifier &camera_id) {
      switch (camera_id) {
        case MLWorldCameraIdentifier_Left:
          return "Left Camera";
        case MLWorldCameraIdentifier_Right:
          return "Right Camera";
        case MLWorldCameraIdentifier_Center:
          return "Center Camera";
        case MLWorldCameraIdentifier_All:
          return "All Cameras";
        default:
          return "Error";
      }
    }

    const char *GetMLWorldCameraFrameTypeString(const MLWorldCameraFrameType &camera_mode) {
      switch (camera_mode) {
        case MLWorldCameraFrameType_Unknown:
          return "Unknown";
        case MLWorldCameraFrameType_LowExposure:
          return "Low Exposure";
        case MLWorldCameraFrameType_NormalExposure:
          return "Normal Exposure";
        default:
          return "Error";
      }
    }
}

class WorldCameraApp : public Application {
public:
    WorldCameraApp(struct android_app *state)
            : Application(state, std::vector<std::string>{"android.permission.CAMERA"}, USE_GUI),
              preview_initialized_(false),
              texture_width_(1016),
              texture_height_(1016),
              world_camera_handle_(ML_INVALID_HANDLE) {
      // Start with all cameras and modes active
      available_cameras_[MLWorldCameraIdentifier_Left] = true;
      available_cameras_[MLWorldCameraIdentifier_Center] = true;
      available_cameras_[MLWorldCameraIdentifier_Right] = true;
      available_modes_[MLWorldCameraFrameType_NormalExposure] = true;
      available_modes_[MLWorldCameraFrameType_LowExposure] = true;

      for (const auto& [camera, _] : available_cameras_) {
        for (const auto& [mode, __] : available_modes_) {
          const auto camera_mode_pair = std::make_pair(camera, mode);
          texture_ids_[camera_mode_pair] = 0;

          // Set to -1 to signify we haven't seen any frames before
          last_frame_num_[camera_mode_pair] = -1;
          dropped_frame_count[camera_mode_pair] = 0;
          text_offsets_[camera_mode_pair] = glm::vec3{-.5f, 0.77f, 0.f};

          // Change these to tune location of displays
          switch (camera) {
            case MLWorldCameraIdentifier_Center:
              preview_offsets_[camera_mode_pair] = glm::vec3{0, -0.3, -2.5};
              break;
            case MLWorldCameraIdentifier_Left:
              preview_offsets_[camera_mode_pair] = glm::vec3{-0.6, -0.3, -2.5};
              break;
            case MLWorldCameraIdentifier_Right:
              preview_offsets_[camera_mode_pair] = glm::vec3{0.6, -0.3, -2.5};
              break;
            default:
              break; // Do nothing
          }

          // Change this to tune the distance between low and normal exposure displays
          if (mode == MLWorldCameraFrameType_LowExposure) {
            preview_offsets_[camera_mode_pair].y += 0.7f;
          }

          last_frame_info_.emplace(camera_mode_pair, MLWorldCameraFrame{});

        }
      }
    }

    void OnStart() override {
      MLWorldCameraSettingsInit(&world_camera_settings_);
      world_camera_settings_.cameras = MLWorldCameraIdentifier_All;
      world_camera_settings_.mode =
              MLWorldCameraFrameType_LowExposure | MLWorldCameraFrameType_NormalExposure;
    }

    void OnResume() override {
      if (ArePermissionsGranted()) {
        SetupRestrictedResources();
        GetGui().Show();
      }
    }

    void OnPause() override {
      if (MLHandleIsValid(world_camera_handle_)) {
        UNWRAP_MLRESULT(MLWorldCameraDisconnect(world_camera_handle_));
        world_camera_handle_ = ML_INVALID_HANDLE;
      }
      // Need to reset the last frame number so that those frames are not counted as dropped
      for (auto& [_,last_frame_num] : last_frame_num_) {
        last_frame_num = -1;
      }
    }

    void OnPreRender() override {
      if (!MLHandleIsValid(world_camera_handle_)) {
        return;
      }
      MLWorldCameraData data;
      MLWorldCameraData *data_ptr = &data;
      MLWorldCameraDataInit(data_ptr);
      MLResult result = MLWorldCameraGetLatestWorldCameraData(world_camera_handle_, 0, &data_ptr);

      // Push frames into map to check each camera has 1 frame per data object,
      // and to ensure order of cameras in GUI is always the same
      std::map<CameraIdModePair, MLWorldCameraFrame> processed_cameras;

      if (result == MLResult_Ok) {
        if (data.frame_count < 1) {
          ALOGW("ERROR: received MLWorldCameraData with less than 1 frame count. Cannot process this data.");
          UNWRAP_MLRESULT(MLWorldCameraReleaseCameraData(world_camera_handle_, &data));
          return;
        }

        // Update display to preview image
        for (int current_frame = 0; current_frame < data.frame_count; current_frame++) {
          const auto frame = &data.frames[current_frame];
          const auto camera =  frame->id;
          const auto mode = frame->frame_type;

          if (mode == MLWorldCameraFrameType_Unknown) {
            ALOGE("ERROR: cannot process unknown mode, skipping frame.");
            continue;
          }

          // uint8_t is the type of MLWorldCameraFrameBuffer.data field
          if (frame->frame_buffer.bytes_per_pixel != sizeof(uint8_t)) {
            ALOGE("Bytes per pixel equal to %d, instead of %ld! Data alignment mismatch for %s %s, skipping frame!",
                  frame->frame_buffer.bytes_per_pixel, sizeof(GL_UNSIGNED_BYTE),
                  GetMLWorldCameraIdentifierString(camera), GetMLWorldCameraFrameTypeString(mode));
            continue;
          }

          const auto camera_mode_pair = std::make_pair(camera, mode);
          if (processed_cameras.insert(std::make_pair(camera_mode_pair, *frame)).second == false) {
            ALOGW("WARNING: camera: %s mode: %s had two frames processed. It is expected that each MLWorldCameraData has only 1 frame for each camera. Not processing second this frame.",
                  GetMLWorldCameraIdentifierString(camera), GetMLWorldCameraFrameTypeString(mode));
            continue;
          }

          glBindTexture(GL_TEXTURE_2D, texture_ids_[camera_mode_pair]);
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texture_width_, texture_height_, 0, GL_RED,
                       GL_UNSIGNED_BYTE, frame->frame_buffer.data);
          glBindTexture(GL_TEXTURE_2D, 0);

          const std::string label = std::string(GetMLWorldCameraIdentifierString(camera)) + "\n" +
                              std::string(GetMLWorldCameraFrameTypeString(mode)) +
                              "\nFrame Number: " + std::to_string(frame->frame_number);

          SetNodeText(camera_mode_pair, label.c_str());

          // Save new frame data to member variable for display on GUI
          last_frame_info_[camera_mode_pair] = *frame;
        }
        UNWRAP_MLRESULT(MLWorldCameraReleaseCameraData(world_camera_handle_, &data));
        CheckDroppedFrames(processed_cameras);
      } else {
        ALOGW("MLWorldCameraGetLatestWorldCameraData returned error: %s!",
              MLGetResultString(result));
      }
      UpdateGuiConsole();
    }

private:
    void SetNodeText(CameraIdModePair camera_mode_pair, const char * label) {
      // Display_nodes_ contains preview_node_
      for (const auto& first_child : display_nodes_[camera_mode_pair]->GetChildren()) {
        for (const auto& second_child : first_child->GetChildren()) {
          auto component = second_child->GetComponent<TextComponent>();
          if (component) {
            component->SetText(label);
          }
        }
      }
    }

    void CheckDroppedFrames(const std::map<CameraIdModePair, MLWorldCameraFrame> &processed_cameras) {
      for (const auto& [camera_mode_pair, frame] : processed_cameras) {
        if ((available_cameras_[camera_mode_pair.first] == false) ||
            (available_modes_[camera_mode_pair.second] == false)) {
          continue;
        }

        if (frame.frame_number < 0) {
          ALOGE("ERROR: %s %s returned an invalid frame number: %ld",
                GetMLWorldCameraIdentifierString(camera_mode_pair.first),
                GetMLWorldCameraFrameTypeString(camera_mode_pair.second), frame.frame_number);
          continue;
        }

        // Check for dropped frames only if last_frame_num_ has been initialized
        if (last_frame_num_[camera_mode_pair] != -1) {
          if (frame.frame_number == last_frame_num_[camera_mode_pair]) {
            ALOGE("ERROR: %s %s received the same frame number twice: %ld",
                  GetMLWorldCameraIdentifierString(camera_mode_pair.first),
                  GetMLWorldCameraFrameTypeString(camera_mode_pair.second), frame.frame_number);
            continue;
          }

          int64_t frame_num_diff;
          // Detect if frame number rolled over
          if (frame.frame_number < last_frame_num_[camera_mode_pair]) {
            frame_num_diff = (INT64_MAX - last_frame_num_[camera_mode_pair]) + frame.frame_number;
            // Frame rolls over to 0 so add 1
            frame_num_diff += 1;
          } else {
            frame_num_diff = frame.frame_number - last_frame_num_[camera_mode_pair];
          }
          // Deal with both normal increment by 1 and rollover to 0 (add 1 above)
          if (frame_num_diff > 1) {
            dropped_frame_count[camera_mode_pair] = dropped_frame_count[camera_mode_pair] + frame_num_diff;
          }
        }
        // Always update last_frame_num_
        last_frame_num_[camera_mode_pair] = frame.frame_number;
      }
    }

    void UpdateGuiConsole() {
      auto &gui = GetGui();
      gui.BeginUpdate();
      bool is_running = true;

      if (gui.BeginDialog("World Camera Information and Settings", &is_running,
                          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                          ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
        DrawSettingsDialog();

        for (const auto& [camera, camera_status] : available_cameras_) {
          if (camera_status == false) {
            continue;
          }
          if (ImGui::CollapsingHeader(GetMLWorldCameraIdentifierString(camera))) {
            for (const auto& [mode, mode_status] : available_modes_) {
              if (mode_status == false) {
                continue;
              }
              const auto camera_mode_pair = std::make_pair(camera, mode);
              const auto it = last_frame_info_.find(camera_mode_pair);
              if (it == last_frame_info_.end()) {
                ALOGE("ERROR: could not find camera: %s and mode %s in last_frame_info_, skipping.",
                      GetMLWorldCameraIdentifierString(camera),
                      GetMLWorldCameraFrameTypeString(mode));
                continue;
              }
              const auto label = std::string(GetMLWorldCameraIdentifierString(camera)) + " " +
                           GetMLWorldCameraFrameTypeString(mode);
              if (ImGui::CollapsingHeader(label.c_str())) {
                const auto frame = it->second;

                ImGui::Text("\tFrame number: %ld", frame.frame_number);
                ImGui::Text("\tDropped frames: %d", dropped_frame_count[camera_mode_pair]);

                timespec ts = {};
                UNWRAP_MLRESULT(MLTimeConvertMLTimeToSystemTime(frame.timestamp, &ts));
                ImGui::Text("\tElapsed time: %ld seconds and %ld nanoseconds", ts.tv_sec, ts.tv_nsec);

                ImGui::Text("\tCamera position xyz: (%.2f, %.2f, %.2f)",
                            frame.camera_pose.position.x,
                            frame.camera_pose.position.y, frame.camera_pose.position.z);
                ImGui::Text("\tCamera rotation xyzw: (%.2f, %.2f, %.2f, %.2f)",
                            frame.camera_pose.rotation.x, frame.camera_pose.rotation.y,
                            frame.camera_pose.rotation.z, frame.camera_pose.rotation.w);

                ImGui::NewLine();
                DrawIntrinsicDetails("Intrinsics:", frame.intrinsics);
              }
            }
          }
        }
        ImGui::Separator();
        ImGui::NewLine();
      }

      gui.EndDialog();
      gui.EndUpdate();

      if (!is_running) {
        FinishActivity();
      }
    }

    void DrawIntrinsicDetails(const char *label, const MLWorldCameraIntrinsics &params) {
      if (ImGui::CollapsingHeader(label)) {
        ImGui::Text("Camera width: %d", params.width);
        ImGui::Text("Camera height: %d", params.height);
        ImGui::Text("Camera focal length x: %.4f y: %.4f", params.focal_length.x,
                    params.focal_length.y);
        ImGui::Text("Camera principal point: x: %.4f y: %.4f", params.principal_point.x,
                    params.principal_point.y);
        ImGui::Text("Camera field of view: %.4f", params.fov);
        ImGui::Text("Camera radial distortion params k1, k2, k3, k4:\n\t\t%.4f %.4f %.4f %.4f",
                    params.radial_distortion[0], params.radial_distortion[1],
                    params.radial_distortion[2], params.radial_distortion[3]);
        ImGui::Text("Camera tangential distortion params p1, p2:\n\t\t%.4f %.4f",
                    params.tangential_distortion[0], params.tangential_distortion[1]);
      }
    }

    void UpdateCameraMode(MLWorldCameraFrameType mode, bool state) {
      for (const auto& [camera, status] : available_cameras_) {
        if (status == false) {
          continue;
        }
        const auto camera_mode_pair = std::make_pair(camera, mode);
        SetPreviewVisibility(display_nodes_[camera_mode_pair], state);
        last_frame_num_[camera_mode_pair] = -1;
      }
      if (state) {
        world_camera_settings_.mode = world_camera_settings_.mode | mode;
      } else {
        world_camera_settings_.mode = world_camera_settings_.mode & ~mode;
      }
    }

    void UpdateCameraId(MLWorldCameraIdentifier id, bool state) {

      for (const auto& [mode, status] : available_modes_) {
        if (status == false) {
          continue;
        }
        const auto camera_mode_pair = std::make_pair(id, mode);
        SetPreviewVisibility(display_nodes_[camera_mode_pair], state);
        last_frame_num_[camera_mode_pair] = -1;
      }
      if (state) {
        world_camera_settings_.cameras = world_camera_settings_.cameras | id;
      } else {
        world_camera_settings_.cameras = world_camera_settings_.cameras & ~id;
      }
    }

    void SetPreviewVisibility(std::shared_ptr<Node> node, bool state) {
      // Display_nodes_ contains preview_node_
      for (auto first_child : node->GetChildren()) {
        // Preview_node contains gui and text
        for (auto second_child : first_child->GetChildren()) {
          auto component = second_child->GetComponent<RenderableComponent>();
          if (component) {
            component->SetVisible(state);
          }
        }
      }
    }

    void DrawSettingsDialog() {
      bool settings_updated = false;
      ImGui::Text("Modes:");
      ImGui::SameLine();
      for (const auto& [mode, _] : available_modes_) {
        const std::string label = std::string(GetMLWorldCameraFrameTypeString(mode)) + " Mode";
        ImGui::SameLine();
        if (ImGui::Checkbox(label.c_str(), &available_modes_[mode])) {
          UpdateCameraMode(mode, available_modes_[mode]);
          settings_updated = true;
        }
      }

      ImGui::Text("Cameras:");
      ImGui::SameLine();
      for (const auto& [camera, _] : available_cameras_) {
        ImGui::SameLine();
        if (ImGui::Checkbox(GetMLWorldCameraIdentifierString(camera),
                            &available_cameras_[camera])) {
          UpdateCameraId(camera, available_cameras_[camera]);
          settings_updated = true;
        }
      }

      if (settings_updated) {
        UNWRAP_MLRESULT(MLWorldCameraUpdateSettings(world_camera_handle_, &world_camera_settings_));
      }
    }

    void SetupRestrictedResources() {
      if (MLHandleIsValid(world_camera_handle_)) {
        ALOGV("Handle already valid.");
        return;
      }
      UNWRAP_MLRESULT(MLWorldCameraConnect(&world_camera_settings_, &world_camera_handle_));
      SetupPreview();
    }

    void DestroyPreview() {
      for (const auto& [camera, _] : available_cameras_) {
        for (const auto& [mode, __] : available_modes_) {
          const auto camera_mode_pair = std::make_pair(camera, mode);
          GetRoot()->RemoveChild(display_nodes_[camera_mode_pair]);
          display_nodes_[camera_mode_pair].reset();
          texture_ids_[camera_mode_pair] = 0;
        }
      }
    }

    void SetupPreview() {
      // DestroyPreview() before reinit for OnResume()
      if (preview_initialized_) {
        DestroyPreview();
      }

      for (const auto& [camera, camera_state] : available_cameras_) {
        for (const auto& [mode, mode_state] : available_modes_) {
          const auto camera_mode_pair = std::make_pair(camera, mode);
          display_nodes_[camera_mode_pair] = std::make_shared<Node>();
          // Node for both preview and label
          auto preview_combined_ = std::make_shared<Node>();

          // Generate texture IDs for each camera
          glGenTextures(1, &texture_ids_[camera_mode_pair]);
          // Bind the texture IDs to a texture
          glBindTexture(GL_TEXTURE_2D, texture_ids_[camera_mode_pair]);
          // Set up the texture
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texture_width_, texture_height_, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

          // Set up texture to be rendered by app framework and add to preview_combined_
          // Textures are owned so are destroyed/cleared when Texture destructor is called
          auto tex = std::make_shared<Texture>(GL_TEXTURE_2D, texture_ids_[camera_mode_pair], texture_width_, texture_height_,true);
          auto quad = Registry::GetInstance()->GetResourcePool()->GetMesh<QuadMesh>();
          auto gui_mat = std::make_shared<TexturedGrayscaleMaterial>(tex);
          gui_mat->SetPolygonMode(GL_FILL);
          auto gui_renderable = std::make_shared<RenderableComponent>(quad, gui_mat);
          auto gui_node = std::make_shared<Node>();
          gui_node->AddComponent(gui_renderable);
          // SetLocalScale with - y axis due to order of pixel data and glTexImage2D orientation mismatch
          gui_node->SetLocalScale(glm::vec3{1.0f, -1.0f, 1.0f});
          preview_combined_->AddChild(gui_node);

          // Create label and add to preview_combined_
          auto text = ml::app_framework::CreatePresetNode(ml::app_framework::NodeType::Text);
          const std::string label = std::string(GetMLWorldCameraIdentifierString(camera)) + "\n" +
                              std::string(GetMLWorldCameraFrameTypeString(mode)) +
                              "\nFrame Number: ";
          text->GetComponent<ml::app_framework::TextComponent>()->SetText(label.c_str());
          text->SetLocalScale(glm::vec3{0.008f, -0.008f, 1.f});
          text->SetLocalTranslation(text_offsets_[camera_mode_pair]);

          preview_combined_->AddChild(text);

          // Add the preview and label to display_nodes_
          preview_combined_->SetLocalTranslation(preview_offsets_[camera_mode_pair]);
          preview_combined_->SetLocalScale(glm::vec3(0.5, 0.5, 0.5));
          display_nodes_[camera_mode_pair]->AddChild(preview_combined_);

          const auto head_pose_opt = GetHeadPoseOrigin();
          if (!head_pose_opt.has_value()) {
            ALOGW("No head pose available at application start! For best experience, start the application while wearing the ML2.");
          }
          const Pose head_pose = head_pose_opt.value_or(
                  GetRoot()->GetWorldPose()).HorizontalRotationOnly();
          display_nodes_[camera_mode_pair]->SetWorldPose(head_pose);

          GetRoot()->AddChild(display_nodes_[camera_mode_pair]);
          SetPreviewVisibility(display_nodes_[camera_mode_pair], mode_state && camera_state);
        }
      }
      preview_initialized_ = true;
    }

    std::unordered_map<MLWorldCameraIdentifier, bool> available_cameras_;
    std::unordered_map<MLWorldCameraFrameType, bool> available_modes_;
    std::map<CameraIdModePair, std::shared_ptr<Node>> display_nodes_;
    std::map<CameraIdModePair, int64_t> last_frame_num_;
    std::map<CameraIdModePair, MLWorldCameraFrame> last_frame_info_;
    std::map<CameraIdModePair, glm::vec3> preview_offsets_, text_offsets_;
    std::map<CameraIdModePair, int> dropped_frame_count;
    std::map<CameraIdModePair, GLuint> texture_ids_;
    bool preview_initialized_;
    int texture_width_, texture_height_;
    MLHandle world_camera_handle_;
    MLWorldCameraSettings world_camera_settings_;
};

void android_main(struct android_app *state) {
#ifndef ML_LUMIN
  ALOGE("This app is not supported on app simulator.");
#else
  WorldCameraApp app(state);
  app.RunApp();
#endif
}
