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