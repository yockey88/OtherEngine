local OE_Launcher = {}

OE_Launcher.name = "OtherEngine-Launcher"
OE_Launcher.path = "./OtherEngine-Launcher"
OE_Launcher.kind = "ConsoleApp"
OE_Launcher.language = "C++"
OE_Launcher.cppdialect = "C++latest"

OE_Launcher.files = function()
  files {
    "./src/**.cpp",
    "./src/**.hpp",
  }
end

OE_Launcher.include_dirs = function()
  includedirs {
    "./src",
  }
end

OE_Launcher.components = {}
OE_Launcher.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"

OE_Launcher.post_build_commands = function()
  filter { "system:windows" , "configurations:Debug" }
    postbuildcommands {
      '{COPY} "%{wks.location}/externals/sdl2/lib/Debug/SDL2d.dll" "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/mono-2.0-sgen.dll "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/mono-2.0-sgen.pdb "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/MonoPosixHelper.dll "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/MonoPosixHelper.pdb "%{cfg.targetdir}"',
    }

  filter { "system:windows" , "configurations:Release" }
    postbuildcommands {
      '{COPY} "%{wks.location}/externals/sdl2/lib/Release/SDL2.dll" "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/mono-2.0-sgen.dll "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/externals/mono/bin/MonoPosixHelper.dll "%{cfg.targetdir}"',
    }
end

AddProject(OE_Launcher)
