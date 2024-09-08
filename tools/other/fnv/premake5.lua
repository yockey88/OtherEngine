local fnv = {
  name = "fnv" ,
  path = "./fnv" ,
  kind = "ConsoleApp" ,
  language = "C++" ,
  cppdialect = "C++latest" ,

  files = function()
    files {
      "./fnv.cpp" ,
    }
  end ,

  components = {
    ["OtherEngine"] = "%{wks.location}/OtherEngine/src" ,
  } ,
}

AddProject(fnv)