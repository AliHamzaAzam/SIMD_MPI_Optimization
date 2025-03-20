import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Load the data from the CSV
df = pd.read_csv('../results/simd_avg_results.csv')
time_column = 'Avg_Time_Nanoseconds'

# Group by implementation and matrix size, then compute the mean
grouped_df = df.groupby(['Implementation', 'Matrix_Size'])[time_column].mean().reset_index()

# Convert to a pivot table for easier plotting (rows: matrix sizes, columns: implementations)
pivot_df = grouped_df.pivot(index='Matrix_Size', columns='Implementation', values=time_column)


plt.figure(figsize=(12, 8))
pastel_colors = sns.color_palette("pastel")

# Plot each implementation
for i, implementation in enumerate(pivot_df.columns):
    plt.plot(pivot_df.index, pivot_df[implementation],
             marker='o', linestyle='-', linewidth=2.5,
             markersize=8, label=implementation,
             color=pastel_colors[i])

# Use log scale for both axes to better visualize the performance differences
plt.xscale('log', base=2)
plt.yscale('log')

# Add grid, labels, and title
plt.grid(True, which='both', linestyle='--', linewidth=0.5, alpha=0.7)
plt.title("Matrix Multiplication Performance Comparison", pad=20, fontsize=18, fontweight='bold')
plt.xlabel("Matrix Size", labelpad=15, fontsize=14, fontweight='bold')
plt.ylabel("Execution Time (nanoseconds)", labelpad=15, fontsize=14, fontweight='bold')
plt.xticks(pivot_df.index, labels=[str(size) for size in pivot_df.index], fontsize=12)
plt.yticks(fontsize=12)
legend = plt.legend(title="Implementation", fontsize=12, title_fontsize=13,
                    bbox_to_anchor=(1.05, 1), loc='upper left', borderaxespad=0.)
legend.get_title().set_fontweight('bold')

# Annotate the fastest implementation for the largest matrix size
largest_size = pivot_df.index.max()
fastest_impl = pivot_df.loc[largest_size].idxmin()
fastest_time = pivot_df.loc[largest_size, fastest_impl]
plt.annotate(f'Fastest at size {largest_size}: {fastest_impl} ({fastest_time:.2e}ns)',
             xy=(0.05, 0.95), xycoords='axes fraction',
             fontsize=14, color='darkgreen', fontweight='bold',
             bbox=dict(facecolor='white', alpha=0.8, edgecolor='darkgreen', boxstyle='round,pad=0.5'))

plt.tight_layout()
plt.savefig("../results/matrix_performance_comparison.png", dpi=300, bbox_inches='tight')
plt.show()

# ---------------------------
# Second Plot: Speedup Comparison
# ---------------------------
# Choose a baseline implementation; here we use "scalar2D" as the baseline.
baseline_impl = "scalar2D"
if baseline_impl not in pivot_df.columns:
    raise ValueError(f"Baseline implementation '{baseline_impl}' not found in data.")

# Compute speedup relative to the baseline:
# Speedup = (baseline execution time) / (implementation execution time)
speedup_df = pivot_df.copy()
for impl in pivot_df.columns:
    speedup_df[impl] = pivot_df[baseline_impl] / pivot_df[impl]

# Plot the speedup
plt.figure(figsize=(12, 8))
for i, implementation in enumerate(speedup_df.columns):
    plt.plot(speedup_df.index, speedup_df[implementation],
             marker='o', linestyle='-', linewidth=2.5,
             markersize=8, label=implementation,
             color=pastel_colors[i])

# Use log scale for the x-axis
plt.xscale('log', base=2)
plt.xlabel("Matrix Size", labelpad=15, fontsize=14, fontweight='bold')
plt.ylabel(f"Speedup (relative to {baseline_impl})", labelpad=15, fontsize=14, fontweight='bold')
plt.title("Matrix Multiplication Speedup Comparison", pad=20, fontsize=18, fontweight='bold')
plt.grid(True, which='both', linestyle='--', linewidth=0.5, alpha=0.7)
plt.xticks(speedup_df.index, labels=[str(size) for size in speedup_df.index], fontsize=12)
plt.yticks(fontsize=12)
legend = plt.legend(title="Implementation", fontsize=12, title_fontsize=13,
                    bbox_to_anchor=(1.05, 1), loc='upper left', borderaxespad=0.)
legend.get_title().set_fontweight('bold')

plt.tight_layout()
plt.savefig("../results/matrix_speedup_comparison.png", dpi=300, bbox_inches='tight')
plt.show()
