require("ymake")

local configuration = {}
configuration.wks_name = "TestWorkspace"
configuration.architecture = "x64"
configuration.start_project = "runtime"
configuration.cpp_dialect = "C++latest"
configuration.static_runtime = "On"
configuration.target_dir = "%{wks.location}/bin/%{cfg.buildcfg}/%{prj.name}"
configuration.obj_dir = "%{wks.location}/bin_obj/%{cfg.buildcfg}/%{prj.name}"

-- TODO: upgrade platform/configuration system
configuration.build_configurations = { "Debug", "Release" }
configuration.platforms = { "Windows" }

configuration.groups = {
}

CppWorkspace(configuration)
