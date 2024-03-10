require("ymake")

local configuration = {}
configuration.wks_name = "OtherEngine"
configuration.architecture = "x64"
configuration.start_project = "OtherEngine"
configuration.cpp_dialect = "C++latest"
configuration.static_runtime = "on"
configuration.target_dir = "%{wks.location}/bin/%{cfg.buildcfg}/%{prj.name}"
configuration.obj_dir = "%{wks.location}/bin_obj/%{cfg.buildcfg}/%{prj.name}"

configuration.build_configurations = { "Debug", "Release" }
configuration.platforms = { "Windows" }

configuration.groups = {
    ["OtherEngine"] = { "./OtherEngine" }
}

if Contains(configuration.build_configurations , "Debug") then
    print("[ Debug ]")
end

local choc = {}
choc.name = "choc"
choc.include_dir = "./externals/choc/choc"

local entt = {}
entt.name = "entt"
entt.include_dir = "./externals/entt/entt"

local glad = {}
glad.name = "glad"
glad.path = "./externals/glad"
glad.include_dir = "%{wks.location}/externals/glad/include"
glad.lib_name = "glad"
glad.lib_dir = "%{wks.location}/bin/Debug/glad"

local glm = {}
glm.name = "glm"
glm.include_dir = "%{wks.location}/externals/glm/glm"

local imgui = {}
imgui.name = "imgui"
imgui.path = "./externals/imgui"
imgui.include_dir = "%{wks.location}/externals/imgui/imgui"
imgui.lib_name = "imgui"
imgui.lib_dir = "%{wks.location}/bin/Debug/imgui"

local lua = {}
lua.name = "lua"
lua.path = "./externals/lua"
lua.include_dir = "%{wks.location}/externals/lua/lua"
lua.lib_name = "lua"
lua.lib_dir = "%{wks.location}/bin/Debug/lua"

local magic_enum = {}
magic_enum.name = "magic_enum"
magic_enum.include_dir = "%{wks.location}/externals/magic_enum/magic_enum"

local mono = {}
mono.name = "mono"
mono.include_dir = "%{wks.location}/externals/mono/include"
mono.lib_dir = "%{wks.location}/externals/mono/lib"
mono.lib_name = "mono-2.0-sgen"

local nativefiledialog = {}
nativefiledialog.name = "nativefiledialog"
nativefiledialog.path = "./externals/nativefiledialog"
nativefiledialog.include_dir = "%{wks.location}/externals/nativefiledialog/src"
nativefiledialog.lib_name = "nfd"
nativefiledialog.lib_dir = "%{wks.location}/bin/Debug/nfd"

local sdl2 = {}
sdl2.name = "sdl2"
sdl2.include_dir = "%{wks.location}/externals/sdl2/SDL2"
sdl2.lib_dir = "%{wks.location}/externals/sdl2/lib/%{cfg.buildcfg}"
sdl2.lib_name = "SDL2"
sdl2.debug_lib_name = "SDL2d"
sdl2.configurations = "Debug" , "Release"

local spdlog = {}
spdlog.name = "spdlog"
spdlog.path = "./externals/spdlog"
spdlog.include_dir = "%{wks.location}/externals/spdlog/spdlog"
spdlog.lib_name = "spdlog"
spdlog.lib_dir = "%{wks.location}/bin/Debug/spdlog"

local zep = {}
zep.name = "zep"
zep.path = "./externals/zep"
zep.include_dir = "%{wks.location}/externals/zep/include"
zep.lib_name = "zep"

AddDependency(choc)
AddDependency(entt)
AddDependency(glad)
AddDependency(glm)
AddDependency(imgui)
AddDependency(lua)
AddDependency(magic_enum)
AddDependency(mono)
AddDependency(nativefiledialog)
AddDependency(sdl2)
AddDependency(spdlog)
AddDependency(zep)

CppWorkspace(configuration)