local sandbox_scripts = {
  name = "SandboxScripts" ,
  path = "./sandbox_scripts" ,
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

AddModule(sandbox_scripts)