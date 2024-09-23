require("externals")
require("workspace")

local function ProjectHeader(project_data)
  project (project_data.name)
    kind (project_data.kind)
    language (project_data.language)

    if project_data.architecture ~= nil then
      architecture (project_data.architecture)
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
    
    if project_data.kind == "StaticLib" then
      staticruntime "On"
    elseif project_data.staticruntime ~= nil then
      staticruntime (project_data.staticruntime)
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

    if project_data.extension ~= nil then
      targetextension (project_data.extension)
    end
end

local function ProcessWindowsFilters()
  -- windows requires dlls to be copied to the output directory
  postbuildcommands {
    '{COPY} %{wks.location}externals/mono/bin/mono-2.0-sgen.dll "%{cfg.targetdir}"',
    '{COPY} %{wks.location}externals/mono/bin/MonoPosixHelper.dll "%{cfg.targetdir}"',
  }
  filter { "configurations:Release" }
    postbuildcommands {
      '{COPY} "%{wks.location}externals/sdl2/lib/Release/SDL2.dll" "%{cfg.targetdir}"',
    }
  filter { "configurations:Debug" }
    postbuildcommands {
      '{COPY} "%{wks.location}externals/mono/bin/mono-2.0-sgen.pdb" "%{cfg.targetdir}"',
      '{COPY} "%{wks.location}externals/mono/bin/MonoPosixHelper.pdb" "%{cfg.targetdir}"',
      '{COPY} "%{wks.location}externals/sdl2/lib/Debug/SDL2d.dll" "%{cfg.targetdir}"',
    }
end

local function ProcessLinuxFilters()
end

local function ProcessFilters(project)
  filter { "system:windows" }
    ProcessWindowsFilters()

  filter { "system:linux" }
    ProcessLinuxFilters()
end

function ProcessModuleComponents(module)
  if module == nil then
    return
  end

  for lib, comp in pairs(module.components) do
    if string.len(lib) > 0 then 
      links { lib }
    end
    if module.language == "C++" and string.len(comp) > 0 then
      includedirs { comp }
    end
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
        runtime "Debug"
        debugdir "."
        optimize "Off"
        symbols "On"
        -- conformancemode "On"
      end

      if not external and project.language == "C++" then
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
        runtime "Release"
        optimize "Full"
        symbols "Off"
        -- conformancemode "On"
      end

      if not external and project.language == "C++" then
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
    project.external_include_dirs()

    component_func(project)

    project.links()
    project.defines()
    
    if project.post_build_commands ~= nil then
      project.post_build_commands()
    elseif (project.language == "C#" and project.needs_dlls) or
       (project.language == "C++") then
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

function AddProject(project)
  _AddProjectOrModule(project , ProcessProjectComponents)
end
