local function ProcessWindowsConfigurations(config)
  filter "system:windows"
    if config.windows_configuration ~= nil then
      config.windows_configuration()
    else
      print(" -- Default windows configuration")
      buildoptions { "/EHsc" , "/Zc:preprocessor" , "/Zc:__cplusplus" }
    end

  filter { "system:windows" , "configurations:Debug" }
    if config.windows_debug_configuration ~= nil then
      config.windows_debug_configuration()
    else
      print(" -- Default windows debug configuration")
      editandcontinue "Off"
      -- sanitize { "Address" } -- address sanitizer seems to be broken on my machine :/
      flags { "NoRuntimeChecks" }
      defines { "NOMINMAX" }
    end

  filter { "system:windows" , "configurations:Release" }
    if config.windows_release_configuration ~= nil then
      config.windows_release_configuration()
    else
      print(" -- Default windows release configuration")
    end
end

local function ProcessLinuxConfigurations(config)
  filter "system:linux"
    if config.linux_configuration ~= nil then
      config.linux_configuration()
    else
      print(" -- Default linux configuration")
    end

  filter { "system:linux" , "configurations:Debug" }
    if config.linux_debug_configuration ~= nil then
      config.linux_debug_configuration()
    else
      print(" -- Default linux debug configuration")
    end

  filter { "system:linux" , "configurations:Release" }
    if config.linux_release_configuration ~= nil then
      config.linux_release_configuration()
    else
      print(" -- Default linux release configuration")
    end
end

local function ProcessSystemConfigurations(config)
  if ContainsValue(config.platforms , "Windows") then
    ProcessWindowsConfigurations(config)
  end

  if Contains(config.platforms , "Linux") then
    ProcessLinuxConfigurations(config)
  end
end

local function ProcessConfigurations(config)
  print("[ Processing Configurations ]")

  if ContainsValue(config.build_configurations , "Debug") then
    filter "configurations:Debug"
    if config.debug_configuration ~= nil then
      config.debug_configuration()
    else
      debugdir "."
      optimize "Off"
      symbols "On"
    end
  else
    print(" -- No debug configuration")
  end

  if ContainsValue(config.build_configurations , "Release") then
    filter "configurations:Release"
      if config.release_configuration ~= nil then
        config.release_configuration()
      else
        optimize "On"
        symbols "Off"
      end
  end

  ProcessSystemConfigurations(config)
end

local function ProcessWorkspaceSettings(config)
  if config.flags ~= nil then
    flags (config.flags)
  else
    flags { "MultiProcessorCompile" }
  end

  if config.defines ~= nil then
    defines (config.defines)
  else
    defines {
      "_CRT_SECURE_NO_WARNINGS" ,
      "_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING" ,
      "_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS"
    }
  end
end

local function ProcessWorkspaceConfig(config)
  filter "action:vs*"
    if config.vs_filters ~= nil then
        config.vs_filters()
    else
        linkoptions { "/ignore:4099" , "/ignore:4098" , "/ignore:4006" }
        disablewarnings { "68" }
    end

  ProcessConfigurations(config)
end

function ValidConfig(config)
  if config == nil then
    print("No configuration provided")
    return false
  end

  if config.wks_name == nil then
    print("No workspace name provided")
    return false
  end

  if config.start_project == nil then
    print("No start project provided")
    return false
  end

  if config.groups == nil and config.project_folders == nil and config.project == nil then
    print("No source code  provided")
    return false
  end

  return true
end

function Configure(config)
  if not ValidConfig(config) then
    return false
  end

  ProcessWorkspaceSettings(config)
  ProcessWorkspaceConfig(config)
  return true
end
