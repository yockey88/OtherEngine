local zep = {}

zep.name = "zep"
zep.kind = "StaticLib"
zep.language = "C++"

zep.files = function()
  files { "include/**.h", "src/**.cpp" }
end

zep.include_dirs = function()
  includedirs { "../imgui/imgui" , "./include" , "./include/zep/mcommon/string" }
end

zep.defines = function()
  defines { "ZEP_FEATURE_CPP_FILE_SYSTEM" }
end

AddExternalProject(zep)
