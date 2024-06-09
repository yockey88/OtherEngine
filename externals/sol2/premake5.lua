local sol2 = {}
sol2.name = "sol2"
sol2.kind = "StaticLib"
sol2.language = "C++"

sol2.files = function()
  files {
    "./sol/*.hpp", 
    "./lua/*.h",
    "./lua/*.c",
  }
end

sol2.include_dirs = function()
  includedirs { "./sol" , "./lua" }
end

sol2.defines = function()
  defines { "SOL_ALL_SAFETIES_ON" }
end

AddExternalProject(sol2)