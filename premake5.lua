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
newaction {
    trigger     = "clean_build",
    description = "Cleans all build artifacts (object files, binaries, etc.) and cache but keeps generated project files",
    execute     = function ()
        print("Cleaning build directories...")

        local dirs = {
            "bin", "bin-int", "build", "obj", ".vs", "*.xcodeproj", "*.xcworkspace"
        }

        for _, dir in ipairs(dirs) do
            for _, d in ipairs(os.matchdirs(dir)) do
                os.rmdir(d)
            end
            os.rmdir(dir)
        end

        print("Build artifacts cleaned!")
    end
}

newaction {
    trigger     = "clean_project",
    description = "Cleans absolutely all Premake-generated files and build artifacts",
    execute     = function ()
        print("Cleaning build and cache directories...")

        local dirs = {
            "bin", "bin-int", "build", "obj", ".vs", "*.xcodeproj", "*.xcworkspace"
        }

        for _, dir in ipairs(dirs) do
            for _, d in ipairs(os.matchdirs(dir)) do
                os.rmdir(d)
            end
            os.rmdir(dir)
        end

        print("Cleaning generated project files...")

        local files = {
            "*.sln", "*.vcxproj", "*.vcxproj.*", "Makefile", "*.make",
            "*.workspace", "*.project", "*.cbp", "compile_commands.json"
        }

        for _, pattern in ipairs(files) do
            for _, file in ipairs(os.matchfiles(pattern)) do
                os.remove(file)
            end
        end

        print("All Premake generation cleaned!")
    end
}

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
    "Include/**.hpp",
    "README.md",
    "TODO.md",
    "WORKLOADS.md"
})

vpaths {
    ["Source"] = "Source/**.cpp",
    ["Include"] = "Include/**.hpp",
    ["Documentation"] = { "README.md", "TODO.md" },
}

project("LinuxSimulator")
kind("ConsoleApp")
language("C++")
cppdialect("C++23")

includedirs({
    "Include",
})

files({
    "Source/main.cpp",
    "Source/**.hpp",
    "Source/utils/**.cpp",
    "Source/utils/**.hpp",
    "Source/linux/**.cpp",
    "Source/linux/**.hpp",
    "Include/**.hpp",
    "README.md",
    "TODO.md",
    "WORKLOADS.md"
})

group("")

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
    "Tests/**.hpp",
    "Source/**.hpp",
    "Source/utils/**.cpp",
    "Source/utils/**.hpp",
    "Source/windows/**.cpp",
    "Source/windows/**.hpp",
    "Include/**.hpp",
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
    "Tests/**.hpp",
    "Source/**.hpp",
    "Source/utils/**.cpp",
    "Source/utils/**.hpp",
    "Source/linux/**.cpp",
    "Source/linux/**.hpp",
    "Include/**.hpp",
})

removefiles({ "Source/main.cpp" })

links({
    "GoogleTest",
})

filter("system:linux")
links({ "pthread" })
filter("{}")
