local OE_ScriptCore = {}

OE_ScriptCore.name = "OtherEngine-CsCore"
OE_ScriptCore.kind = "SharedLib"
OE_ScriptCore.language = "C#"
OE_ScriptCore.dotnetframework = "4.7.2"

OE_ScriptCore.files = function()
  files {
    "./Source/**.cs" ,
    "./Properties/**.cs"
  }
end

OE_ScriptCore.defines = function()
  defines {
    "OE_MODULE" ,
  }
end

AddModule(OE_ScriptCore)