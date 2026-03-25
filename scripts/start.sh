#!/bin/bash

echo "Initializing OS Simulator Development Environment"

echo "Updating submodules"
git submodule update --init --recursive

echo "Starting Docker container"
docker-compose up -d

