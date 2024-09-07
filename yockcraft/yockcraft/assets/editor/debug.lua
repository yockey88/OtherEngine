local scene_running = true

--- @function main debug ui window
local function RenderControlWindow(args)
  local open , can_draw = ImGui.Begin("Control Center")
  if not can_draw then
    ImGui.End()
    return
  end

  if ImGui.Button("Pause") then
    scene_running = not scene_running
  end

  ImGui.End()
end

--- @function render scene controls
local function RenderSceneContols(args)
  -- local open , can_draw = ImGui.Begin("Scene Controls")
  -- if not can_draw then
  --   ImGui.End()
  --   return
  -- end


  -- ImGui.End()
end

--- @class Debug : Object 
Debug = {
  OnInitialize = function()
  end,

  RenderUI = function()
    RenderControlWindow({})
    RenderSceneContols({})
  end,
} 