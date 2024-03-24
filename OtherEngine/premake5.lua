local OtherEngine = {}

OtherEngine.name = "OtherEngine"
OtherEngine.path = "./OtherEngine"
OtherEngine.kind = "StaticLib"
OtherEngine.language = "C++"
OtherEngine.cppdialect = "C++latest"

OtherEngine.files = function()
  files {
    "./src/**.cpp",
    "./src/**.hpp",
  }
end

OtherEngine.include_dirs = function()
  includedirs {
    "./src",
  }
end

OtherEngine.components = {}
OtherEngine.components["imgui"] = "%{wks.location}/externals/imgui/imgui"

OtherEngine.post_build_commands = function()
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

AddProject(OtherEngine)
