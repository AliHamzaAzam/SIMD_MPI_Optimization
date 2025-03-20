import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Load the MPI performance data from CSV.
# Expected columns: 'filename', 'np' (number of processes), 'time' (in nanoseconds)
df = pd.read_csv('../results/mpi_times.csv')

# Group by number of processes and filename, then compute the mean execution time.
grouped_df = df.groupby(['np', 'filename'])['time'].mean().reset_index()

# Create a pivot table with np as index and each dataset (filename) as separate columns.
pivot_df = grouped_df.pivot(index='np', columns='filename', values='time')

plt.figure(figsize=(12, 8))
pastel_colors = sns.color_palette("pastel")

# Plot each dataset's MPI execution time as a function of the number of processes.
for i, dataset in enumerate(pivot_df.columns):
    plt.plot(pivot_df.index, pivot_df[dataset],
             marker='o', linestyle='-', linewidth=2.5,
             markersize=8, label=dataset,
             color=pastel_colors[i % len(pastel_colors)])

# Use log scale for x-axis assuming the number of processes are powers of two,
# and log scale for y-axis if there is a large variation in execution times.
plt.xscale('log', base=2)
plt.yscale('log')

plt.xlabel("Number of Processes (np)", labelpad=15, fontsize=14, fontweight='bold')
plt.ylabel("Execution Time (nanoseconds)", labelpad=15, fontsize=14, fontweight='bold')
plt.title("MPI Execution Time Comparison", pad=20, fontsize=18, fontweight='bold')
plt.grid(True, which='both', linestyle='--', linewidth=0.5, alpha=0.7)

# Format x-axis tick labels.
plt.xticks(pivot_df.index, labels=[str(int(x)) for x in pivot_df.index], fontsize=12)
plt.yticks(fontsize=12)

legend = plt.legend(title="Dataset", fontsize=12, title_fontsize=13,
                    loc='upper right', borderaxespad=0.)
legend.get_title().set_fontweight('bold')

plt.tight_layout()
plt.savefig("../results/mpi_performance.png", dpi=300, bbox_inches='tight')
plt.show()
