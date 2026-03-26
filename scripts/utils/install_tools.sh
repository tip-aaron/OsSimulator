#!/bin/bash

install() {
    echo ""
    echo "Installing necessary tools and dependencies..."
    echo "Checking system dependencies..."
    echo ""

    echo "Checking for Git..."
    if ! command -v git &> /dev/null; then
        echo "Git was not found. Attempting to install..."
        echo ""

        if command -v apt-get &> /dev/null; then
            # Debian, Ubuntu, Linux Mint, Pop!_OS
            sudo apt-get update && sudo apt-get install -y git
        elif command -v dnf &> /dev/null; then
            # Fedora, RHEL 8+, AlmaLinux, Rocky Linux
            sudo dnf install -y git
        elif command -v pacman &> /dev/null; then
            # Arch Linux, Manjaro, EndeavourOS
            sudo pacman -Sy --noconfirm git
        elif command -v zypper &> /dev/null; then
            # openSUSE, SUSE Linux Enterprise
            sudo zypper install -y git
        elif command -v yum &> /dev/null; then
            # Older CentOS / RHEL 7
            sudo yum install -y git
        elif command -v apk &> /dev/null; then
            # Alpine Linux
            sudo apk add git
        elif [[ "$OSTYPE" == "darwin"* ]] && command -v brew &> /dev/null; then
            # macOS (requires Homebrew)
            brew install git
        elif command -v pkg &> /dev/null; then
            # FreeBSD or Termux (Android)
            sudo pkg install -y git
        else
            echo "Error: Unsupported package manager."
        fi

        # Verify if Git was actually installed
        if command -v git &> /dev/null; then
            echo "Git installed successfully!"
        else
            echo "Error: Failed to install Git automatically. Please install it manually."
        fi
    else
        echo "Git is already installed."
    fi

    echo ""
    echo "Checking for Python 3..."
    if ! command -v python3 &> /dev/null; then
        echo "Python 3 was not found. Attempting to install..."
        echo ""

        if command -v apt-get &> /dev/null; then
            # Debian, Ubuntu, Linux Mint, Pop!_OS
            sudo apt-get update && sudo apt-get install -y python3 python3-venv python3-pip
        elif command -v dnf &> /dev/null; then
            # Fedora, RHEL 8+, AlmaLinux, Rocky Linux
            sudo dnf install -y python3 python3-pip
        elif command -v pacman &> /dev/null; then
            # Arch Linux, Manjaro, EndeavourOS
            sudo pacman -Sy --noconfirm python python-pip
        elif command -v zypper &> /dev/null; then
            # openSUSE, SUSE Linux Enterprise
            sudo zypper install -y python3 python3-pip
        elif command -v yum &> /dev/null; then
            # Older CentOS / RHEL 7
            sudo yum install -y python3 python3-pip
        elif command -v apk &> /dev/null; then
            # Alpine Linux
            sudo apk add python3 py3-pip
        elif [[ "$OSTYPE" == "darwin"* ]] && command -v brew &> /dev/null; then
            # macOS (requires Homebrew)
            brew install python3
        elif command -v pkg &> /dev/null; then
            # FreeBSD or Termux (Android)
            sudo pkg install -y python3
        else
            echo "Error: Could not find a supported package manager."
        fi

        # Verify if Python 3 was actually installed
        if command -v python3 &> /dev/null; then
            echo "Python 3 installed successfully!"
        else
            echo "Error: Failed to install Python 3. Please install it manually from https://www.python.org/"
        fi
    else
        echo "Python 3 is already installed."

        # Debian/Ubuntu quirk: Python exists, but venv/pip might be missing
        if command -v apt-get &> /dev/null; then
            if ! dpkg -s python3-venv &> /dev/null || ! dpkg -s python3-pip &> /dev/null; then
                echo "Missing python3-venv or python3-pip on Debian-based system. Installing..."
                echo ""

                if sudo apt-get update && sudo apt-get install -y python3-venv python3-pip; then
                    echo "Python 3 modules installed successfully!"
                else
                    echo "Error: Failed to install python3-venv or python3-pip."
                fi
            fi
        fi
    fi

    echo ""
    echo "Checking for pip..."
    if ! command -v pip &> /dev/null && ! command -v pip3 &> /dev/null; then
        echo "Pip was not found. Attempting to bootstrap pip..."

        if python3 -m ensurepip --default-pip; then
            echo "Pip installed successfully!"
        else
            echo "Error: Failed to install pip automatically."
        fi
    else
        echo "Pip is already installed."
    fi

    echo ""
    echo "All necessary tools and dependencies are installed!"
}

install