local tracy  = {}

tracy.name = "tracy"
tracy.kind = "StaticLib"
tracy.language = "C++"
tracy.cppdialect = "C++20"

tracy.files = function()
  files {
    "./tracy/client/**.h" ,
    "./tracy/client/**.hpp" ,
    "./tracy/client/**.cpp" ,

    "./tracy/common/**.h" ,
    "./tracy/common/**.hpp" ,
    "./tracy/common/**.cpp" ,

    "./tracy/tracy/**.h" ,
    "./tracy/tracy/**.hpp" ,
    "./tracy/tracy/**.cpp" ,

    "./tracy/libbacktrace/alloc.cpp" ,
    "./tracy/libbacktrace/sort.cpp" ,
    "./tracy/libbacktrace/state.cpp" ,
  }
end

tracy.include_dirs = function()
  includedirs {
    "./tracy/public" ,
  }
end

tracy.defines = function()
  defines {
    "TRACY_HAS_CALLSTACK" ,
    "TRACY_ENABLE" ,
    "TRACY_ON_DEMAND"
  }
end

tracy.windows_configuration = function()
  systemversion "latest"
  links { "DbgHelp" }
end

AddExternalProject(tracy)
