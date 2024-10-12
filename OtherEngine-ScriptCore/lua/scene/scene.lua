local Behavior = require("other.behavior")
Scene = Behavior:new()

function Scene:new(o)
  o = o or {}
  setmetatable(o, self)
  self.__index = self
  return o
end

return Scene