local game2025 = OtherBuildTools.project_table("game2025", "./game2025")
game2025.custom_configurations = function()
  filter "configurations:Profile"
  runtime "Release"
  optimize "Full"
  symbols "On"
  
  defines { 
    "OTHER_PROFILE_BUILD",
    "TRACY_ENABLE",
    "TRACY_ON_DEMAND",
    "TRACY_CALLSTACK=10", 
  }

  files { 
    "%{wks.location}/externals/tracy/TracyClient.cpp",
  }
  includedirs { "%{wks.location}/externals/tracy" }
  
  ProcessDependencies("Release")
end

AddProject(game2025)