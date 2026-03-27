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

filter("action:vs* or toolset:msc")
    buildoptions({ "/Zc:__cplusplus" })

filter("toolset:gcc or toolset:clang")
    buildoptions({ "-Wall", "-Wextra" })
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
    "Source/*.hpp",
    "Source/*.cpp",
    "Source/windows/*.cpp",
    "Source/windows/*.hpp",
    "Source/utils/*.cpp",
    "Source/utils/*.hpp",
    "Include/**/*.hpp",
    "README.md",
    "TODO.md",
    "WORKLOADS.md"
})

project("LinuxSimulator")
kind("ConsoleApp")
language("C++")
cppdialect("C++23")

includedirs({
    "Include",
})

files({
    "Source/*.hpp",
    "Source/*.cpp",
    "Source/linux/*.cpp",
    "Source/linux/*.hpp",
    "Source/utils/*.cpp",
    "Source/utils/*.hpp",
    "Include/**/*.hpp",
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
    "Source",
    "Source/windows",
    "Source/utils",
    "Include",
    "vendor/googletest/googletest/include",
})


files({
    "Tests/main.cpp",
    "Tests/*.cpp",
    "Tests/*.hpp",
    "Tests/windows/*.cpp",
    "Tests/windows/*.hpp",
    "Tests/windows/**/*.cpp",
    "Tests/windows/**/*.hpp",
    "Source/windows/*.cpp",
    "Source/windows/*.hpp",
    "Source/utils/*.cpp",
    "Source/utils/*.hpp",
    "Include/**/*.hpp"
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
    "Source",
    "Source/linux",
    "Source/utils",
    "Include",
    "vendor/googletest/googletest/include",
})


files({
    "Tests/main.cpp",
    "Tests/*.cpp",
    "Tests/*.hpp",
    "Tests/linux/*.cpp",
    "Tests/linux/*.hpp",
    "Tests/linux/**/*.cpp",
    "Tests/linux/**/*.hpp",
    "Source/linux/*.cpp",
    "Source/linux/*.hpp",
    "Source/utils/*.cpp",
    "Source/utils/*.hpp",
    "Include/**/*.hpp"
})

removefiles({ "Source/main.cpp" })

links({
    "GoogleTest",
})

filter("system:linux")
links({ "pthread" })
filter("{}")
