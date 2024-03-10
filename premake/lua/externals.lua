include "ordered_pairs.lua"

local function FirstToUpper(str)
  return (str:gsub("^%l", string.upper))
end

local external_paths = {}

External = {
  -- Example
  -- Mono = {
  --   include_dir = "%{wks.location}/externals/mono/include" ,
  --   lib_name = "mono-2.0-sgen" ,
  --   lib_dir = "%{wks.location}/externals/mono/lib" ,
  --   configurations = "Debug" , "Release"
  -- } ,
}


local function LinkDependency(table, debug, target)
  -- Setup library directory
  if table.lib_dir ~= nil then
    libdirs { table.lib_dir }
  end

  -- Try linking
  local lib_name = nil
  if table.lib_name ~= nil then
    lib_name = table.lib_name
  end

  if table.debug_lib_name ~= nil and debug and target == "Windows" then
    lib_name = table.debug_lib_name
  end

  if lib_name ~= nil then
    links { lib_name }
    return true
  end

  return false
end

local function AddInclude(table)
  if table.include_dir ~= nil then
    externalincludedirs { table.include_dir }
  end
end

function ProcessDependencies(configuration)
  if #External == 0 then
    return
  end

  local target = FirstToUpper(os.target())

  for key, lib_data in OrderedPairs(External) do
    local matches_config = true

    if configuration ~= nil and lib_data.configurations ~= nil then
      matches_config = string.find(lib_data.configurations, configuration)
    end

    local is_debug = configuration == "Debug"

    if matches_config then
      local continue_link = true

      if lib_data[target] ~= nil then
        continue_link = not LinkDependency(lib_data[target], is_debug, target)
        AddInclude(lib_data[target])
      end

      if continue_link then
        LinkDependency(lib_data, is_debug, target)
      end

      AddInclude(lib_data)
    end
  end
end

function IncludeDependencies(configuration)
  local target = FirstToUpper(os.target())

  for key, lib_data in OrderedPairs(configuration) do
    local matches_config = true

    if configuration ~= nil and lib_data.Configurations ~= nil then
      matches_config = string.find(lib_data.Configurations, configuration)
    end

    if matches_config then
      AddInclude(lib_data)

      if lib_data[target] ~= nil then
        AddInclude(lib_data[target])
      end
    end
  end
end

function AddDependencies()
  if #external_paths == 0 then
    return
  else
    print("Processing external dependencies")
  end

  print("[ Group ] : Externals")
  group "Externals"
  for _, lib_data in OrderedPairs(external_paths) do
    include (lib_data)
  end
  group ""
end

function AddDependency(data)
  if data.name == nil then
    print("AddDependency : no name given")
    return
  end

  if data.path ~= nil then
    table.insert(external_paths, data.path)
  end

  table.insert(External, data)
end
