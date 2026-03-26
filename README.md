# OS Simulator

## Performance Metrics

## Setup

Only Git is needed at first as the scripts in this project will automatically download and set up any additional tools and dependencies required for development.

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
    and IDE. The script will automatically download and set up necessary tools and dependencies.

4. After the script completes, open your chosen code editor or IDE and choose this
    project to start working on it.

    - If you chose **Visual Studio**, you can *click* on the generated `.sln` file to open the project right away.
    - If you chose **Visual Studio Code**, you can *open* the project folder and it should automatically detect the generated build files.
    - If you chose **CLion**, you can *open* the project folder and it should automatically detect the generated CMake files.
