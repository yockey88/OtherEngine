local lua = {}

lua.name = "lua"
lua.kind = "StaticLib"
lua.language = "C"

lua.files = function()
  files { "./lua/**.c" , "./lua/**.h" }
end

lua.include_dirs = function()
  includedirs { "." }
end

AddExternalProject(lua)

local lua_lang_module = {}
lua_lang_module.name = "LuaModule"
lua_lang_module.kind = "SharedLib"
lua_lang_module.language = "C++"
lua_lang_module.cppdialect = "C++latest"

lua_lang_module.files = function()
  files {
    "./src/**.hpp", "./src/**.cpp",
  }
end

lua_lang_module.include_dirs = function()
  includedirs {
    "./src",
  }
end

lua_lang_module.defines = function()
  defines {
    "OE_MODULE",
  }
end

lua_lang_module.components = {}
lua_lang_module.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"

local lua_lib = {}
lua_lib.name = "lua"
lua_lib.path = "./OtherEngine-Components/language-modules/lua"
lua_lib.include_dir = "%{wks.location}/OtherEngine-Components/language-modules/lua"
lua_lib.lib_name = "lua"
lua_lib.lib_dir = "%{wks.location}/bin/Debug/lua"

lua_lang_module.extra_dependencies = function(configuration)
  ProcessDependency(configuration , lua_lib)
end

AddProject(lua_lang_module)
