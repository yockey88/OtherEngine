local test_scripts = {
  name = "TestScripts" ,
  path = "./tests/scripts" ,
  kind = "SharedLib" ,
  language = "C#" ,
  dotnetframework = "net8.0" ,

  files = function()
    files {
      "./Source/**.cs" ,
    }
  end,

  components = {
    ["OtherEngine-CsCore"] = ""
  }
}

AddModule(test_scripts)