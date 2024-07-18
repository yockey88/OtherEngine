require("util")
require("configuration")

Tdir = "%{wks.location}/bin/%{cfg.buildcfg}/%{prj.name}"
Odir = "%{wks.location}/bin_obj/%{cfg.buildcfg}/%{prj.name}"

local function WorkspaceHeader(config)
  workspace (config.wks_name)
  if config.architecture ~= nil then
    architecture (config.architecture)
  end

  if config.config_table == nil then
    config.config_table = { "Debug" , "Release" }
  end
  
  config.config_table = config.config_table or { "Debug" , "Release" }
  configurations (config.config_table)

  startproject (config.start_project)

  language "C++"
  if config.cpp_dialect ~= nil then
    cppdialect (config.cpp_dialect)
  else
    cppdialect "C++latest"
  end

  if config.static_runtime ~= nil then
    staticruntime (config.static_runtime)
  else
    staticruntime "On"
  end
end

local function ProcessGroups(groups)
  for key , value in pairs(groups) do
    print("[ Group ] : " .. key)
    group (key)
    for _, dir in pairs(value) do
      include (dir)
    end
    group ""
    print("")
  end
end

local function SetTargets(config)
  if config.target_dir ~= nil then
    targetdir (config.target_dir)
    Tdir = config.target_dir
  else
    targetdir (Tdir)
  end

  if config.obj_dir ~= nil then
    objdir (config.obj_dir)
    Odir = config.obj_dir
  else
    objdir (Odir)
  end
end

local function AddSource(config)
  print("[ Processing source code ]")
  AddDependencies()

  if config.groups ~= nil and config.project_folders == nil and config.project == nil then
    ProcessGroups(config.groups)
  elseif config.project_folders ~= nil and config.groups == nil and config.project == nil then
    for project in config.project_folders do
      print(" - Project : " .. project)
      include (project)
    end
  elseif config.project ~= nil and config.groups == nil and config.project_folders == nil then
    print(" - Project : " .. config.project)
    include (config.project)
  else
    print("UNREACHABLE : No source code provided")
  end
end

function CppWorkspace(config)
  print("\n===> Running Workspace Configuration <===")
  if not ValidConfig(config) then
    print("Workspace configuration failed")
    print("=====================================")
    print("\n")
    return
  end

  print("[ Creating workspace ] : " .. config.wks_name)
  WorkspaceHeader(config)
    if not Configure(config) then
      print("Workspace configuration failed")
      print("=====================================")
      return
    end
    print("=====================================\n")

    SetTargets(config)

    print("===> Processing source code <===")
    AddSource(config)
    print("=====================================\n")
end
