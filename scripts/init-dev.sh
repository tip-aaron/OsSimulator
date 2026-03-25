#!/bin/bash
set -e

echo "Initializing development environment"

echo "Generating project environment"
docker exec -it os-simulator-container ./vendor/premake5-linux gmake2

echo "compiling initial release build"
docker exec -it os-simulator-container make config=release -j

echo "Environment ready! Scripts: run, test, and visualize can now be run."

