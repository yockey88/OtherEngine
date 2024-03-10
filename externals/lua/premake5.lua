local lua = {}

lua.name = "lua"
lua.kind = "StaticLib"
lua.language = "C++"
lua.cppdialect = "C++20"

lua.files = function() 
  files { "./lua/**.c" , "./lua/**.h" }
end

lua.include_dirs = function() 
  includedirs { "lua" }
end

AddExternalProject(lua)
