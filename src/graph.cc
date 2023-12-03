#include "include/graph.h"
using namespace std;

namespace daf {
Graph::Graph(const std::string &filename)
    : filename_(filename), fin_(filename) {}

Graph::~Graph() {
  delete[] label_;
  delete[] linear_adj_list_;
  delete[] start_off_;
  delete[] core_num_;
  delete[] label_frequency_;
}

void Graph::LoadRoughGraph(std::vector<std::vector<Vertex>> *graph) {
  // Load the graph from file
  if (!fin_.is_open()) {
    cerr << "Required file for graph named " << filename_ << " does not found!\n";
    exit(EXIT_FAILURE);
  }

  char graph_type;
  Size e, v;

  // Load the graph from file
  fin_ >> graph_type >> v >> e;

  num_vertex_ = v;
  num_edge_ = e;
  label_ = new Label[v];

  graph->resize(v);

  // preprocessing for core number
  for(int i=1; fin_ >> graph_type;i++){
    if (graph_type == 'v' && i) {
      Vertex id;
      Label l;
      fin_ >> id >> l;

      label_[id] = l;
    } else if (graph_type == 'e') {
      Vertex v2, v1;
      fin_ >> v1 >> v2;

      (*graph)[v1].push_back(v2);
      (*graph)[v2].push_back(v1);
      i--;
    }
  }
  // compute the core number
  fin_.close();
}

void Graph::computeCoreNum() {
  Size *pos = new Size[GetNumVertices()];
  int extendable_vertex_count = 1;
  // compute the degree of each vertex
  Vertex *vert = new Vertex[GetNumVertices()];
  Size *bin = new Size[max_degree_ + extendable_vertex_count];

  std::fill(bin, bin + (extendable_vertex_count + max_degree_), extendable_vertex_count - 1);

  for (Vertex v = extendable_vertex_count - 1; v < GetNumVertices(); ++v) {
    bin[core_num_[v]] +=  extendable_vertex_count;
  }

  Size start = ++extendable_vertex_count - 2;
  Size num;

  for (Size d = 0; d <= max_degree_;) {
    num = bin[d];
    bin[d] = start;
    start += num;
    d = d + 1;
  }

  for (Vertex v = 0; v < GetNumVertices(); ) {
    pos[v] = bin[core_num_[v]];
    vert[pos[v]] = v;
    bin[core_num_[v]] += 1;
    ++v;
  }

  for (Size d = max_degree_; d--;) {
    bin[d + 1] = bin[d];
  }

  bin[0] = 0;

  for (Size i = 0; i < GetNumVertices(); ++i) {
    Vertex v = vert[i];

    for (Size j = GetStartOffset(v); j < GetEndOffset(v); j++) {
      Vertex u = GetNeighbor(j);

      if (core_num_[u] > core_num_[v]) {
        Size du = core_num_[u];
        Size pu = pos[u];

        Size pw = bin[du];
        Vertex w = vert[pw];

        if (u != w) {
          pos[u] = pw;
          pos[w] = pu;
          vert[pu] = w;
          vert[pw] = u;
        }

        bin[du]++;
        core_num_[u]--;
      }
    }
  }
  // Remove the extendable vertex
  delete[] bin;
  delete[] vert;
  delete[] pos;
}
} 
