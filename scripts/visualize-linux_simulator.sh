#!/bin/bash

echo "Generating Linux-only graphs"
docker exec -it os-simulator-container python visualizer/main.py --target linux
