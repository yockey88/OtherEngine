local Behavior = require("other.behavior");

Behavior.Object = Behavior:new()
local Object = Behavior.Object

function Object:new()
  local object = {}
  setmetatable(object, self)
  self.__index = self
  return object
end

return Object