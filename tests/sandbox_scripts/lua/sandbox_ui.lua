local other = require("other")
local behavior = require("other.behavior")

local SandboxUI = behavior:new()
function SandboxUI:OnBehaviorLoad()
  Logger.WriteDebug("SandboxUI.OnBehaviorLoad()")
end

function SandboxUI:OnBehaviorUnload()
  Logger.WriteDebug("SandboxUI.OnBehaviorUnload()")
end

function SandboxUI:OnUpdate(dt)
end

function SandboxUI:RenderUI()
  ImGui.Begin("SandboxUI")
  ImGui.Text("Hello from SandboxUI")
  ImGui.End()
end

--- goal is to implement all of this in lua
-- UI::BeginFrame();
-- const ImVec2 win_size = {(float)Renderer::WindowSize().x, (float)Renderer::WindowSize().y};
-- if (ImGui::Begin("Stats")) {
--   ImGui::Text("FPS : %.2f", fps(delta));
-- }
-- ImGui::End();

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
--   bool edited = false;
--   ui::Underline();

--   ImGui::Text("Debug Controls");
--   ImGui::Separator();
--   ui::widgets::DrawVec3Control("outline color", outline_color, edited, 0.f, 100.f, ui::VectorAxis::ZERO,
--                                {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, 0.1f);
--   ImGui::Separator();

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
-- UI::EndFrame();