/**
 * \file scene_layer.cpp
 **/
#ifndef OTHER_ENGINE_SCENE_LAYER_HPP
#define OTHER_ENGINE_SCENE_LAYER_HPP

#include "core/layer.hpp"

#include "application/app.hpp"
#include "event/scene_events.hpp"

#include "scene/bvh.hpp"
#include "scene/scene.hpp"

using namespace other;

class SceneLayer : public Layer {
 public:
  SceneLayer(App* parent_app, const std::string& name)
      : Layer(parent_app, name) {}

  Ref<Scene> scene = nullptr;
  Ref<BvhTree> bvh = nullptr;

 protected:
  virtual void OnAttach() override;
  virtual void OnDetach() override;
  // virtual void OnEarlyUpdate(float dt) {}
  // virtual void OnUpdate(float dt) {}
  virtual void OnLateUpdate(float dt) override;
  // virtual void OnUIRender() {}

 private:
  bool HandleSceneLoad(SceneLoad& event);

  Material cube_material1 = Material({ 1.0f, 0.5f, 0.31f, 1.f }, 32.f);
  Material cube_material2 = Material({ 0.1f, 0.5f, 0.31f, 1.f }, 32.f);
  Path scenepath;
};

#endif  // !OTHER_ENGINE_SCENE_LAYER_HPP