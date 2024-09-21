local test_module1 = {
  name = "Mod1",
  path = "./DotOther/Tests/Managed",
  kind = "SharedLib",
  language = "C#",
  dotnetframework = "net8.0",
  tdir = "%{wks.location}/bin/%{cfg.buildcfg}/DotOther.Tests",

  files = function()
    files {
      "./Source/**.cs" ,
    }
  end,

  windows_configuration = function()
    propertytags {
      { "AppendFrameworkToOutputPath", "false" } ,
      { "Nullable", "enable" } ,
    }

    disablewarnings {
      "CS8500" ,
    }
  end,

  components = {
    ["DotOther.Managed"] = "",
  }
}

AddModule(test_module1)