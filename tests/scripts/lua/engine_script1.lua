local other = require("other")
local behavior = require("other.behavior")
local object = require("other.object")

TestScript = behavior:new() 

function TestScript:test()
  return "TestScript.test()"
end

function TestScript:Method()
  Logger.WriteDebug("TestScript.Method()")
end

TestScript2 = object:new()

function TestScript2:test()
  Logger.WriteDebug("TestScript2.test()\n")
  return "TestScript2.test()"
end

function TestScript2:OnBehaviorLoad()
  Logger.WriteDebug("TestScript2.OnBehaviorLoad()")
end

function TestScript2:OnBehaviorUnload()
  Logger.WriteDebug("TestScript2.OnBehaviorUnload()")
end