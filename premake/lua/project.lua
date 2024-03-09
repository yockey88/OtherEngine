include "lua/externals.lua"

local function ProjectHeader(project_data)
  project (project_data.name)
    kind (project_data.kind)
    language "C++"

    if project_data.cppdialect ~= nil then
      cppdialect (project_data.cppdialect)
    else
      cppdialect "C++latest"
    end

    if project_data.kind == "StaticLib" then
      staticruntime "On"
    elseif project_data.kind ~= "DynamicLib" and project_data.staticruntime ~= nil then
      staticruntime (project_data.staticruntime)
    elseif project_data.kind ~= "DynamicLib" then
      staticruntime "On"
    end

    targetdir (tdir)
    objdir (odir)
end

local function ProcessProjectComponents(project)
end

local function ProcessConfigurations(project , external)
    filter "system:windows"
      if project.windows_configuration ~= nil then
          project.windows_configuration()
      else
          systemversion "latest"
      end

    filter { "system:windows", "configurations:Debug" }
      if project.windows_debug_configuration ~= nil then
        project.windows_debug_configuration()
      else
        editandcontinue "Off"
        flags { "NoRuntimeChecks" }
        defines { "NOMINMAX" }
      end

    filter { "system:windows", "configurations:Release" }
      if project.windows_release_configuration ~= nil then
        project.windows_release_configuration()
      end

    filter "system:linux"
      if project.linux_configuration ~= nil then
        project.linux_configuration()
      end

    filter "configurations:Debug"
      if project.debug_configuration ~= nil then
        project.debug_configuration()
      else
        debugdir "."
        optimize "Off"
        symbols "On"
      end
      if not external then
        ProcessDependencies("Debug")
      end

    filter "configurations:Release"
      if project.release_configuration ~= nil then
        project.release_configuration()
      else
        optimize "On"
        symbols "Off"
      end
      if not external then
        ProcessDependencies("Release")
      end
end

local function VerifyProject(project)
  if project == nil then
    return false, "AddProject: project is nil"
  end

  if project.name == nil then
    return false, "AddProject: project.name is nil"
  end

  if project.kind == nil then
    return false, "AddProject: project.kind is nil"
  end

  if project.files == nil then
    return false, "AddProject: project.files is nil"
  end

  return true , ""
end

function AddExternalProject(project)
  local success, message = VerifyProject(project)
  if not success then
    print(" -- Error: " .. message)
    return
  end

  project.include_dirs = project.include_dirs or function() end

  print(" -- Adding Dependency : " .. project.name)
  ProjectHeader(project)
    project.files()
    project.include_dirs()

    if project.defines ~= nil then
      project.defines()
    end

    ProcessConfigurations(project , true)
end

function AddProject(project)
  local success, message = VerifyProject(project)
  if not success then
    print(" -- Error: " .. message)
    return
  end

  project.include_dirs = project.include_dirs or function() end

  print(" -- Adding project : " .. project.name)
  ProjectHeader(project)
    project.files()
    project.include_dirs()

    ProcessProjectComponents(project)

    if project.defines ~= nil then
      project.defines()
    end

    ProcessConfigurations(project)
    RunPostBuildCommands()
end
