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
  
  defines = function()
    defines {
      "OE_MODULE" ,
    }
  end,
  
  components = {
    ["OtherEngine-CsCore"] = ""
  }
}

AddModule(yockcraft_scripts)

-- local editor_scripts = {
--   name = "YockCraftEditorScripts",
--   kind = "SharedLib",
--   language = "C#",
--   dotnetframework = "net8.0",
  
--   files = function()
--     files {
--       "%{wks.location}/yockcraft/yockcraft/assets/editor/*.cs"
--     }
--   end,
  
--   defines = function()
--     defines {
--       "OE_MODULE" ,
--     }
--   end,
  
--   components = {
--     ["OtherEngine-CsCore"] = "",
--     ["YockCraftScripts"] = ""
--   }
-- }
-- AddModule(editor_scripts)