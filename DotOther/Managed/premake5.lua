local managed = {
  name = "DotOther.Managed",
  path = "./Managed",
  kind = "SharedLib",
  language = "C#",
  dotnetframework = "net8.0",
  
  files = function()
    files {
      "./Source/**.cs"
    }
  end,

  defines = function()
    defines {
      "OE_MODULE" ,
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
}

AddModule(managed)