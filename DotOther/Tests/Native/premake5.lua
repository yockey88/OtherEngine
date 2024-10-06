local host_tests = {
  name = "HostTests",
  path = "./DotOther/Tests/Native",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",
  tdir = "%{wks.location}/bin/%{cfg.buildcfg}/DotOther.Tests",

  files = function()
    files {
      "specialized_tests/host_tests.cpp" ,
      "core/*.cpp" ,
    }
  end,

  post_build_commands = function()
    filter { "system:windows" }
      postbuildcommands {
        "{COPY} %{wks.location}/DotOther/NetCore/nethost.dll %{cfg.targetdir}",
      }
  end
}

local do_unit_tests = {
  name = "UnitTests",
  path = "./Dotother/Tests/Native",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",
  tdir = "%{wks.location}/bin/%{cfg.buildcfg}/DotOther.Tests",

  files = function()
    files {
      "unit_tests/*.cpp" ,
      "core/*.cpp" ,
    }
  end,
}

local function add_dotother_comps(project, addnethost)
  addnethost = addnethost or false

  project.include_dirs = function()
    includedirs {
      "." ,
      "%{wks.location}/DotOther/Native" ,
    }
  end

  project.defines = function()
    defines {
      "OE_MODULE" ,
    }
  end

  project.windows_configuration = function()
    defines {
      "DOTOTHER_WINDOWS",
    }
  end

  project.components = {}
  project.components["DotOther.Native"] = "%{wks.location}/DotOther/Native/src"
  project.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"
  project.components[""] = "%{wks.location}/externals/gtest/googlemock/include"
  
  if addnethost then
    old_include_dirs = project.include_dirs or function() end
    
    project.include_dirs = function()
      old_include_dirs()
      includedirs {
        "%{wks.location}/DotOther/NetCore" ,
      }
    end

    project.links = function()
      libdirs {
        "%{wks.location}/DotOther/NetCore" ,
      }
    end

    project.components["nethost"] = "%{wks.location}/DotOther/NetCore"
  end

end

add_dotother_comps(host_tests, true)
add_dotother_comps(do_unit_tests, true)
AddProject(host_tests)
AddProject(do_unit_tests)
