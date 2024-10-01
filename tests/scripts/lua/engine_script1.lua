Other = {

  TestScript = {
    test = function()
      return "TestScript.test()"
    end,

    Method = function()
      Logger.WriteDebug("TestScript.Method()")
    end
  } ,

}

TestScript2 = {
  test = function()
    io.write("TestScript2.test()\n")
    return "TestScript2.test()"
  end
}