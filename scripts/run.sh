#!/bin/bash
echo "Starting comparative simulation"

./scripts/run-linux_simulator.sh
./scripts/run-windows_simulator.sh

echo ""
echo "Simulation complete. Results overview:"
echo "--------------------------------------------------------------------------------"
docker exec -it os-simulator-container column -s, -t data/metrics.csv
echo "--------------------------------------------------------------------------------"
