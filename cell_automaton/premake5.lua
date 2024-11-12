local cell_automaton = {
  name = "cell_automaton",
  path = "./cell_automaton",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",
  architecture = "x86_64",

  files = function()
    files {
      "./cell_automaton/src/**.cpp",
      "./cell_automaton/src/**.hpp",
    }
  end,

  include_dirs = function()
    includedirs { "./cell_automaton" }
    externalincludedirs { "%{wks.location}/DotOther/NetCore" }
  end,

  defines = function()
    defines { "OE_MODULE" }
  end,

  windows_configuration = function()
    entrypoint "WinMainCRTStartup"
  end,

  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src"
  }
}

AddProject(cell_automaton)

include "cell_automaton_modules/premake5.lua"
