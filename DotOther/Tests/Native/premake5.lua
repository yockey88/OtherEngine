local host_tests = {
  name = "HostTests",
  path = "./DotOther/Tests/Native",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",
  tdir = "%{wks.location}/bin/%{cfg.buildcfg}/DotOther.Tests",

  files = function()
    files {
      "host_tests.cpp" ,
      "dotest.cpp" ,
    }
  end,

  include_dirs = function()
    includedirs {
      "." ,
    }
  end,

  links = function()
    libdirs {
      "%{wks.location}/DotOther/NetCore" ,
    }
  end,

  defines = function()
    defines {
      "OE_MODULE" ,
    }
  end,

  windows_configuration = function()
    defines {
      "DOTOTHER_WINDOWS",
    }
  end,

  post_build_commands = function()
    filter { "system:windows" }
      postbuildcommands {
        "{COPY} %{wks.location}/DotOther/NetCore/nethost.dll %{cfg.targetdir}",
      }
  end
}

local function add_dotother_comps(project)
  project.components = {}
  project.components["DotOther.Native"] = "%{wks.location}/DotOther/Native/src"
  project.components["nethost"] = "%{wks.location}/DotOther/NetCore"
  project.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"
  project.components[""] = "%{wks.location}/externals/gtest/googlemock/include"
end

add_dotother_comps(host_tests)
AddProject(host_tests)