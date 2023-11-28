#!/bin/bash

DATA_GRAPH_PATH="Datasets/youtube/data_graph/youtube.graph"
QUERY_GRAPH_DIR="Datasets/youtube/query_graph"


QUERY_DIR="Datasets/yeast/query_graph"

# Output CSV file
OUTPUT_CSV="output.csv"

# Write CSV header
echo "Matches,Recursive calls,Total time,Search time" > "$OUTPUT_CSV"

# Assuming you have a specific pattern for your query files (e.g., all files ending with ".graph")
for query_file in "$QUERY_DIR"/*.graph; do
    # Run the command and capture the output
    output=$(./build/main/DAF -d Datasets/yeast/data_graph/yeast.graph -q "$query_file" -m 40)

    # Extract the relevant information using awk
    matches=$(echo "$output" | awk '/#Matches:/ {print $2}')
    recursive_calls=$(echo "$output" | awk '/#Recursive calls:/ {print $3}')
    total_time=$(echo "$output" | awk '/Total time:/ {print $3}')
    search_time=$(echo "$output" | awk '/Search time:/ {print $3}')

    # Append the values to the CSV file
    echo "$matches,$recursive_calls,$total_time,$search_time" >> "$OUTPUT_CSV"
done
