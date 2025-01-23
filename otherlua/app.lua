local M = {}

M.project_table = function(name, path, kind, architecture)
  if not name then
    error("name is required")
  end

  if not path then
    error("path is required")
  end

  kind = kind or "ConsoleApp"
  architecture = architecture or "x86_64"

  cpp_path = "./" .. name .. "/**.cpp"
  hpp_path = "./" .. name .. "/**.hpp"

  include_path = "./" .. name

  local table = {
    name = name,
    path = path,
    kind = kind,
    architecture = architecture,

    files = function()
      files { cpp_path, hpp_path }
    end,
  
    include_dirs = function()
      includedirs { include_path }
      externalincludedirs { "%{wks.location}/DotOther/NetCore" }
    end,
  
    defines = function()
      defines { "OE_MODULE" }
    end,
  
    windows_filters = function()
      filter { "configurations:Release" }
        postbuildcommands {
          '{COPY} "%{wks.location}externals/sdl2/lib/Release/SDL2.dll" "%{cfg.targetdir}"',
          '{COPY} "%{wks.location}externals/assimp/lib/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"',
          '{COPY} "%{wks.location}externals/steamworks/bin/steam_api64.dll" "%{cfg.targetdir}"',
        }
      filter { "configurations:Profile" }
        postbuildcommands {
          '{COPY} "%{wks.location}externals/sdl2/lib/Release/SDL2.dll" "%{cfg.targetdir}"',
          '{COPY} "%{wks.location}externals/assimp/lib/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"',
          '{COPY} "%{wks.location}externals/steamworks/bin/steam_api64.dll" "%{cfg.targetdir}"',
        }
      filter { "configurations:Debug" }
        postbuildcommands {
          '{COPY} "%{wks.location}externals/sdl2/lib/Debug/SDL2d.dll" "%{cfg.targetdir}"',
          '{COPY} "%{wks.location}externals/assimp/lib/Debug/assimp-vc143-mtd.dll" "%{cfg.targetdir}"',
          '{COPY} "%{wks.location}externals/steamworks/bin/steam_api64.dll" "%{cfg.targetdir}"',
        }
    end,

    components = {
      ["OtherEngine"] = "%{wks.location}/OtherEngine/src"
    }
  }
  return table
end

return M