#!/bin/bash

echo "Generating Windows-only graphs"
docker exec -it os-simulator-container python visualizer/main.py --target windows
