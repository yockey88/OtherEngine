local engine_stats = {}
      
local function fps(dt)
  if (dt == nil or dt == 0) then
    return 0
  end
  return (1 / dt) * 1000
end

local function render_stats(fps)
  ImGui.Begin("Stats")
  if (engine_stats.dt == nil or engine_stats.dt == 0) then
    ImGui.Text("FPS : N/A")
  else
    text = string.format("FPS : %.2f", fps(engine_stats.dt))
    ImGui.Text(text)
  end
  ImGui.End()
end

local function render_scene(scene_ctx)
  ImGui.Begin("Scene")

  scene_handle_str = string.format("Scene Handle : [%d:%x]" , scene_ctx, scene_ctx)
  ImGui.Text(scene_handle_str)

  entities = Scene.SceneEntities()
  if (entities == nil) then
    ImGui.Text("No entities in scene")
  else
    ImGui.Text(string.format("Entities In Scene : %d", #entities))
    
    for e=1,#entities do
      ent = entities[e]
      ImGui.Text(string.format("Entity [%d] : %s", ent.id, ent.name))
      
      transform = ent.transform
      ImGui.Text(string.format("Position : < %.2f, %.2f, %.2f >", transform.position.x, transform.position.y, transform.position.z))
    end

  end

  ImGui.End()
end

SandboxUI = {
  OnBehaviorLoad = function()
    engine_stats.dt = 0
    Logger.WriteDebug("Loading SandboxUI")
  end ,
  OnBehaviorUnload = function()
    Logger.WriteDebug("Unload SandboxUI")
  end ,

  Update = function(dt)
    if (engine_stats.dt ~= nil) then
      engine_stats.dt = dt
    end
  end ,

  RenderUI = function()
    render_stats(fps)

    scene_ctx = Scene.ContextHandle()
    if (scene_ctx ~= nil) then
      render_scene(scene_ctx)
    end
  end
}

--- goal is to implement all of this in lua
-- const ImVec2 win_size = {(float)Renderer::WindowSize().x, (float)Renderer::WindowSize().y};
-- if (ImGui::Begin("Frames")) {
--   if (!success) {
--     ScopedColor red(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
--     ImGui::Text("Failed to render frame");
--   } else {
--     ImGui::Text("Frames %llu", frames.size());
--     if (auto frame = frames.find(FNV("Debug")); frame != frames.end()) {
--       RenderItem(frame->second->texture, "Debug", ImVec2(win_size.x, win_size.y));
--     }
--   }
-- }
-- ImGui::End();

-- if (ImGui::Begin("Render Settings")) {
--   bool edited = false

--   ImGui::Text("===== Scene Controls =====");
--   auto& reg = scene->Registry();

--   ImGui::Text(" - Transforms =====");
--   reg.view<Tag, Transform>().each([&](Tag& tag, Transform& transform) {
--     ImGui::PushID((tag.name + "##transform-widget").c_str());
--     if (ui::widgets::DrawVec3Control(fmtstr("{} position", tag.name),
--                                      transform.position, edited, 0.f, 100.f, ui::VectorAxis::ZERO,
--                                      {-100.f, -100.f, -100.f}, {100.f, 100.f, 100.f}, 0.5f)) {}
--     ImGui::Separator();
--     ImGui::PopID();
--   });

--   ImGui::Text(" - Materials =====");

--   reg.view<Tag, StaticMesh>().each([&](Tag& tag, StaticMesh& mesh) {
--     ImGui::PushID((tag.name + "##static-mesh-widget").c_str());
--     RenderMaterial(fmtstr("{} material", tag.name), mesh.material);
--     ImGui::Separator();
--     ImGui::PopID();
--   });
--   ImGui::Separator();

--   ImGui::Text(" - Light Controls =====");
--   uint32_t i = 0;
--   edited = false;
--   reg.view<LightSource, Transform>().each([&](LightSource& light, Transform& transform) {
--     switch (light.type) {
--       case POINT_LIGHT_SRC:
--         edited = RenderPointLight(fmtstr("point light [{}]", i++), light.pointlight) && edited;
--         break;
--       case DIRECTION_LIGHT_SRC:
--         edited = RenderDirectionLight(fmtstr("direction light [{}]", i++), light.direction_light) && edited;
--         break;
--       default:
--         break;
--     }
--     ++i;
--   });

--   if (edited) {
--     scene->RebuildEnvironment();
--   }

--   ImGui::Separator();
-- }
-- ImGui::End();