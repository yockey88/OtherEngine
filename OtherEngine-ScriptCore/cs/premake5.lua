local ScriptCore = {
  name = "OtherEngine-CsCore",
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

  links = function()
    links {
      "DotOther.Managed"
    }
  end,
}


AddModule(ScriptCore)