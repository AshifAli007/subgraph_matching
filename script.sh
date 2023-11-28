#!/bin/bash

DATA_GRAPH_PATH="Datasets/youtube/data_graph/youtube.graph"
QUERY_GRAPH_DIR="Datasets/youtube/query_graph"


QUERY_DIR="Datasets/yeast/query_graph"

# Assuming you have a specific pattern for your query files (e.g., all files ending with ".graph")
for query_file in "$QUERY_DIR"/*.graph; do
    ./build/main/DAF -d Datasets/yeast/data_graph/yeast.graph -q "$query_file" -m 40
done