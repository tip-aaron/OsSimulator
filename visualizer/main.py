import argparse
import pandas as pd
import matplotlib.pyplot as plt
import os

def plot_linux_metrics(df):
    # Filter for Linux data and plot...
    # plt.savefig('data/linux_metrics.png')

def plot_windows_metrics(df):
    # Filter for Windows data and plot...
    # plt.savefig('data/windows_metrics.png')

def plot_comparison(df):
    # Group by OS and plot them side-by-side
    # Example: df.groupby('OS')['TurnaroundTime'].mean().plot(kind='bar')
    # plt.savefig('data/os_comparison.png')

def main():
    parser = argparse.ArgumentParser(description="OS Simulator Visualizer")
    parser.add_argument(
        '--target', 
        type=str, 
        choices=['linux', 'windows', 'compare'], 
        required=True, 
        help="Specify which graphs to generate"
    )
    
    args = parser.parse_args()

    csv_path = 'data/metrics.csv'
    if not os.path.exists(csv_path):
        print(f"❌ Error: {csv_path} not found. Run the C++ simulation first.")
        return

    df = pd.read_csv(csv_path)

    if args.target == 'linux':
        plot_linux_metrics(df)
    elif args.target == 'windows':
        plot_windows_metrics(df)
    elif args.target == 'compare':
        plot_comparison(df)

if __name__ == "__main__":
    main()
