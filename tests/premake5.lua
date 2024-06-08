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
    "./scripting_tests/**.cpp",
    "./scripting_tests/**.hpp",
  }
end

scripting_tests.include_dirs = function()
  includedirs {
    "./scripting_tests",
  }
end

scripting_tests.components = {}
scripting_tests.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
scripting_tests.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"

AddProject(sandbox)
