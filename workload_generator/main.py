"""
Command-line entry point for generating OS simulation workloads.
"""

from workloads import gen_interactive, gen_background, gen_mixed_interactive_and_background

def main():
    """
    Executes the generation of all three required academic workloads.
    """
    number_of_processes = 1000

    print("Generating Scenario A (Interactive Workload)...")
    gen_interactive(number_of_processes)

    print("Generating Scenario B (Background/Data Workload)...")
    gen_background(number_of_processes)

    print("Generating Scenario C (Mixed Workload)...")
    gen_mixed_interactive_and_background(number_of_processes)

    print("\nAll workloads and reference traces successfully generated in their respective directories.")

if __name__ == "__main__":
    main()