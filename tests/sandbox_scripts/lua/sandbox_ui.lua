-- local other_ui = require("other.ui")

local engine_stats = {}
      
local function fps(dt)
  if (dt == nil or dt == 0) 
  then
    return 0
  end
  return (1 / dt) * 1000
end

local function render_stats(fps)
  ImGui.Begin("Stats")
  if (engine_stats.dt == nil or engine_stats.dt == 0) 
  then
    ImGui.Text("FPS : N/A")
  else
    text = string.format("FPS : %.2f", fps(engine_stats.dt))
    ImGui.Text(text)
  end
  ImGui.End()
end

local function add_vertical_gap(size)
  ImGui.SetCursorPosY(ImGui.GetCursorPosY() + size)
end

local function indent_then_do(indent, func)
  ImGui.SetCursorPosX(ImGui.GetCursorPosX() + indent)
  func()
end

local function render_text_with_color(text, color)
  ImGui.PushStyleColor(ImGuiCol_Text, color)
  ImGui.Text(text)
  ImGui.PopStyleColor()
end

local color_min = Vec3:new(0, 0, 0)
local color_max = Vec3:new(1, 1, 1)
local function render_entity_mesh(mesh)
  ImGui.PushId(mesh.mesh)

  local function indent_then_do(indent, func)
    ImGui.SetCursorPosX(ImGui.GetCursorPosX() + indent)
    func()
  end


  -- local col_vec3 = Vec3:new(mesh.material.color.x, mesh.material.color.y, mesh.material.color.z)
  -- local shininess = mesh.material.shininess

  indent_then_do(20.0, function() 
    ImGui.Text(string.format("Mesh [%d]", mesh.mesh))
  end)

  indent_then_do(20.0, function() 
    ImGui.Text("Material")
  end)
  
  indent_then_do(20.0, function() 
    local color = {mesh.material.color.x, mesh.material.color.y, mesh.material.color.z, mesh.material.color.w}
    local col, used = ImGui.ColorEdit4("color", color)
    if (used) 
    then
      mesh.material.color = Vec4:new(col[1], col[2], col[3], col[4])
    end
  end)

  -- local alpha, used = ImGui.SliderFloat("alpha", mesh.material.color.w, 0.01 , 0.0, 1.0)
  -- if (used)
  -- then
  --   mesh.material.color.w = alpha
  -- end

  indent_then_do(20.0, function()
    local shininess, used = ImGui.DragFloat("shininess", mesh.material.shininess, 1.0 , 0.0, 100.0)
    if (used)
    then
      mesh.material.shininess = shininess
    end
  end)


  ImGui.PopId()
end


local vec3_min = Vec3:new(-200 , -200 , -200)
local vec3_max = Vec3:new(200 , 200 , 200)
local function render_entity(ent)
  ImGui.PushId(ImGui.GetId(ent.name))

  ImGui.PushStyleColor(ImGuiCol.Text, 0.0 , 0.7 , 1.0 , 1.0)
  ImGui.Text(string.format("Entity [%d] : %s", ent.id, ent.name))
  ImGui.PopStyleColor()
  
  ImGui.Separator()
  OtherUI.DrawVec3("Position", ent.transform.position, 0.0, 100.0, VectorAxis.ZERO , vec3_min, vec3_max, 0.1)
  ImGui.SetCursorPosY(ImGui.GetCursorPosY() + 20.0)

  if (ent:HasComponent("mesh"))
  then
    -- render_entity_mesh(ent.mesh)
  end
    
  if (ent:HasComponent("static-mesh"))
  then
    render_entity_mesh(ent.static_mesh)
  end

  ImGui.PopId()

  add_vertical_gap(20.0)
end

local function render_scene(scene_ctx)
  ImGui.Begin("Scene")

  scene_handle_str = string.format("Scene Handle : [%d:%x]" , scene_ctx, scene_ctx)
  ImGui.Text(scene_handle_str)

  local entities = Scene.SceneEntities()
  if (entities == nil) 
  then
    ImGui.Text("No entities in scene")
  else
    ImGui.Text(string.format("Entities In Scene : %d", #entities))
    add_vertical_gap(20.0)
    
    local vec3_zero = Vec3:new(0, 0, 0)
    for e=1,#entities 
    do
      ent = entities[e]
      render_entity(ent)
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
    if (engine_stats.dt ~= nil) 
    then
      engine_stats.dt = dt
    end
  end ,

  RenderUI = function()
    render_stats(fps)

    scene_ctx = Scene.ContextHandle()
    if (scene_ctx ~= nil) 
    then
      render_scene(scene_ctx)
    end

    -- ImGui.Begin("Other UI")
    
    -- pressed = ImGui.Button("Press Me")
    -- if (pressed) then
    --   Logger.WriteDebug("Button Pressed")
    -- end

    -- ImGui.End()
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