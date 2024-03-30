local cs_lang_module = {}
cs_lang_module.name = "CsModule"
cs_lang_module.kind = "SharedLib"
cs_lang_module.language = "C++"
cs_lang_module.cppdialect = "C++latest"

cs_lang_module.files = function()
  files {
    "./src/**.hpp", "./src/**.cpp",
  }
end

cs_lang_module.include_dirs = function()
  includedirs {
    "./src",
  }
end

cs_lang_module.defines = function()
  defines {
    "OE_MODULE",
  }
end

cs_lang_module.components = {}
cs_lang_module.components["OtherEngine"] = "%{wks.location}/OtherEngine/src"

local mono = {}
mono.name = "mono"
mono.include_dir = "%{wks.location}/OtherEngine-Components/language-modules/cs/mono/include"
mono.lib_dir = "%{wks.location}/OtherEngine-Components/language-modules/cs/mono/lib"
mono.lib_name = "mono-2.0-sgen"

cs_lang_module.extra_dependencies = function(configuration)
  ProcessDependency(configuration , mono)
end

cs_lang_module.post_build_commands = function()
  filter { "system:windows" , "configurations:Debug" }
    postbuildcommands {
      '{COPY} %{wks.location}/OtherEngine-Components/language-modules/cs/mono/bin/mono-2.0-sgen.dll "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/OtherEngine-Components/language-modules/cs/mono/bin/mono-2.0-sgen.pdb "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/OtherEngine-Components/language-modules/cs/mono/bin/MonoPosixHelper.dll "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/OtherEngine-Components/language-modules/cs/mono/bin/MonoPosixHelper.pdb "%{cfg.targetdir}"',
    }

  filter { "system:windows" , "configurations:Release" }
    postbuildcommands {
      '{COPY} %{wks.location}/OtherEngine-Components/language-modules/cs/mono/bin/mono-2.0-sgen.dll "%{cfg.targetdir}"',
      '{COPY} %{wks.location}/OtherEngine-Components/language-modules/cs/mono/bin/MonoPosixHelper.dll "%{cfg.targetdir}"',
    }
end

AddProject(cs_lang_module)

local OE_ScriptCore = {}

OE_ScriptCore.name = "OtherEngine-CsCore"
OE_ScriptCore.kind = "SharedLib"
OE_ScriptCore.language = "C#"

OE_ScriptCore.files = function()
  files {
    "./core/Source/**.cs" ,
    "./core/Properties/**.cs"
  }
end

AddModule(OE_ScriptCore)
