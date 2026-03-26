group("Dependencies")

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

group("Tests")

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