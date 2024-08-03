local sandbox = {}

sandbox.name = "sandbox"
sandbox.path = "./sandbox"
sandbox.kind = "ConsoleApp"
sandbox.language = "C++"
sandbox.cppdialect = "C++latest"

sandbox.files = function()
  files {
    "./sandbox/**.cpp",
    "./sandbox/**.hpp",
  }
end

sandbox.include_dirs = function()
  includedirs {
    "./sandbox",
  }
end

sandbox.components = {}
sandbox.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
sandbox.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"


sandbox.post_build_commands = function()
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

AddProject(sandbox)

local gl_sandbox = {}

gl_sandbox.name = "gl_sandbox"
gl_sandbox.path = "./gl_sandbox"
gl_sandbox.kind = "ConsoleApp"
gl_sandbox.language = "C++"
gl_sandbox.cppdialect = "C++latest"

gl_sandbox.files = function()
  files {
    "./gl_sandbox/**.cpp",
    "./gl_sandbox/**.hpp",
  }
end

gl_sandbox.include_dirs = function()
  includedirs {
    "./gl_sandbox",
  }
end

gl_sandbox.components = {}
gl_sandbox.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
gl_sandbox.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"

AddProject(gl_sandbox)

local OctreeTests = {}

OctreeTests.name = "OctreeTests"
OctreeTests.path = "./octree-tests"
OctreeTests.kind = "ConsoleApp"
OctreeTests.language = "C++"
OctreeTests.cppdialect = "C++latest"

OctreeTests.files = function()
  files {
    "./octree-tests/**.cpp",
    "./octree-tests/**.hpp",
  }
end

OctreeTests.include_dirs = function()
  includedirs {
    "./octree-tests",
  }
end

OctreeTests.components = {}
OctreeTests.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
OctreeTests.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"

AddProject(OctreeTests)

local sandbox = {}

sandbox.name = "sandbox"
sandbox.path = "./sandbox"
sandbox.kind = "ConsoleApp"
sandbox.language = "C++"
sandbox.cppdialect = "C++latest"

sandbox.files = function()
  files {
    "./sandbox/**.cpp",
    "./sandbox/**.hpp",
  }
end

sandbox.include_dirs = function()
  includedirs {
    "./sandbox",
  }
end

sandbox.components = {}
sandbox.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
sandbox.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"

local scripting_tests = {}

scripting_tests.name = "scripting_tests"
scripting_tests.path = "./scripting_tests"
scripting_tests.kind = "ConsoleApp"
scripting_tests.language = "C++"
scripting_tests.cppdialect = "C++latest"

scripting_tests.files = function()
  files {
    "./scripting-tests/main.cpp"
  }
end

scripting_tests.include_dirs = function()
  includedirs {
    "./scripting_tests",
  }
end

scripting_tests.post_build_commands = function()
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

scripting_tests.components = {}
scripting_tests.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
scripting_tests.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"

AddProject(scripting_tests)

local test_asm = {}

test_asm.name = "test_asm"
test_asm.kind = "SharedLib"
test_asm.language = "C#"

test_asm.files = function()
  files {
    "./scripting-tests/test_script_cs.cs" ,
  }
end

test_asm.defines = function()
  defines {
    "OE_MODULE" ,
  }
end

AddModule(test_asm)

local reflection_testing = {}

reflection_testing.name = "reflection_tests"
reflection_testing.path = "./reflection_tests"
reflection_testing.kind = "ConsoleApp"
reflection_testing.language = "C++"
reflection_testing.cppdialect = "C++latest"

reflection_testing.files = function()
  files {
    "./reflection_tests/main.cpp"
  }
end

reflection_testing.include_dirs = function()
  includedirs {
    "./reflection_tests",
  }
end

reflection_testing.post_build_commands = function()
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

reflection_testing.components = {}
reflection_testing.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
reflection_testing.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"

AddProject(reflection_testing)

local physics_tests = {}

physics_tests.name = "physics_tests"
physics_tests.path = "./physics_tests"
physics_tests.kind = "ConsoleApp"
physics_tests.language = "C++"
physics_tests.cppdialect = "C++latest"

physics_tests.files = function()
  files {
    "./physics_tests/main.cpp"
  }
end

physics_tests.include_dirs = function()
  includedirs {
    "./physics_tests",
  }
end

physics_tests.post_build_commands = function()
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

physics_tests.components = {}
physics_tests.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
physics_tests.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"

AddProject(physics_tests)
