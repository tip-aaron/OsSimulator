group("Core")

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

group("")