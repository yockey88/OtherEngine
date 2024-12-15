/**
 * \file editor/panels/shader_creator.cpp
 **/
#include "editor/panels/shader_creator.hpp"

#include <imgui/imgui.h>

#include "core/directory.hpp"
#include "core/filesystem.hpp"

#include "application/app_state.hpp"

#include "rendering/shader.hpp"
#include "rendering/ui/ui_helpers.hpp"

namespace other {

  void ShaderCreator::OnAttach() {
    Ref<Directory> project_dir = Filesystem::GetDirectory("project-root");

    auto vert_files = project_dir->GetFiles(".vert");
    for (auto& file : vert_files) {
      OE_ASSERT(file != nullptr, "Failed to load vertex shader file");
      vertex_files[file->handle.Get()] = file;
    }

    auto frag_files = project_dir->GetFiles(".frag");
    for (auto& file : frag_files) {
      OE_ASSERT(file != nullptr, "Failed to load fragment shader file");
      fragment_files[file->handle.Get()] = file;
    }

    auto geom_files = project_dir->GetFiles(".geom");
    for (auto& file : geom_files) {
      OE_ASSERT(file != nullptr, "Failed to load geometry shader file");
      geometry_files[file->handle.Get()] = file;
    }

    auto other_files = project_dir->GetFiles(".oshader");
    for (auto& file : other_files) {
      OE_ASSERT(file != nullptr, "Failed to load other shader file");
      other_shader_files[file->handle.Get()] = file;
    }
  }

  void ShaderCreator::OnDetach() {
    vertex_files.clear();
    fragment_files.clear();
    geometry_files.clear();
    other_shader_files.clear();
  }

  bool ShaderCreator::OnGuiRender(bool& is_open) {
    if (!ImGui::Begin("Shader Creator", &is_open)) {
      ImGui::End();
    }

    /// table of shader files
    ImGui::Columns(4, "Shader Files", true);
    ImGui::Separator();
    if (!vertex_selection.has_value() && !other_shader_selection.has_value()) {
      auto vs_selection = RenderColumn("Vertex Shaders", "vs", vertex_files);
      if (vs_selection != nullptr) {
        vertex_selection = vs_selection;
      } else {
        vertex_selection = std::nullopt;
      }
    }
    ImGui::NextColumn();
    if (!fragment_selection.has_value() && !other_shader_selection.has_value()) {
      auto fs_selection = RenderColumn("Fragment Shaders", "fs", fragment_files);
      if (fs_selection != nullptr) {
        fragment_selection = fs_selection;
      } else {
        fragment_selection = std::nullopt;
      }
    }

    ImGui::NextColumn();
    if (!geometry_selection.has_value() && !other_shader_selection.has_value()) {
      auto gs_selection = RenderColumn("Geometry Shaders", "gs", geometry_files);
      if (gs_selection != nullptr) {
        geometry_selection = gs_selection;
      } else {
        geometry_selection = std::nullopt;
      }
    }

    ImGui::NextColumn();
    if (!vertex_selection.has_value() && !fragment_selection.has_value() && !geometry_selection.has_value()) {
      bool already_set = other_shader_selection.has_value();

      auto os_selection = RenderColumn("Other Shaders", "os", other_shader_files);
      if (os_selection != nullptr) {
        other_shader_selection = os_selection;
      } else {
        other_shader_selection = std::nullopt;
      }

      if (!already_set && other_shader_selection.has_value()) {
        // shader = BuildShader(other_shader_selection.value()->AbsolutePath());
      }
    }

    ImGui::Columns(1);
    ImGui::Separator();

    bool shader_good = shader.has_value() && !(*shader == nullptr);
    {
      ScopedColor green_text(ImGuiCol_Text, shader_good ? ui::theme::green : ui::theme::red);

      if (other_shader_selection.has_value()) {
        ImGui::Text("Other Shader: %s", (*other_shader_selection)->FileName().c_str());
      } else {
        if (vertex_selection.has_value()) {
          ImGui::Text("Vertex Shader: %s", (*vertex_selection)->FileName().c_str());
        }

        if (geometry_selection.has_value()) {
          ImGui::Text("Geometry Shader: %s", (*geometry_selection)->FileName().c_str());
        }

        if (fragment_selection.has_value()) {
          ImGui::Text("Fragment Shader: %s", (*fragment_selection)->FileName().c_str());
        }
      }
    }

    if (shader_good) {
      ui::Button("Finalize Shader", [&]() {
        // AppState::Assets()->AddAsset<Shader>(shader.value());
        // is_open = false;
      });
    }

    ImGui::End();
    return false;
  }

  Ref<FileHandle> ShaderCreator::RenderColumn(const std::string_view title, const std::string_view prefix, std::map<UUID, Ref<FileHandle>>& files) {
    ImGui::Text("%s", title.data());
    for (auto& [id, file] : files) {
      ImGui::PushID(id.Get());
      if (ui::Selectable(file->FileName().c_str())) {
        if ((vertex_selection.has_value() && file == *vertex_selection) ||
            (fragment_selection.has_value() && file == *fragment_selection) ||
            (geometry_selection.has_value() && file == *geometry_selection) ||
            (other_shader_selection.has_value() && file == *other_shader_selection)) {
          ImGui::PopID();
          return nullptr;
        }

        ImGui::PopID();
        return file;
      }
      ImGui::PopID();
    }

    if (vertex_selection.has_value()) {
      return *vertex_selection;
    } else if (fragment_selection.has_value()) {
      return *fragment_selection;
    } else if (geometry_selection.has_value()) {
      return *geometry_selection;
    } else if (other_shader_selection.has_value()) {
      return *other_shader_selection;
    }

    return nullptr;
  }

}  // namespace other
