#!/bin/bash

get_premake_path() {
    local vendor_path="./vendor"
    local os_name="${OSTYPE}"

    if [ -z "$os_name" ]; then
        os_name=$(uname -s | tr '[:upper:]' '[:lower:]')
    fi

    case "$os_name" in
        *"linux"*)
            echo "$vendor_path/premake5-linux"
            ;;
        *"darwin"*)
            echo "$vendor_path/premake5-mac"
            ;;
        *"msys"* | *"cygwin"* | *"mingw"*)
            echo "WINDOWS_ERROR"
            ;;
        *)
            echo "UNSUPPORTED_OS_ERROR"
            ;;
    esac
}

# If the script is run directly (not sourced), it prints the path
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    get_premake_path
fi
