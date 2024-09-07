local OtherEngine = {}

OtherEngine.name = "OtherEngine"
OtherEngine.path = "./OtherEngine"
OtherEngine.kind = "StaticLib"
OtherEngine.language = "C++"
OtherEngine.cppdialect = "C++latest"

OtherEngine.files = function()
  files {
    "./src/**.cpp",
    "./src/**.hpp",
  }
end

OtherEngine.include_dirs = function()
  includedirs {
    "./src",
  }
end

OtherEngine.windows_configuration = function()
  files {
    "./platform/windows/**.hpp",
    "./platform/windows/**.cpp",
  }
  includedirs {
    "./platform",
  }
  systemversion "latest"
  buildoptions { "/EHsc" , "/Zc:preprocessor" , "/Zc:__cplusplus" }
end

OtherEngine.windows_debug_configuration = function()
  links {
    "DbgHelp",
  }
  defines {
    "TRACY_ENABLE" ,
    "TRACY_ON_DEMAND" ,
  }

  systemversion "latest"
  buildoptions { "/EHsc" , "/Zc:preprocessor" , "/Zc:__cplusplus" }
end

AddProject(OtherEngine)
