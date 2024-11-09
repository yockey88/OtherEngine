/**
 * \file rendering_layer.hpp
 **/
#ifndef OTHER_ENGINE_RENDERING_LAYER_HPP
#define OTHER_ENGINE_RENDERING_LAYER_HPP

#include "core/defines.hpp"
#include "core/layer.hpp"

#include "event/key_events.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/scene_renderer.hpp"
#include "rendering/shader.hpp"
#include "rendering/uniform.hpp"
#include "rendering/vertex.hpp"

using namespace other;

class RenderingLayer : public Layer {
 public:
  RenderingLayer(App* parent_app, const std::string& name)
      : Layer(parent_app, name) {}

  Ref<Shader> fbshader = nullptr;
  Ref<VertexArray> fb_mesh = nullptr;
  Ref<CameraBase> camera = nullptr;
  Ref<Framebuffer> framebuffer = nullptr;

  Ref<UniformBuffer> camera_uniforms = nullptr;
  Ref<UniformBuffer> light_uniforms = nullptr;

  Ref<RenderPass> normal_pass = nullptr;
  Ref<RenderPass> pure_geom_pass = nullptr;
  Ref<RenderPass> geom_pass = nullptr;
  Ref<RenderPass> outline_pass = nullptr;

  Ref<ScriptObject> sandbox_ui = nullptr;

 protected:
  virtual void OnAttach() override;
  virtual void OnDetach() override;

  // virtual void OnEarlyUpdate(float dt) {}
  // virtual void OnUpdate(float dt) {}
  virtual void OnLateUpdate(float dt) override;
  virtual void OnRender() override;
  virtual void OnUIRender() override;

  // virtual void OnSceneLoad(const SceneMetadata* metadata) {}
  // virtual void OnSceneUnload() {}

  // virtual void OnScriptReload() {}

  bool HandleKeyPressed(KeyPressed& event);

  bool camera_lock = true;

  Path shader_dir;
  Path default_path;
  Path normals_path;
  Path fbshader_path;
  Path deferred_shader_path;
  Path add_fog_shader_path;
  Path red_path;
  Path outline_path;
  Path pure_geometry_path;
  Path texture_dir;
  Path editor_texture_dir;
};

#endif  // !OTHER_ENGINE_RENDERING_LAYER_HPP