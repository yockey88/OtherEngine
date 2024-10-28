TestScript = {}
function TestScript:test()
  return "TestScript.test()"
end

TestScript2 = {}
function TestScript2:test()
  Logger.WriteDebug("TestScript2.test()\n")
  return "TestScript2.test()"
end

