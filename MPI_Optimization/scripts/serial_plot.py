import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Load the serial performance data from CSV.
# Expected columns: 'filename', 'time' (in nanoseconds)
df = pd.read_csv('../results/serial_times.csv')

# If there are multiple runs per file, group by filename and compute the mean time.
grouped_df = df.groupby('filename')['time'].mean().reset_index()

plt.figure(figsize=(12, 8))
pastel_colors = sns.color_palette("pastel")

# Plot serial execution times. Since filenames are categorical, we'll use a line plot with markers.
plt.plot(grouped_df['filename'], grouped_df['time'],
         marker='o', linestyle='-', linewidth=2.5,
         markersize=8, color=pastel_colors[0], label='Serial')

plt.grid(True, which='both', linestyle='--', linewidth=0.5, alpha=0.7)
plt.title("Serial Execution Time per Dataset", pad=20, fontsize=18, fontweight='bold')
plt.xlabel("Dataset (Filename)", labelpad=15, fontsize=14, fontweight='bold')
plt.ylabel("Execution Time (nanoseconds)", labelpad=15, fontsize=14, fontweight='bold')
plt.xticks(rotation=45, ha='right', fontsize=12)
plt.yticks(fontsize=12)

plt.legend(title="Execution", fontsize=12, title_fontsize=13, loc='upper right', borderaxespad=0.)
plt.tight_layout()
plt.savefig("../results/serial_performance.png", dpi=300, bbox_inches='tight')
plt.show()
