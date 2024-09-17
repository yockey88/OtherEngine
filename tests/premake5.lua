local sandbox = {}

sandbox.name = "sandbox"
sandbox.path = "./sandbox"
sandbox.kind = "ConsoleApp"
sandbox.language = "C++"
sandbox.cppdialect = "C++latest"

sandbox.files = function()
  files {
    "./sandbox/**.cpp",
    "./sandbox/**.hpp",
    "./sandbox_ui.cpp" ,
    "./sandbox_ui.hpp" ,
    "./mock_app.cpp"
  }
end

sandbox.include_dirs = function()
  includedirs {
    "./sandbox",
    "." ,
  }
end

sandbox.defines = function()
  defines {
    "OE_MODULE" ,
  }
end

sandbox.components = {}
sandbox.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
sandbox.components[""] = "%{wks.location}/externals/gtest/googlemock/include"

AddProject(sandbox)

local gl_sandbox = {}

gl_sandbox.name = "gl_sandbox"
gl_sandbox.path = "./gl_sandbox"
gl_sandbox.kind = "ConsoleApp"
gl_sandbox.language = "C++"
gl_sandbox.cppdialect = "C++latest"

gl_sandbox.files = function()
  files {
    "./gl_sandbox/**.cpp",
    "./gl_sandbox/**.hpp",
    "./sandbox_ui.cpp" ,
    "./sandbox_ui.hpp" ,
    "./mock_app.cpp"
  }
end

gl_sandbox.include_dirs = function()
  includedirs {
    "./gl_sandbox",
    "." ,
  }
end

gl_sandbox.defines = function()
  defines {
    "OE_MODULE" ,
  }
end

gl_sandbox.components = {}
gl_sandbox.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
gl_sandbox.components[""] = "%{wks.location}/externals/gtest/googlemock/include"

AddProject(gl_sandbox)

local unit_tests = {}
unit_tests.name = "unit_tests"
unit_tests.path = "./unit_tests"
unit_tests.kind = "ConsoleApp"
unit_tests.language = "C++"
unit_tests.cppdialect = "C++latest"

unit_tests.files = function()
  files {
    "./unit_tests/**.cpp" ,
  }
end

unit_tests.include_dirs = function()
  includedirs {
    "." ,
  }
end

unit_tests.links = function()
  libdirs {
    "%{wks.location}/DotOther/NetCore" ,
  }
end

unit_tests.components = {}
unit_tests.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
unit_tests.components["DotOther.Native"] = "%{wks.location}/DotOther/Native/src"
unit_tests.components["nethost"] = "%{wks.location}/DotOther/NetCore"
unit_tests.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"
unit_tests.components[""] = "%{wks.location}/externals/gtest/googlemock/include"

AddProject(unit_tests)

local dotother_tests = {
  name = "dotother_tests",
  path = "./dotother_tests",
  kind = "ConsoleApp",
  language = "C++",
  cppdialect = "C++latest",

  files = function()
    files {
      "./dotother_tests/**.cpp" ,
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
      "DOTOTHER_WINDOWS" ,
    }
  end,

  post_build_commands = function()
    local target = "%{wks.location}/bin/%{cfg.buildcfg}/dotother_tests"

    local nethostdll = "%{wks.location}/DotOther/NetCore/nethost.dll"
    -- local runtimeconfig = "%{wks.location}/bin/%{cfg.buildcfg}/DotOther.Managed/net8.0/DotOther.Managed.runtimeconfig.json"
    -- local dll = "%{wks.location}/bin/%{cfg.buildcfg}/DotOther.Managed/net8.0/DotOther.Managed.dll"
    -- local pdb = "%{wks.location}bin/%{cfg.buildcfg}/DotOther.Managed/net8.0/DotOther.Managed.pdb"

    postbuildcommands {
      '{COPY} ' .. nethostdll .. " " .. target ,
    }
  end,
}

dotother_tests.components = {}
dotother_tests.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"
dotother_tests.components["DotOther.Native"] = "%{wks.location}/DotOther/Native/src"
dotother_tests.components["nethost"] = "%{wks.location}/DotOther/NetCore"
dotother_tests.components["gtest"] = "%{wks.location}/externals/gtest/googletest/include"
dotother_tests.components[""] = "%{wks.location}/externals/gtest/googlemock/include"

AddProject(dotother_tests)