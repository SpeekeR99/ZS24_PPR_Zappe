# Parallel Computation of Coefficient of Variation and Median Absolute Deviation


## Project Description
This project focuses on the parallel computation of the **Coefficient of Variation (CV)** and the **Median Absolute Deviation (MAD)** for large datasets.
The computations are implemented in multiple ways to compare performance across different approaches, including serial, vectorized, multithreaded, and GPU-based calculations.

## Objectives
1. Compute CV and MAD for datasets using:
   - Serial code.
   - Vectorized code.
   - Multithreaded (non-vectorized) code.
   - Multithreaded and vectorized code.
   - GPU-based code.
2. Perform each computation 10 times and report the median execution time.
3. Generate three `.svg` graphs:
   - Execution times for all computation types as data size increases.
   - Convergence of CV values across computation types.
   - Convergence of MAD values across computation types.

## Dataset
The dataset consists of accelerometer values (X, Y, Z) extracted from files named `ACC*.csv`. Each dataset contains 74,381 values per axis. The computations are performed incrementally for subsets of 1,000 to all 74,381 values.

## Methodology

### **1. Data Preprocessing**
- Data is loaded using optimized methods to reduce I/O overhead.
- Dynamic memory allocation is minimized by preallocating arrays based on empirical estimates.

### **2. Computation Approaches**
- **Serial Code:** Standard implementation without parallelization or vectorization.
- **Vectorized Code:** Manual AVX2 vectorization using `immintrin.h`.
- **Multithreaded Code:** Parallel execution using `std::execution::par` and dynamic load balancing.
- **GPU Code:** Implemented with OpenCL, performing reduction operations directly on the GPU.

### **3. Algorithms**
- **MAD Calculation:**
  - Compute the median of the dataset.
  - Calculate absolute deviations from the median.
  - Compute the median of these deviations.
- **CV Calculation:**
  - Compute the mean (\( \mu \)) and variance (\( \sigma^2 \)).
  - Use the formula \( CV = \frac{\sigma}{\mu} = \sqrt{\frac{\text{Var}(X)}{\mu}} \).

## Implementation Details

### Programming Language
The project is implemented in C++17 without external libraries except for SVG rendering.

### Optimizations
1. **Data Loading:**
   - Replaced `std::ifstream` with `fopen()` for faster file I/O.
   - Parallelized line processing after loading files into RAM.
2. **MAD Optimization:**
   - Avoided redundant sorting by leveraging properties of sorted arrays.
3. **Vectorization:**
   - Used AVX2 instructions for operations like subtraction and absolute value calculations.

### Performance Enhancements
- Dynamic load balancing ensures efficient use of CPU cores.
- GPU kernels handle reduction operations to maximize parallelism.

## Results

### Performance Comparison
1. Execution times decrease significantly with vectorization and parallelization.
2. GPU-based computations achieve the fastest execution times for large datasets.

### Graphs
1. Execution time vs. data size for all computation types.
2. Convergence of CV values across computation types.
3. Convergence of MAD values across computation types.

## How to Run

### Build

The project can be built using the provided `CMakeLists.txt` file.

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

```bash
mkdir build
cd build
cmake ..
make
```

Compile with the following flag to switch from double to float precision:

```bash
cmake .. -DCMAKE_CXX_FLAGS="-D_USE_FLOAT"
```

### Run

After building the project, execute the generated binary to perform the computations.
One flag is required, that is the path to the directory containing the accelerometer data files or the concrete file with the data.
There are many optional flags to control the execution of the program.

Example:

```bash
./programme -d ../data
```

or 

```bash
./programme -f ../data/ACC1.csv
```

### Flags

The following is a list of possible flags and switches with their explanations:

#### Required Flags:
- `-f <file>` – Defines the input file.
- `-d <directory>` – Defines the directory containing input files.

These flags tell the program where to retrieve the data from. You can process either a single file or an entire directory of data files (in the expected format specified in the task). These flags are mutually exclusive, but one of them is required.

#### Optional Flags:
- `-r <number of repetitions>` – This flag specifies the number of repetitions for the experiment. The median of the results from all experiments is written to the final graph. To meet the task requirements, you need to run with `-r 10`.
- `-n <batch size>` – Defines the number of chunks the input data should be split into for the r repetitions of calculations. This essentially controls the granularity of the X-axis in the output graphs.
- `--par` – No value is expected after this flag. It switches between serial and parallel computation.
- `--vec` – No value is expected after this flag. It switches between sequential and vectorized computation.
- `--gpu` – Again, no value is expected. This flag switches between CPU and GPU computation.
- `--all` – No value is expected. This flag allows all combinations of computation types to be iteratively performed on the data file. When used, the graphical output changes to display five curves, each corresponding to a different type of computation. If the program is run in a single computation mode, the graphs will display three curves (one for each input data column – X, Y, and Z).
- `--no-graphs` – No value is expected. This flag prevents the generation of images at the end of the program execution (useful mainly during development for debugging purposes).
- `-h` – Displays help information.
- `--help` – Displays help information.

### Example

```bash
./programme -d ../data -r 10 -n 10 --all
```

This command will run the program on all data files in the specified directory, with 10 repetitions for each batch size of 10. The program will perform all computation types and generate the required graphs.
