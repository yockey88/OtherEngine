local yockcraft = {}

yockcraft.name = "yockcraft"
yockcraft.path = "./yockcraft"
yockcraft.kind = "ConsoleApp"
yockcraft.language = "C++"
yockcraft.cppdialect = "C++latest"

yockcraft.files = function()
  files {
    "./src/**.cpp" ,
    "./src/**.hpp"
  }
end

yockcraft.include_dirs = function()
  includedirs {
    "./src"
  }
end

yockcraft.links = function()
  libdirs {
    "./engine/Debug" ,
    "./engine/Release"
  }
end

yockcraft.defines = function()
  defines {
    "OE_MODULE"
  }
end

yockcraft.components = {}
yockcraft.components["C:/Yock/code/OtherEngine/bin/Debug/OtherEngine/OtherEngine"] = 
  "C:/Yock/code/OtherEngine/OtherEngine/src"

yockcraft.window_configuration = function()
  includedirs {
    "./platform",
  }
  systemversion "latest"
  buildoptions { "/EHsc" , "/Zc:preprocessor" , "/Zc:__cplusplus" }
end

yockcraft.window_debug_configuration = function()
  links {
    "DbgHelp",
  }
  defines {
    "TRACY_ENABLE" ,
    "TRACY_ON_DEMAND" ,
  }

  systemversion "latest"
  buildoptions { "/EHsc" , "/Zc:preprocessor" , "/Zc:__cplusplus" }
end

yockcraft.post_build_commands = function()
  filter { "system:windows" , "configurations:Debug" }
    postbuildcommands {
      '{COPY} "C:/Yock/code/OtherEngine/externals/sdl2/lib/Debug/SDL2d.dll" "%{cfg.targetdir}"',
      '{COPY} C:/Yock/code/OtherEngine/externals/mono/bin/mono-2.0-sgen.dll "%{cfg.targetdir}"',
      '{COPY} C:/Yock/code/OtherEngine/externals/mono/bin/mono-2.0-sgen.pdb "%{cfg.targetdir}"',
      '{COPY} C:/Yock/code/OtherEngine/externals/mono/bin/MonoPosixHelper.dll "%{cfg.targetdir}"',
      '{COPY} C:/Yock/code/OtherEngine/externals/mono/bin/MonoPosixHelper.pdb "%{cfg.targetdir}"',
    }

  filter { "system:windows" , "configurations:Release" }
    postbuildcommands {
      '{COPY} "C:/Yock/code/OtherEngine/externals/sdl2/lib/Release/SDL2.dll" "%{cfg.targetdir}"',
      '{COPY} C:/Yock/code/OtherEngine/externals/mono/bin/mono-2.0-sgen.dll "%{cfg.targetdir}"',
      '{COPY} C:/Yock/code/OtherEngine/externals/mono/bin/MonoPosixHelper.dll "%{cfg.targetdir}"',
    }
end

AddProject(yockcraft)

local yockcraft_scripts = {}

yockcraft_scripts.name = "YockCraftScripts"
yockcraft_scripts.kind = "SharedLib"
yockcraft_scripts.language = "C#"

yockcraft_scripts.files = function()
  files {
    "./assets/scripts/*.cs"
  }
end

yockcraft_scripts.components = {}
yockcraft_scripts.components["C:/Yock/code/OtherEngine/bin/Debug/OtherEngine-CsCore/OtherEngine-CsCore"] = ""

local editor_scripts = {}

editor_scripts.name = "YockCraftEditorScripts"
editor_scripts.kind = "SharedLib"
editor_scripts.language = "C#"

editor_scripts.files = function()
  files {
    "./assets/editor/*.cs"
  }
end

editor_scripts.components = {}
editor_scripts.components["C:/Yock/code/OtherEngine/bin/Debug/OtherEngine-CsCore/OtherEngine-CsCore"] = ""
editor_scripts.components["YockCraftScripts"] = ""

AddModule(yockcraft_scripts)
AddModule(editor_scripts)
