local imguizmo = {
  name = "imguizmo",
  kind = "StaticLib",
  language = "C++",
  cppdialect = "C++20",

  files = function()
    files {
      "./imguizmo/*.cpp",
      "./imguizmo/*.h",
    }
  end,

  include_dirs = function()
    includedirs {
      ".",
      "%{wks.location}/externals/imgui/imgui"
    }
  end,

  externalincludedirs = function()
    externalincludedirs {
      "../sdl2/SDL2",
    }
  end,

  components = {
    ["imgui"] = "%{wks.location}/externals/imgui"
  }
}


AddExternalProject(imguizmo)
