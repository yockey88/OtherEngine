require("externals")
require("workspace")

local function ProjectHeader(project_data)
  project (project_data.name)
    kind (project_data.kind)
    language (project_data.language)
    if project_data.language == "C#" then
      dotnetframework "4.7.2"
    end

    if project_data.cppdialect ~= nil then
      cppdialect (project_data.cppdialect)
    else
      cppdialect "C++latest"
    end

    if project_data.kind == "StaticLib" then
      staticruntime "On"
    elseif project_data.kind ~= "SharedLib" and project_data.staticruntime ~= nil then
      staticruntime (project_data.staticruntime)
    elseif project_data.kind ~= "SharedLib" then
      staticruntime "On"
    end

    if project_data.tdir == nil then
      assert(Tdir ~= nil, "Tdir is nil")
      project_data.tdir = Tdir
    end

    if project_data.odir == nil then
      assert(Odir ~= nil, "Odir is nil")
      project_data.odir = Odir
    end

    targetdir (project_data.tdir)
    objdir (project_data.odir)
end

local function ProcessModuleConfigurations(project , external)
  filter "configurations:Debug"
    defines { "OE_DEBUG_BUILD" }
    if project.debug_configuration ~= nil then
      project.debug_configuration()
    else
      debugdir "."
      optimize "Off"
      symbols "On"
    end

  filter "configurations:Release"
    defines { "OE_RELEASE_BUILD" }
    if project.release_configuration ~= nil then
      project.release_configuration()
    else
      optimize "On"
      symbols "Off"
    end
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
      defines { "OE_DEBUG_BUILD" }
      if project.debug_configuration ~= nil then
        project.debug_configuration()
      else
        debugdir "."
        optimize "Off"
        symbols "On"
      end
      if not external then
        ProcessDependencies("Debug")
        if project.extra_dependencies ~= nil then
          project.extra_dependencies("Debug")
        end
      end

    filter "configurations:Release"
      defines { "OE_RELEASE_BUILD" }
      if project.release_configuration ~= nil then
        project.release_configuration()
      else
        optimize "On"
        symbols "Off"
      end
      if not external then
        ProcessDependencies("Release")
        if project.extra_dependencies ~= nil then
          project.extra_dependencies("Release")
        end
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

    if (project.links ~= nil) then
      project.links()
    end

    if (project.externalincludedirs ~= nil) then
      project.externalincludedirs()
    end

    if project.defines ~= nil then
      project.defines()
    end

    ProcessConfigurations(project , true)
end

function AddModule(project)
  local success, message = VerifyProject(project)
  if not success then
    print(" -- Error: " .. message)
    return
  end

  project.include_dirs = project.include_dirs or function() end

  print(" -- Adding Module : " .. project.name)
  ProjectHeader(project)
    project.files()

    if project.defines ~= nil then
      project.defines()
    end

    if project.language == "C++" then
      project.include_dirs()
    end

    ProcessModuleComponents(project)
    ProcessModuleConfigurations(project)
end

function AddProject(project)
  local success, message = VerifyProject(project)
  if not success then
    print(" -- Error: " .. message)
    return
  end

  project.include_dirs = project.include_dirs or function() end
  project.external_include_dirs = project.external_include_dirs or function() end

  print(" -- Adding project : " .. project.name)
  ProjectHeader(project)
    project.files()
    project.include_dirs()
    project.external_include_dirs()

    ProcessProjectComponents(project)

    if project.defines ~= nil then
      project.defines()
    end

    ProcessConfigurations(project)

    if project.post_build_commands ~= nil then
      project.post_build_commands()
    end
end
