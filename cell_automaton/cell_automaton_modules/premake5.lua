
local cell_automaton_scripts = {
  name = "CellAutomatonScripts" ,
  path = "./cell_automaton" ,
  kind = "SharedLib" ,
  language = "C#" ,
  dotnetframework = "net8.0" ,

  files = function()
    files {
      "./Source/**.cs" ,
    }
  end,

  components = {
    ["OtherEngine-CsCore"] = ""
  }
}
AddModule(cell_automaton_scripts)