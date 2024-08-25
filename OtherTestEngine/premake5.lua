local OtherTestEngine = {}

OtherTestEngine.name = "OtherTestEngine"
OtherTestEngine.path = "./OtherTestEngine"
OtherTestEngine.kind = "ConsoleApp"
OtherTestEngine.language = "C++"
OtherTestEngine.cppdialect = "C++latest"

OtherTestEngine.files = function()
  files {
    "./src/**.cpp",
    "./src/**.hpp",
    "./tests/**.cpp" ,
    "./tests/**.hpp" ,
  }
end

OtherTestEngine.include_dirs = function()
  includedirs {
    "./src",
    "./tests" ,
  }
end

OtherTestEngine.defines = function()
  defines {
    "TRACY_ENABLE" ,
    "TRACY_ON_DEMAND" ,
    "TRACY_CALLSTACK=10"
  }
end

OtherTestEngine.windows_configuration = function()
  files {
    "./platform/windows/**.hpp",
    "./platform/windows/**.cpp",
  }
  includedirs {
    "./platform",
  }
  systemversion "latest"
  buildoptions { "/EHsc" , "/Zc:preprocessor" , "/Zc:__cplusplus" }
end

OtherTestEngine.components = {}
OtherTestEngine.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
OtherTestEngine.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"
OtherTestEngine.components[""] = "%{wks.location}/externals/gtest/googlemock/include"

OtherTestEngine.post_build_commands = function()
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

AddProject(OtherTestEngine)
