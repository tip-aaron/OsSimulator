"""
Command-line entry point for generating OS simulation workloads.
"""

import argparse
from workloads_engine import gen_interactive, gen_background, gen_mixed_interactive_and_background

def main():
    """
    Executes the generation of all three required academic workloads based on the specified mode.
    """
    # Set up the argument parser
    parser = argparse.ArgumentParser(description="Generate OS simulation workloads.")
    parser.add_argument(
        '--mode',
        type=str,
        choices=['read', 'write'],
        default='read',
        help="Specify the workload access pattern: 'read' (read-heavy) or 'write' (write-heavy)."
    )

    args = parser.parse_args()

    # Translate the command-line string into our boolean flag
    is_read_heavy = (args.mode == 'read')
    mode_label = "Read-Heavy" if is_read_heavy else "Write-Heavy"

    number_of_processes = 10000

    print(f"--- Generating Workloads in {mode_label} Mode ---\n")

    print(f"Generating Scenario A (Interactive Workload - {mode_label})...")
    gen_interactive(number_of_processes, is_read_heavy=is_read_heavy)

    print(f"Generating Scenario B (Background/Data Workload - {mode_label})...")
    gen_background(number_of_processes, is_read_heavy=is_read_heavy)

    print(f"Generating Scenario C (Mixed Workload - {mode_label})...")
    gen_mixed_interactive_and_background(number_of_processes, is_read_heavy=is_read_heavy)

    print("\nAll workloads and reference traces successfully generated in their respective directories.")

if __name__ == "__main__":
    main()