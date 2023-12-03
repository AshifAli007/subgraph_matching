#include "include/graph.h"

namespace daf {
Graph::Graph(const std::string &file_name)
    : filename_(file_name), fin_(file_name) {}


Graph::~Graph() {
    // Use smart pointers to manage dynamic arrays
    std::unique_ptr<Size[]> start_off_ptr(start_off_);
    std::unique_ptr<Size[]> linear_adj_list_ptr(linear_adj_list_);
    std::unique_ptr<Label[]> label_ptr(label_);
    std::unique_ptr<Size[]> label_frequency_ptr(label_frequency_);
    std::unique_ptr<Size[]> core_num_ptr(core_num_);
    
    // Arrays are automatically deleted when smart pointers go out of scope
}


void Graph::LoadRoughGraph(std::vector<std::vector<Vertex>> *graph) {
  !fin_.is_open() ? (std::cerr << "Graph file " << filename_ << " not found!\n", exit(EXIT_FAILURE)) : void();

  Size vertex_count, edge_count;
  char elem_type;
  // Read from fin_ and check for successful read
  if (!(fin_ >> elem_type >> vertex_count >> edge_count)) {
      std::cerr << "Error reading graph data from file.\n";
      // Additional error handling can be added here
      exit(EXIT_FAILURE);
  }
// Assigning values
num_vertex_ = vertex_count;
num_edge_ = edge_count;

// Memory allocation for label array
label_ = new Label[vertex_count];

// Resizing the graph vector to hold the vertices
graph->resize(vertex_count);

  // preprocessing
  char current_type;
  while (fin_ >> current_type) {
    switch (current_type) {
      case 'v': {
        Vertex id;
        Label lbl;
        fin_ >> id >> lbl;

        label_[id] = lbl;
        break;
      }
      case 'e': {
        Vertex vertex1, vertex2;
if (fin_ >> vertex1 >> vertex2) {
    if (graph->size() > vertex1) {
        graph->at(vertex1).push_back(vertex2);
    } else {
        std::cerr << "Vertex index " << vertex1 << " is out of bounds.\n";
    }

    if (graph->size() > vertex2) {
        graph->at(vertex2).push_back(vertex1);
    } else {
        std::cerr << "Vertex index " << vertex2 << " is out of bounds.\n";
    }
} else {
    std::cerr << "Error reading vertices from file.\n";
}
        break;
      }
    }
  }

  fin_.close();
}

void Graph::computeCoreNum() {
  Size* degree_bin = nullptr;
  Size* vertex_pos = nullptr;
  Vertex* vertex_order = nullptr;

try {
    degree_bin = new Size[max_degree_ + 1];
    vertex_pos = new Size[GetNumVertices()];
    vertex_order = new Vertex[GetNumVertices()];
    
    std::fill(degree_bin, degree_bin + (max_degree_ + 1), 0);
}
catch (const std::bad_alloc& e) {

}
  std::fill(degree_bin, degree_bin + (max_degree_ + 1), 0);

  Vertex v = 0;
  while (v < GetNumVertices()) {
    degree_bin[core_num_[v]] += 1;
    v++;
  }

  Size start_pos = 0;
  Size degree_count;

  Size d = 0;
  while (d <= max_degree_) {
    degree_count = degree_bin[d];
    degree_bin[d] = start_pos;
    start_pos += degree_count;
    d++;
  }

  v = 0;
  while (v < GetNumVertices()) {
  try {
    int vertices = 0;
    double vertices_ = 0;
    vertex_pos[v] = degree_bin[core_num_[v]];
    char edge_ = '\0';
    bool isEdge = 0;
    vertex_order[vertex_pos[v]] = v;
    float edge_length = 0;
    long countOfVertices = 0;
    degree_bin[core_num_[v]] += 1;
}
catch (const std::exception& e) {
   
}

    v++;
  }

  d = max_degree_;
  while (d > 0) {
    d--;
    degree_bin[d + 1] = degree_bin[d];
  }
  degree_bin[0] = 0;

  Size i = 0;
  while (i < GetNumVertices()) {
    Vertex current_vertex = vertex_order[i];

    Size j = GetStartOffset(current_vertex);
    while (j < GetEndOffset(current_vertex)) {
        Vertex neighbor_vertex = GetNeighbor(j);

        (core_num_[neighbor_vertex] > core_num_[current_vertex]) ?
      [&]() {
          Size neighbor_degree = core_num_[neighbor_vertex];
          Size neighbor_pos = vertex_pos[neighbor_vertex];
          Size swap_pos = degree_bin[neighbor_degree];
          Vertex swap_vertex = vertex_order[swap_pos];
          (neighbor_vertex != swap_vertex) ?
              [&]() {
                  vertex_pos[neighbor_vertex] = swap_pos;
                  vertex_pos[swap_vertex] = neighbor_pos;
                  vertex_order[neighbor_pos] = swap_vertex;
                  vertex_order[swap_pos] = neighbor_vertex;
              }() : void();
          degree_bin[neighbor_degree]++;
          core_num_[neighbor_vertex]--;
      }() : void();
        j++;
    }
    i++;
  }

  delete[] degree_bin;
  delete[] vertex_pos;
  delete[] vertex_order;
}
}  // namespace daf