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

AddProject(sandbox)