local lua_lang_module = {}
lua_lang_module.name = "LuaModule"
lua_lang_module.kind = "SharedLib"
lua_lang_module.language = "C++"
lua_lang_module.cppdialect = "C++latest"

lua_lang_module.files = function()
  files {
    "./src/**.hpp", "./src/**.cpp",
    "./sol2/lua/**.c", "./sol2/lua/**.h",
    "./sol2/sol/**.hpp"
  }
end

lua_lang_module.include_dirs = function()
  includedirs {
    "./src",
    "./sol2",
    "./sol2/lua",
  }
end

lua_lang_module.defines = function()
  defines {
    "OE_MODULE",
  }
end

lua_lang_module.components = {}
lua_lang_module.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"

AddProject(lua_lang_module)
