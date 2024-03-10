local glad = {}

glad.name = "glad"
glad.kind = "StaticLib"
glad.language = "C"

glad.files = function() 
  files { "./include/**.h", "./src/**.c" }
end

glad.include_dirs = function() 
  includedirs { "./include" }
end

AddExternalProject(glad)
