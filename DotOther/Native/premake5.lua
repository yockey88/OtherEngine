local native = {
  name = "DotOther.Native",
  path = "./Native",
  kind = "StaticLib",
  language = "C++",
  cppdialect = "C++latest",
  staticruntime = "Off",
  
  files = function()
    files {
      "./**.cpp",
      "./**.hpp",
      "../NetCore/*.h",
    }
  end,

  include_dirs = function()
    includedirs {
      "." ,
      "../NetCore" ,
    }
    links {
      "spdlog"
    }
    externalincludedirs {
      "%{wks.location}/externals/refl-cpp",
      "%{wks.location}/externals/spdlog/include",
      "%{wks.location}/externals/magic_enum",
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
    }
  end,

  windows_debug_configuration = function()
    defines {
      "DOTOTHER_WINDOWS_DEBUG"
    }

    links {
      "gtest"
    }
    externalincludedirs {
      "%{wks.location}/externals/gtest/googletest/include"
    }
  end,
}

AddExternalProject(native)