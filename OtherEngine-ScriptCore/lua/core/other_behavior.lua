local Other = require("other")

Other.Behavior = {}
local Behavior = Other.Behavior

function Behavior:new(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

function Behavior:OnBehaviorLoad()end

function Behavior:OnBehaviorUnload()end

function Behavior:OnInitialize()end
function Behavior:OnShutdown()end

function Behavior:OnStart()end
function Behavior:OnStop()end

function Behavior:EarlyUpdate(dt)end
function Behavior:Update(dt)end
function Behavior:LateUpdate(dt)end

function Behavior:Render()end
function Behavior:RenderUI()end

return Behavior