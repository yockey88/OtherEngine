local imflow = {}

imflow.name = "imflow"
imflow.kind = "StaticLib"
imflow.language = "C++"
imflow.cppdialect = "C++Latest"

imflow.files = function()
  files{
    "./src/*.cpp" ,
    "./src/*.h" ,
  }
end

imflow.include_dirs = function()
  includedirs {
    "." ,
    "./imflow" ,
    "../imgui/"
  }
end

imflow.external_include_dirs = function()end

AddExternalProject(imflow)
