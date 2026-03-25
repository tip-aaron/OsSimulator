#!/bin/bash

echo "Generating Comparative Analysis graphs"
docker exec -it os-simulator-container python visualizer/main.py --target compare
