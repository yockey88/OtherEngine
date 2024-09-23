local yockcraft = {}

yockcraft.name = "yockcraft"
yockcraft.path = "./yockcraft"
yockcraft.kind = "ConsoleApp"
yockcraft.language = "C++"
yockcraft.cppdialect = "C++latest"

yockcraft.files = function()
  files {
    "./yockcraft/src/**.cpp" ,
    "./yockcraft/src/**.hpp" ,
  }
end

yockcraft.include_dirs = function()
  includedirs {
    "./yockcraft/src",
  }
end

yockcraft.links = function()
  libdirs {
    "./engine/%{cfg.buildcfg}" ,
  }
end

yockcraft.defines = function()
  defines {
    "OE_MODULE"
  }
end

yockcraft.components = {}
yockcraft.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"

yockcraft.window_configuration = function()
  includedirs {
    "./platform",
  }
  systemversion "latest"
  buildoptions { 
    "/EHsc" , 
    "/Zc:preprocessor" , 
    "/Zc:__cplusplus"
  }
end

yockcraft.window_debug_configuration = function()
  links {
    "DbgHelp",
  }
  defines {
    "TRACY_ENABLE" ,
    "TRACY_ON_DEMAND" ,
  }
end

AddProject(yockcraft)

local yockcraft_scripts = {}

yockcraft_scripts.name = "YockCraftScripts"
yockcraft_scripts.kind = "SharedLib"
yockcraft_scripts.language = "C#"

yockcraft_scripts.files = function()
  files {
    "%{wks.location}/yockcraft/yockcraft/assets/scripts/*.cs"
  }
end

yockcraft_scripts.defines = function()
  defines {
    "OE_MODULE" ,
  }
end

yockcraft_scripts.components = {}
yockcraft_scripts.components["OtherEngine-CsCore"] = ""

local editor_scripts = {}

editor_scripts.name = "YockCraftEditorScripts"
editor_scripts.kind = "SharedLib"
editor_scripts.language = "C#"

editor_scripts.files = function()
  files {
    "%{wks.location}/yockcraft/yockcraft/assets/editor/*.cs"
  }
end

editor_scripts.defines = function()
  defines {
    "OE_MODULE" ,
  }
end

editor_scripts.components = {}
editor_scripts.components["OtherEngine-CsCore"] = ""
editor_scripts.components["YockCraftScripts"] = ""

AddModule(yockcraft_scripts)
AddModule(editor_scripts)