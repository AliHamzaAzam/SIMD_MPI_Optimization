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


## Results site

`web/` is a light, responsive Vite site using vanilla TypeScript. It presents a small
recorded serial benchmark, source-level explanations and the limits of the data.
It does not claim measured MPI or SIMD acceleration.

Requirements: Node.js 20.19+ or 22.12+, npm. The full test suite and measurement
capture also require Python 3 and an existing C++17 `g++` compiler. The benchmark capture script targets
Linux and reads `/proc/cpuinfo`; run it only on a Linux machine with that file
available. The static site itself is portable and uses no production services.

```bash
cd web
npm ci --cache /tmp/results-site-npm-cache
npm run data       # Regenerate data/results.json from checked-in raw outputs
npm run typecheck
npm test
npm run build
npm run dev        # Open the local URL printed by Vite
# Or serve the production build:
npm run preview
```

To replace the measurement record on your machine:

```bash
cd web
npm run benchmark
npm test
npm run build
```

The benchmark compiles the unchanged serial source with `g++ -std=c++17 -O2`,
generates four synthetic protein sequences at each of 64, 256 and 512 residues,
and records five full-process wall-clock samples after one excluded warm-up per
input. It passes only a FASTA filename so the alignment stage is included. The
record includes process startup, I/O and output. No claim of biological validity
is made for the synthetic fixture. `web/raw/` retains timings, stdout, stderr,
FASTA input, compiler/machine information and source/input SHA256 hashes.
`web/data/results.json` is deterministic and generated; edit the raw evidence
or rerun the benchmark instead of editing aggregate values.

All displayed datasets use one source revision, compiler and machine. They differ
in sequence length and are not a speedup comparison. Short runs are sensitive to
launch overhead and cloud scheduling. MPI headers were unavailable in the cloud
environment despite its compiler wrapper. SIMD requires Apple/ARM headers. MPI
speedup and efficiency are explicitly unavailable. In addition, serial and MPI
use different alphabet-to-number mappings, so a valid cross-implementation
comparison needs numerical validation. The SIMD expression is
`C[i,j] = A[j,i] * B[i,j]`, transposed elementwise multiplication, rather than a
general matrix product. Only the MPI sequence FFT stage is distributed; distance
calculation and progressive alignment remain on rank 0.

For the served browser check, use an already available Chromium and Playwright
installation. This does not download a browser. Start the production preview in
one terminal, then run the check in another:

```bash
cd web
npm run preview -- --port 4175 --strictPort
# In another terminal, from web/:
PLAYWRIGHT_MODULE=/path/to/existing/playwright-core \
CHROMIUM_PATH=/path/to/existing/chromium \
BASE_URL=http://127.0.0.1:4175 npm run test:browser
```

In this cloud environment the existing module is
`/opt/codex/cua_node/lib/node_modules/playwright-core` and Chromium is
`/usr/bin/chromium`. The check covers 320, 390 and 1440 pixel widths, the results
table, navigation, readable diagram labels, page description, horizontal page
overflow and JavaScript errors. It writes screenshots to `/tmp/c10-final-*.png`.

The data generator validates all 15 timed outputs and the three excluded warm-up
outputs. It checks output/input/source hashes, empty stderr, finite symmetric
four-by-four distance matrices, equal alignment widths, and preservation of each
input sequence after removing gap markers. The test suite independently computes
the real part of the direct Fourier transform and Pearson distances for these
fixtures, and compiles the unchanged program with `-O2` to reproduce the saved
outputs without timing those test runs. These checks cover the recorded fixtures;
they do not establish biological alignment accuracy or general optimality.

The program prints the computed distance matrix and alignment, and the harness
captures and validates that observable output. This prevents treating an unused
result as evidence of work and guards against dead-code elimination of the
measured computation. Timings use `perf_counter_ns`, divide by one billion to
store seconds, and multiply by one thousand only for displayed milliseconds.

Benchmark capture uses a temporary staging directory. Compilation, execution or
validation failure leaves the previous `raw/` record intact. Only a fully
validated capture replaces it. Regeneration is byte-deterministic, and the review
round did not replace any recorded timing values. The site provides explicit empty
and invalid-data states, a keyboard-focusable runtime table, and resize checks.
