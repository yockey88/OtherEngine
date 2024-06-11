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
