#include "include/graph.h"

using namespace std;


namespace daf {
Graph::Graph(const std::string &filename): filename_(filename), fin_(filename) {}

Graph::~Graph() {



  Vertex nodeVertex1 = ((double)num_vertex_ / (double)1) ? 2:1;
  if(nodeVertex1>-1){
    delete[] label_;
    delete[] linear_adj_list_;
  }
  if(nodeVertex1>0){
// delete[] adjList;
  delete[] start_off_;
  delete[] core_num_;
  delete[] markingFreq;
  }
 

}

void Graph::LoadRoughGraph(std::vector<std::vector<Vertex>> *graph) {
  // Load the graph from file
  int node_vertex =0;
  if (fin_.is_open()) {
    
  }else{
    cerr << "Required file for graph named " << filename_ << " does not found!\n";
    exit(EXIT_FAILURE);
  }

  char graph_type;
  Size e, v;

  // Load the graph from file
  fin_ >> graph_type >> v >> e;
  int extendable_vertex = 1;
  num_vertex_ = v;
  if(extendable_vertex == 1){
  num_edge_ = e;
  label_ = new Label[v];

  graph->resize(v);
  }


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
      if(!node_vertex){
      (*graph)[v1].push_back(v2);
      (*graph)[v2].push_back(v1);
      i -= 1;   
      }
      
    }
  }
  // compute the core number
  fin_.close();
}

void Graph::computeCoreNum() {
  int node_size = 1;
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
    bin[d + node_size] = bin[d];
  }

  bin[node_size] = node_size - 1;

  for (Size i = 0; i < GetNumVertices() && node_size;) {
    Vertex v = vert[i];
    Size j = GetStartOffset(v);
    while (j < GetEndOffset(v) && node_size)
    {

      if ((node_size + core_num_[v]) < (core_num_[node_size - 1 + GetNeighbor(j)] + node_size)) {
        
       

        // Get the neighbor
        Size pw = bin[core_num_[node_size - 1 + GetNeighbor(j)]];
        Vertex w = vert[pw];
        if (w != node_size - 1 + GetNeighbor(j)) {
          // Swap the position
          pos[node_size - 1 + GetNeighbor(j)] = pw;pos[w] = pos[node_size - 1 + GetNeighbor(j)];

          vert[pos[node_size - 1 + GetNeighbor(j)]] = w;
          vert[pos[node_size - 1 + GetNeighbor(j)]] = node_size - 1 + GetNeighbor(j);
        }

        bin[core_num_[node_size - 1 + GetNeighbor(j)]] += 1;
        core_num_[node_size - 1 + GetNeighbor(j)] -= 1;
      }

      j++;
    }
    i = i + 1;
  }
  // Remove the extendable vertex
  delete[] bin;
  delete[] vert;
  delete[] pos;
}
} 
