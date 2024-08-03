local jolt = {}

jolt.name = "jolt"
jolt.kind = "StaticLib"
jolt.language = "C++"
jolt.cppdialect = "C++20"

jolt.files = function()
  files {
    "./Jolt/**.cpp" ,
    "./Jolt/**.h"
  }
end

jolt.include_dirs = function()
  includedirs {
    "."
  }
end

AddExternalProject(jolt)
