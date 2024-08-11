local gtest = {}

gtest.name = "gtest"
gtest.kind = "StaticLib"
gtest.language = "C++"

gtest.files = function()
  files {
    "googletest/src/**.cc" ,
    "googletest/src/**.h" ,
    "googletest/include/**.h" ,
    "googlemock/src/**.cc" ,
    "googlemock/src/**.h" ,
    "googlemock/include/**.h" ,
  }
end

gtest.include_dirs = function()
    includedirs { 
      "./googletest/include" , 
      "./googletest" , 
      "./googlemock/include" , 
      "./googlemock" 
    }
end

AddExternalProject(gtest)
