require("externals")
require("workspace")

local function ProjectHeader(project_data)
  project (project_data.name)
    kind (project_data.kind)
    language (project_data.language)

    if project_data.architecture ~= nil then
      architecture (project_data.architecture)
    else
      architecture "x86_64"
    end

    if project_data.language == "C#" then
      project_data.dotnetframework = project_data.dotnetframework or "4.7.2"
      project_data.clr = project_data.clr or "Unsafe"
      dotnetframework (project_data.dotnetframework)
      clr (project_data.clr)
    end

    if project_data.cppdialect ~= nil then
      cppdialect (project_data.cppdialect)
    else
      cppdialect "C++latest"
    end

    staticruntime "off"

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

    if project_data.extension ~= nil then
      targetextension (project_data.extension)
    end
end

local function ProcessFilters(project)
  filter { "system:windows" }
    if project.windows_filters ~= nil then
      project.windows_filters()
    end

  filter { "system:linux" }
    if project.linux_filters ~= nil then
      project.linux_filters()
    end
end

function ProcessModuleComponents(module)
  for lib, comp in pairs(module.components) do
    if string.len(lib) > 0 then
      links { lib }
    end
    if module.language == "C++" and string.len(comp) > 0 then
      includedirs { comp }
    end
  end

  filter "system:windows"
    if module.windows_configuration ~= nil then
      module.windows_configuration()
    end
    
    if module.language == "C#" then
      clr "Unsafe"
      propertytags {
        { "AppendTargetFrameworkToOutputPath", "false" },
        { "Nullable", "enable" },
      }
    end
end

function ProcessProjectComponents(project)
  for lib, comp in pairs(project.components) do
    if string.len(lib) > 0 then
      links { lib }
    end
    if project.language == "C++" and string.len(comp) > 0 then
      includedirs { comp }
    end
  end
end

local function ProcessConfigurations(project , external)
    project.cross_platform_main = project.cross_platform_main or false
    project.windows_configuration = project.windows_configuration or function() end
    project.windows_debug_configuration = project.windows_debug_configuration or function() end
    project.windows_release_configuration = project.windows_release_configuration or function() end
    project.linux_configuration = project.linux_configuration or function() end
    project.linux_debug_configuration = project.linux_debug_configuration or function() end
    project.linux_release_configuration = project.linux_release_configuration or function() end

    project.debug_configuration = project.debug_configuration or function() end
    project.extra_dependencies = project.extra_dependencies or function(str)end
    project.release_configuration = project.release_configuration or function() end

    filter "system:windows"
      systemversion "latest"
      if project.kind == "ConsoleApp" and project.cross_platform_main then
        entrypoint "mainCRTStartup"
      elseif project.kind == "ConsoleApp" then
        entrypoint "WinMainCRTStartup"
      end

      defines { "OE_WINDOWS" }
      project.windows_configuration()

    filter { "system:windows", "configurations:Debug" }
      editandcontinue "Off"
      flags { "NoRuntimeChecks" }
      defines { "NOMINMAX" }
      project.windows_debug_configuration()

    filter { "system:windows", "configurations:Release" }
      project.windows_release_configuration()

    filter "system:linux"
      defines { "OE_LINUX" }
      project.linux_configuration()

    filter { "system:linux", "configurations:Debug" }
      project.linux_debug_configuration()

    filter { "system:linux", "configurations:Release" }
      project.linux_release_configuration()

    filter "configurations:Debug"
      runtime "Debug"
      debugdir "."
      optimize "Off"
      symbols "On"
      project.debug_configuration()

      if not external and project.language == "C++" then
        ProcessDependencies("Debug")
        project.extra_dependencies("Debug")
      end

    filter "configurations:Release"
      runtime "Release"
      optimize "Full"
      symbols "Off"
      defines { "OE_RELEASE" }
      project.release_configuration()

      if not external and project.language == "C++" then
        ProcessDependencies("Release")
        project.extra_dependencies("Release")
      end

    if project.custom_configurations ~= nil then
      project.custom_configurations()
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

  if project.components == nil then
    project.components = {}
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

function _AddProjectOrModule(project , component_func)
  local success, message = VerifyProject(project)
  if not success then
    print(" -- Error: " .. message)
    return
  end

  project.include_dirs = project.include_dirs or function() end
  project.links = project.links or function() end
  project.defines = project.defines or function() end

  print(" -- Adding project : " .. project.name)
  ProjectHeader(project)
    project.files()
    project.include_dirs()

    component_func(project)

    project.links()
    project.defines()

    if project.post_build_commands ~= nil then
      project.post_build_commands()
    elseif (project.language == "C#" and project.needs_dlls) or (project.language == "C++") then
      ProcessFilters(project)
    end

    ProcessConfigurations(project , false)

    if project.language == "C#" and project.nuget_packages ~= nil then
      project.nuget_packages()
    end
end

function AddModule(project)
  _AddProjectOrModule(project , ProcessModuleComponents)
end

function AddConsoleProject(project)
  project.cross_platform_main = true
  _AddProjectOrModule(project , ProcessProjectComponents)
end

function AddProject(project)
  _AddProjectOrModule(project , ProcessProjectComponents)
end
