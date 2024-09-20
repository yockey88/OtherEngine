local native = {
  name = "DotOther.Native",
  path = "./Native",
  kind = "StaticLib",
  language = "C++",
  cppdialect = "C++latest",
  staticruntime = "Off",
  architecture = "x86_64",
  
  files = function()
    files {
      "./src/**.cpp",
      "./src/**.hpp",
      "../NetCore/*.h",
    }
  end,

  include_dirs = function()
    includedirs {
      "./src" ,
      "../NetCore" ,
    }
  end,

  defines = function()
    defines {
      "OE_MODULE" ,
    }
  end,

  windows_configuration = function()
    defines {
      "OE_MODULE" ,
      "DOTOTHER_WINDOWS" ,
    }
  end,

  components = {
    ["gtest"] = "%{wks.location}/externals/gtest/googletest/include"
  } ,
}

AddProject(native)