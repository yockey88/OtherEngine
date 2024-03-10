local nfd = {}

nfd.name = "nfd"
nfd.kind = "StaticLib"
nfd.language = "C++"
nfd.cppdialect = "C++20"

nfd.files = function() 
  files { "./src/nfd_common.c", "./src/nfd_common.h", "./src/nfd/nfd.h" , "./src/simple_exec.h" }
end

nfd.include_dirs = function() 
  includedirs { "." , "./src" }
end

nfd.windows_configuration = function() 
  systemversion "latest"
  files { "./src/nfd_win.cpp" }
end

nfd.linux_configuration = function() 
  systemversion "latest"
  files { "./src/nfd_gtk.c" }
end

AddExternalProject(nfd)