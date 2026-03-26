workspace("OsSimulator")
architecture("x86_64")
startproject("LinuxSimulator")

configurations({
    "Debug",
    "Release",
})

targetdir("bin/%{cfg.buildcfg}-%{cfg.system}")
objdir("bin-int/%{cfg.buildcfg}-%{cfg.system}")

filter("configurations:Debug")
defines({ "DEBUG" })
symbols("On")

filter("system:windows")
systemversion("latest")
buildoptions({ "/std:c++latest", "/Zc:__cplusplus" })

filter("system:linux or system:macosx")
buildoptions({ "-std=c++23", "-Wall", "-Wextra" })

filter("{}")

-- Load our modularized Premake scripts
dofile("premake/actions.lua")
dofile("premake/simulators.lua")
dofile("premake/tests.lua")