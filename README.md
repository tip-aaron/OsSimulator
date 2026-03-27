# OS Simulator

## Naming Conventions

- `Namespaces` must be lowercase_letters
- `Classes` must be PascalCase
- `private` attributes must be prepended with `m` and have PascalCase,
- `function names` must begin with a verb and use camelCase
- `function arguments` must use camelCase
- `variable` names must use camelCase with optional camelCase_withUnderscore
- `pointers` must begin with `p` and the '*' be placed exactly before the variable name.
- `references` must begin with `r` and the '&' be placed exactly after the type name.
- `constants` must be in ALL_CAPS
- `file_names` must be in snake_case._

## Performance Metrics

## Setup

### Prerequisites

- Git
- Python 3.x (for running the setup scripts) and pip (Python package installer)
- C++ compiler (e.g., GCC, Clang, MSVC). You should have this already if you have an IDE like Visual Studio or CLion installed.
- Make or CMake (If you don't use an IDE like Visual Studio or CLion)
- An IDE or code editor of your choice (e.g., Visual Studio, Visual Studio Code, CLion)

### Getting the Project

To get started with this project, you can clone the repository using Git. If you don't have Git installed, you can download it from [git-scm.com](https://git-scm.com/).

1. Fork the repository to your own GitHub account (optional but recommended for contributing).
2. Clone the forked repository to your local machine.

```bash
git clone https://github.com/your-username/OsSimulator.git
```

### Initializing Development Environment

Below are instructions to setup this project on your local machine.
Please follow the steps carefully to ensure a successful setup.

1. Open **terminal** or **command prompt** and navigate to the root
    directory of this project.

    - Example:

        ```bash
        C:\Users\YourUsername>cd OsSimulator
        ```

2. Run the `generate` script to generate project files and set up the
    development environment.

    - On **Windows**:

        ```bash
        C:\Users\YourUsername\OsSimulator>scripts\generate.bat
        ```

    - On **Linux/macOS**:

        ```bash
        C:/Users/YourUsername/OsSimulator>./scripts/generate.sh
        ```

3. Follow the prompts in the script to select your desired build system
    and IDE.

4. After the script completes, open your chosen code editor or IDE and choose this
    project to start working on it.

    - If you chose **Visual Studio**, you can *click* on the generated `.sln` file to open the project right away.
    - If you chose **Visual Studio Code**, you can *open* the project folder and it should automatically detect the generated build files.
    - If you chose **CLion**, you can *open* the project folder and it should automatically detect the generated CMake files.

## Running the Application

- To run the application, build the project using your IDE or command line, and then execute the generated binary. The application will simulate an operating system environment, allowing you to test various features and functionalities.

- Visualizing saved data
  - The application saves data in a specific format (e.g., JSON, CSV, or binary). You can use tools like Excel, Python scripts, or custom visualization software to analyze and visualize this data.
  - For running our own visualizer, you can execute the `main.py` script in `visualizer/` using **Python**.

## Developing

- If you add new files, make sure to rerun `scripts/premake.bat` or `scripts/premake.sh` to update the project files, then recompile the project using your IDE or command line.
- For using `VsCode` with `Make`, you can run the following command in the terminal:

```bash
make LinuxSimulator WindowsSimulator
```

- To run the tests, you can run the scripts `scripts/test.bat` or `scripts/test.sh` depending on your operating system.

- If you add new folders, make sure to update `premake5.lua` to include those folders in the project files. For example, if you added a new folder in `Source` like a `lib` folder, you would need to go into `premake5.lua` and find something like this:

```lua
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
```

and add the new folder like this:

```lua
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
    "WORKLOADS.md",
    -- Added new folder
    "Source/lib/*.cpp",
    "Source/lib/*.hpp"
    "Source/lib/**/*.cpp",
    "Source/lib/**/*.hpp"
})
```

Make sure to add them in all projects `WindowsSimulator`, `LinuxSimulator`, `TestLinuxSimulator` and `TestWindowsSimulator` if you use them in all of them.

A project is defined in `premake5.lua` like this:

```lua
project("WindowsSimulator")
-- Project configuration for Windows Simulator
```
