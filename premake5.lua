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

project("WindowsSimulator")
kind("ConsoleApp")
language("C++")
cppdialect("C++23")

includedirs({
	"Include",
})

files({
	"Source/main.cpp",
	"Source/utils/**.cpp",
	"Source/utils/**.hpp",
	"Source/windows/**.cpp",
	"Source/windows/**.hpp",
})

project("LinuxSimulator")
kind("ConsoleApp")
language("C++")
cppdialect("C++23")

includedirs({
	"Include",
})

files({
	"Source/main.cpp",
	"Source/utils/**.cpp",
	"Source/utils/**.hpp",
	"Source/linux/**.cpp",
	"Source/linux/**.hpp",
})

project("GoogleTest")
kind("StaticLib")
language("C++")
cppdialect("C++23")

includedirs({
	"vendor/googletest/googletest/include",
	"vendor/googletest/googletest",
})

files({
	"vendor/googletest/googletest/src/gtest-all.cc",
})

filter("system:windows")
warnings("Off")
filter("system:linux or system:macosx")
buildoptions({ "-w" })
filter("{}")

project("TestWindowsSimulator")
kind("ConsoleApp")
language("C++")
cppdialect("C++23")

includedirs({
	"include",
	"vendor/googletest/googletest/include",
})

files({
	"Tests/**.cpp",
	"Source/utils/**.cpp",
	"Source/windows/**.cpp",
})

removefiles({ "Source/main.cpp" })

links({
	"GoogleTest",
})

filter("system:linux")
links({ "pthread" })
filter("{}")

project("TestLinuxSimulator")
kind("ConsoleApp")
language("C++")
cppdialect("C++23")

includedirs({
	"include",
	"vendor/googletest/googletest/include",
})

files({
	"Tests/**.cpp",
	"Source/utils/**.cpp",
	"Source/linux/**.cpp",
})

removefiles({ "Source/main.cpp" })

links({
	"GoogleTest",
})

filter("system:linux")
links({ "pthread" })
filter("{}")
