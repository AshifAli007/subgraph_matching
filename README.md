# Introduction
This project leverages the Dynamic Adaptive Failing set (DAF) algorithm, designed to tackle the computationally intensive problem of subgraph matching with unprecedented efficiency. With a focus on robustness and scalability, our implementation facilitates the discovery of subgraph patterns within larger graphs, a task important to various domains such as social network analysis, bioinformatics, and pattern recognition in large datasets.

# Prerequisites
To run this project, you must have the following installed:

- CMake (version 3.5 or higher recommended)
- Boost C++ Libraries

# Build
```
mkdir build
cd build
cmake ..
make
```

# Running the algorithm
To run the algorithm with the compiled executable, navigate to the build directory and use the following command syntax:
```
./build/main/DAF [options]
```
Options: 
- -d: specify the data graph file name.
- -q: specify the query graph file name.
- -m: specify the number of matches to find (if not specified, the algorithm will find all matches).

Example:
```
./build/main/DAF -d dataset_example/data.graph -q dataset_example/query.graph -m 10
```

# Input File Format
The graph file format for data graphs and query graphs is a text format to store a vertex-labeled undirected graph.
Structure:
- The first line: t #vertices #edges
- Vertex lines: v vertex-ID vertex-label
    * Vertex IDs should range from 0 to #vertices - 1 and be listed in ascending order.
- Edge lines: e vertex-ID1 vertex-ID2
    * These indicate the undirected edges in the graph.
  Example Input File:
```
t 4 5
v 0 0
v 1 1
v 2 2
v 3 3
e 0 1
e 0 2
e 1 2
e 1 3
e 2 3
```
You can find example data graph and query graph files under the dataset_example directory in the repository.
