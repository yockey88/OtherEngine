local yockcraft_scripts = {
  name = "YockCraftScripts",
  kind = "SharedLib",
  language = "C#",
  dotnetframework = "net8.0",
  
  files = function()
    files {
      "./Source/*.cs"
    }
  end,
  
  components = {
    ["OtherEngine-CsCore"] = ""
  }
}

AddModule(yockcraft_scripts)