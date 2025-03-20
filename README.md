# Computational Optimization Project

This project demonstrates optimization techniques for computationally intensive tasks using SIMD (Single Instruction Multiple Data) and MPI (Message Passing Interface) parallelization. The project consists of two main components:

1. **SIMD Optimization** - Matrix multiplication optimization using SIMD instructions
2. **MPI Optimization** - Progressive sequence alignment algorithm using MPI for distributed computing

## Table of Contents

- [Requirements](#requirements)
- [SIMD Optimization](#simd-optimization)
- [MPI Optimization](#mpi-optimization)
- [Benchmarking](#benchmarking)
- [Results Visualization](#results-visualization)

## Requirements

- C++ compiler with C++17 support
- SIMD support (ARM NEON or Apple-specific optimizations)
- MPI library (for MPI component)
- Python 3.x with pandas, matplotlib, and seaborn (for visualization)
- FFTW library (for FFT computations)

## SIMD Optimization

This component implements and benchmarks different matrix multiplication algorithms:

- `scalar1D` - Basic single-threaded implementation with 1D memory layout
- `scalar2D` - Basic single-threaded implementation with 2D memory layout
- `neon` - SIMD-optimized implementation using ARM NEON instructions
- `apple` - Apple-specific optimized implementation

### Usage

```bash
cd SIMD_Optimization/scripts
./run.sh
```

This script will:
1. Compile the code
2. Run benchmarks for various matrix sizes (4 to 16384)
3. Generate raw and average results in CSV format
4. Clean up temporary files

## MPI Optimization

This component implements a progressive sequence alignment algorithm used in bioinformatics:

- Reads sequences from FASTA files
- Computes a distance matrix using FFT-based correlation
- Builds a guide tree for alignment
- Performs progressive alignment

### Usage

#### Serial Version
```bash
cd MPI_Optimization/scripts
./serial_run.sh
```

#### MPI Version
```bash
cd MPI_Optimization/scripts
./mpi_run.sh
```

## Benchmarking

The project includes comprehensive benchmarking scripts that:

1. Measure execution time across different implementations
2. Test with varying problem sizes (matrix dimensions or sequence lengths)
3. Compare performance across different numbers of processes (for MPI)
4. Save results to CSV files for analysis

## Results Visualization

Python scripts in each component's `scripts` directory generate visualizations:

### SIMD Visualization

```bash
cd SIMD_Optimization/scripts
python plot.py
```

Generates:
- Performance comparison plots with execution time vs. matrix size
- Speedup comparison relative to baseline implementation

### MPI Visualization

```bash
cd MPI_Optimization/scripts
python serial_plot.py  # For serial performance
python mpi_plot.py     # For MPI scaling performance
```

Generates:
- Serial execution time per dataset
- MPI scaling performance across different numbers of processes

## Implementation Details

- The SIMD component uses ARM NEON and Apple SIMD intrinsics for vectorized operations
- The MPI component distributes computation across multiple processes
- Both components include careful performance measurement and analysis tools

