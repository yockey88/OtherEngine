local pybind = {
  name = "pybind11",
  kind = "StaticLib",
  language = "C++",

  files = function()
    files {
      "./pybind11.cpp"
    }
  end,

  include_dirs = function()
    includedirs { 
      "." 
    }
  end,
}

AddExternalProject(pybind)