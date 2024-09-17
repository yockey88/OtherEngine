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
    ["OtherEngine"] = { "./OtherEngine" } ,
    ["OtherEngine-CsCore"] = { "./OtherEngine-ScriptCore/cs" } ,
    ["DotOther"] = { "./DotOther" } ,

    ["OtherEngine-Tools"] = { "./OtherEngine-Launcher" } ,

    ["Testing"] = { 
      "./tests" , 
      "./OtherTestEngine" 
    } ,

    ["Tools"] = { 
      "./tools" ,
      "./CsBindingGenerator" ,
    } ,
    ["Games"] = { "./yockcraft" } ,
}

local choc = {}
choc.name = "choc"
choc.include_dir = "%{wks.location}/externals/choc"

local entt = {}
entt.name = "entt"
entt.include_dir = "%{wks.location}/externals/entt"

local glad = {}
glad.name = "glad"
glad.path = "./externals/glad"
glad.include_dir = "%{wks.location}/externals/glad/include"
glad.lib_name = "glad"
glad.lib_dir = "%{wks.location}/bin/Debug/glad"

local glm = {}
glm.name = "glm"
glm.include_dir = "%{wks.location}/externals/glm"

local gtest = {}
gtest.name = "gtest"
gtest.path = "./externals/gtest"
gtest.include_dir = "%{wks.location}/externals/gtest/googletest/include/gtest"
gtest.lib_name = "gtest"
gtest.lib_dir = "%{wks.location}/bin/Debug/gtest"

local imgui = {}
imgui.name = "imgui"
imgui.path = "./externals/imgui"
imgui.include_dir = "%{wks.location}/externals/imgui"
imgui.lib_name = "imgui"
imgui.lib_dir = "%{wks.location}/bin/Debug/imgui"

local magic_enum = {}
magic_enum.name = "magic_enum"
magic_enum.include_dir = "%{wks.location}/externals/magic_enum/magic_enum"

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
sdl2.configurations = { "Debug" , "Release" }

local spdlog = {}
spdlog.name = "spdlog"
spdlog.path = "./externals/spdlog"
spdlog.include_dir = "%{wks.location}/externals/spdlog/include"
spdlog.lib_name = "spdlog"
spdlog.lib_dir = "%{wks.location}/bin/Debug/spdlog"

local zep = {}
zep.name = "zep"
zep.path = "./externals/zep"
zep.include_dir = "%{wks.location}/externals/zep/include"
zep.lib_name = "zep"

local mono = {}
mono.name = "mono"
mono.include_dir = "%{wks.location}/externals/mono/include"
mono.lib_dir = "%{wks.location}/externals/mono/lib"
mono.lib_name = "mono-2.0-sgen"

local sol2 = {}
sol2.name = "sol2"
sol2.path = "./externals/sol2"
sol2.include_dir = "%{wks.location}/externals/sol2"
sol2.lib_name = "sol2"

local box2d = {}
box2d.name = "box2d"
box2d.path = "./externals/box2d"
box2d.include_dir = "%{wks.location}/externals/box2d"
box2d.lib_name = "box2d"

local stb = {}
stb.name = "stb"
stb.include_dir = "%{wks.location}/externals/stb"

local jolt = {}
jolt.name = "jolt"
jolt.path = "./externals/jolt"
jolt.include_dir = "%{wks.location}/externals/jolt"
jolt.lib_name = "jolt"

local tracy = {}
tracy.name = "tracy"
tracy.path = "./externals/tracy"
tracy.include_dir = "%{wks.location}/externals/tracy/tracy"
tracy.lib_name = "tracy"

function query_terminal(command)
  local success, handle = pcall(io.popen, command)
  if not success then 
      return ""
  end

  result = handle:read("*a")
  handle:close()
  result = string.gsub(result, "\n$", "") -- remove trailing whitespace
  return result
end

function get_python_path()
  local p = query_terminal('cmd.exe /c python -c "import sys; import os; print(os.path.dirname(sys.executable))"')
  
  -- sanitize path before returning it
  p = string.gsub(p, "\\", "/") -- replace double backslash
  return p
end

function get_python_lib()
  return query_terminal("cmd.exe /c python -c \"import sys; import os; import glob; path = os.path.dirname(sys.executable); libs = glob.glob(path + '/libs/python*'); print(os.path.splitext(os.path.basename(libs[-1]))[0]);\"")
end

python_path = get_python_path()
python_include_path = python_path .. "/include"
python_lib_path = python_path .. "/libs"
python_lib = get_python_lib()
if python_path == "" or python_lib == "" then
  error("Failed to find python path or pybind11 dependency")
else
  print("Python Path: " .. python_path)
  print("Python Include Path: " .. python_include_path)
  print("Python Lib Path: " .. python_lib_path)
  print("Python Lib: " .. python_lib)
end

PythonPaths = {
  path = python_path,
  include_path = python_include_path,
  lib_path = python_lib_path,
  lib = python_lib
}

local python = {}
python.name = "python"
python.path = PythonPaths.path
python.include_dir = PythonPaths.include_path
python.lib_dir = PythonPaths.lib_path
python.lib_name = PythonPaths.lib

local pybind = {}
pybind.name = "pybind11"
pybind.path = "./externals/pybind11"
pybind.include_dir = "%{wks.location}/externals/pybind11"
pybind.lib_name = "pybind11"

AddDependency(choc)
AddDependency(entt)
AddDependency(glad)
AddDependency(glm)
AddDependency(gtest)
AddDependency(imgui)
AddDependency(magic_enum)
AddDependency(nativefiledialog)
AddDependency(sdl2)
AddDependency(spdlog)
AddDependency(zep)
AddDependency(mono)
AddDependency(sol2)
AddDependency(box2d)
AddDependency(stb)
AddDependency(jolt)
AddDependency(pybind)
-- AddDependency(tracy)

CppWorkspace(configuration)
