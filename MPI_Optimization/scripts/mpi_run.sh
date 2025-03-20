#!/bin/bash
# This script times the MPI executable "mpi" for each .tfa file and for different numbers of processes.
# It writes the filename, number of processes, and runtime to mpi_times.csv

OUTPUT_FILE="mpi_times.csv"
echo "filename,np,time" > "$OUTPUT_FILE"

# Define the desired numbers of processes (adjust NP_LIST as needed)
NP_LIST=(1 2 4 8 16)

for file in *.tfa; do
    for np in "${NP_LIST[@]}"; do
        echo "Running mpirun -np $np on $file"
        # Use /usr/bin/time to measure elapsed time; note that output is captured from stderr.
        runtime=$(/usr/bin/time -f "%e" mpirun -np "$np" -hostfile ~/hostfile ./mpi "$file" 2>&1 1>/dev/null)
        echo "$file,$np,$runtime" >> "$OUTPUT_FILE"
    done
done

echo "Results saved in $OUTPUT_FILE"
