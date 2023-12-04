#ifndef GRAPH_H_
#define GRAPH_H_

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "global/global.h"

namespace daf {

class Graph {
 public:
  explicit Graph(const std::string& file_name);
  ~Graph();

  Graph(const Graph&) = delete;
  Graph& operator=(const Graph&) = delete;

  inline Size GetMaxDegree() const { return max_degree_; }
  inline Size GetNumLabels() const { return num_label_; }

   
  // Inline methods for accessing vertex properties
  inline Label GetLabel(Vertex vertex) const { return label_[vertex]; }
  inline Size GetDegree(Vertex vertex) const {
    return start_off_[vertex + 1] - start_off_[vertex];
  }
  inline Size GetStartOffset(Vertex vertex) const { return start_off_[vertex]; }
  inline Size GetEndOffset(Vertex vertex) const { return start_off_[vertex + 1]; }
  inline Size GetCoreNum(Vertex vertex) const { return core_num_[vertex]; }

  // Inline method for label frequency
  inline Label GetLabelFrequency(Label label_id) const {
    return label_frequency_[label_id];
  }

  // Method for accessing neighbors
  inline Vertex GetNeighbor(Size index) const { return linear_adj_list_[index]; }
  // Inline methods for accessing graph properties
  inline Size GetNumVertices() const { return num_vertex_; }
    inline Size GetNumEdges() const { return num_edge_; }
   

 protected:
  // Member variables
  Size num_edge_;
  Label* label_;
  Size* start_off_;
  Vertex* linear_adj_list_;
 void LoadRoughGraph(std::vector<std::vector<Vertex>>* graph_structure);
  Size* label_frequency_;
  Size* core_num_;
  Size num_label_;
  Size max_degree_;
  void computeCoreNum();
  const std::string& filename_;
  Size num_vertex_;
  std::ifstream fin_;
};

}  // namespace daf

#endif  // GRAPH_H_