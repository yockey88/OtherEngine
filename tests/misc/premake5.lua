local physics = {
  name = "physics_tests",
  path = "./physics",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",
  architecture = "x86_64",
  cross_platform_main = true,

  files = function()
    files { "./physics/*.cpp" }
  end,

  include_dirs = function()
    includedirs { "./physics" }
  end,

  defines = function()
    defines { "OE_MODULE" }
  end,

  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src",
  }
}

AddProject(physics)
