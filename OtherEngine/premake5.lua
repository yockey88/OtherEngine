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

OtherEngine.windows_configuration = function()
  files {
    "./platform/windows/**.hpp",
    "./platform/windows/**.cpp",
  }
  includedirs {
    "./platform",
  }
  systemversion "latest"
end

OtherEngine.post_build_commands = function()
  filter { "system:windows" , "configurations:Debug" }
    postbuildcommands {
      '{COPY} "%{wks.location}/externals/sdl2/lib/Debug/SDL2d.dll" "%{cfg.targetdir}"',
    }

  filter { "system:windows" , "configurations:Release" }
    postbuildcommands {
      '{COPY} "%{wks.location}/externals/sdl2/lib/Release/SDL2.dll" "%{cfg.targetdir}"',
    }
end

AddProject(OtherEngine)
