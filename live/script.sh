#!/bin/bash

DATA_GRAPH_PATH="../Datasets/yeast/data_graph/yeast.graph"
QUERY_DIR="../Datasets/yeast/query_graph"
OUTPUT_CSV="output.csv"

echo "Matches,RecursiveCalls,TotalTime,SearchTime" > "$OUTPUT_CSV"
# echo "Total time,Search time" > "$OUTPUT_CSV"
# echo "Search time" > "$OUTPUT_CSV"

for query_file in "$QUERY_DIR"/*.graph; do
    output=$(../build/main/DAF -d ../Datasets/yeast/data_graph/yeast.graph -q "$query_file" -m 20)

    matches=$(echo "$output" | awk '/#Matches:/ {print $2}')
    recursive_calls=$(echo "$output" | awk '/#Recursive calls:/ {print $3}')
    total_time=$(echo "$output" | awk '/Total time:/ {print $3}')
    search_time=$(echo "$output" | awk '/Search time:/ {print $3}')

    # echo "$search_time" >> "$OUTPUT_CSV"
    echo "$matches,$recursive_calls,$total_time,$search_time" >> "$OUTPUT_CSV"

done
