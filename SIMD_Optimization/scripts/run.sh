#!/bin/bash

# Compile the code
clang++ -std=c++17 -O3 -o simd ../src/simd.cpp -framework CoreServices

# Create results directory
mkdir -p ../results

# Create results files
raw_results_file="../results/simd_raw_results.csv"
avg_results_file="../results/simd_avg_results.csv"

# Initialize results files with headers
echo "Implementation,Matrix_Size,Time_Nanoseconds" > $raw_results_file
echo "Implementation,Matrix_Size,Avg_Time_Nanoseconds" > $avg_results_file

# Test configurations
implementations=("neon" "apple" "scalar1D" "scalar2D")
sizes=(4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384)
repetitions=10

# Run tests
for size in "${sizes[@]}"; do
    echo "Testing with matrix size $size..."

    for impl in "${implementations[@]}"; do
        echo "  Implementation: $impl"
        total_time=0

        # Run multiple times and save the raw results
        for (( i=1; i<=$repetitions; i++ )); do
            echo "    Run $i/$repetitions"

            # Capture the output of the command
            result=$(./simd $size $impl)

            # Save raw result
            echo "$result" >> $raw_results_file

            # Extract time value for averaging
            time_value=$(echo "$result" | cut -d',' -f3)
            total_time=$(echo "$total_time + $time_value" | bc)
        done

        # Calculate average and save it
        avg_time=$(echo "scale=3; $total_time / $repetitions" | bc)
        echo "$impl,$size,$avg_time" >> $avg_results_file
    done
done

# Remove the compiled code
rm simd

echo "Tests completed."
echo "Raw results saved to $raw_results_file"
echo "Average results saved to $avg_results_file"