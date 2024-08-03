local spdlog = {}

spdlog.name = "spdlog"
spdlog.kind = "StaticLib"
spdlog.language = "C++"

spdlog.files = function()
  files {
    "./src/**.cpp" ,
    "./include/**.h" ,
  }
end

spdlog.include_dirs = function()
  includedirs { "./include" }
end

spdlog.defines = function()
  defines { "SPDLOG_COMPILED_LIB" }
end

AddExternalProject(spdlog)
