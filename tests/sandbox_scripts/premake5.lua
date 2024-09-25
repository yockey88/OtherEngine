local sandbox_scripts = {
  name = "SandboxScripts" ,
  path = "./sandbox_scripts" ,
  kind = "SharedLib" ,
  language = "C#" ,
  dotnetframework = "net8.0" ,

  files = function()
    files {
      "./sandbox/**.cs" ,
    }
  end,

  components = {
    ["OtherEngine-CsCore"] = "%{wks.location}/OtherEngine-CsCore/cs"
  }
}

AddModule(sandbox_scripts)