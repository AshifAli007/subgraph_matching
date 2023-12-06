#ifndef QUERY_GRAPH_H_
#define QUERY_GRAPH_H_

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "include/data_graph.h"
#include "include/graph.h"

namespace daf {
struct NECElement;

class QueryGraph : public Graph {
 public:
  inline bool IsInNEC(Vertex v) const;
  inline Size GetNECSize(Vertex v) const;
  bool LoadAndProcessGraph(const DataGraph &data);
  inline bool IsNECRepresentation(Vertex v) const;
  inline Size GetNumNECLabel() const;
  QueryGraph &operator=(const QueryGraph &) = delete;
  inline Size GetNECStartOffset(Size i) const;
  explicit QueryGraph(const std::string &filename);
  inline const NECElement &GetNECElement(Size i) const;
  inline bool IsTree() const;
  inline Size GetNumNonLeafVertices() const;
 QueryGraph(const QueryGraph &) = delete;
  inline Size GetNECRepresentative(Vertex v) const;
  inline Label GetMaxLabel() const;
 ~QueryGraph();
 inline Size GetNECEndOffset(Size i) const;
 private:
  bool is_tree_;
  Size num_non_leaf_vertices_;
  Vertex *NEC_map_;
  Size *NEC_start_offs_;
  Size *NEC_size_;
  Size num_NEC_label_;
  NECElement *NEC_elems_;
  Label max_label_;
  void ExtractResidualStructure();
};
struct NECElement {
  Label label;
  Vertex adjacent;
  Vertex represent;
  Size size;
  Size represent_idx;
};
inline bool QueryGraph::IsTree() const { return is_tree_; }
inline Size QueryGraph::GetNECStartOffset(Size i) const {
  return NEC_start_offs_[i];
}
inline const NECElement &QueryGraph::GetNECElement(Size i) const {
  return NEC_elems_[i];
}
inline Size QueryGraph::GetNumNECLabel() const { return num_NEC_label_; }
inline Size QueryGraph::GetNECRepresentative(Vertex v) const {
  if (IsInNEC(v)) {
    return NEC_map_[v];
  } else {
    return v;
  }
}
inline Size QueryGraph::GetNumNonLeafVertices() const {
  return num_non_leaf_vertices_;
}inline bool QueryGraph::IsInNEC(Vertex v) const {
  return NEC_map_[v] != INVALID_VTX;
}
inline Size QueryGraph::GetNECEndOffset(Size i) const {
  return NEC_start_offs_[i + 1];
}
inline Size QueryGraph::GetNECSize(Vertex v) const { return NEC_size_[v]; }
inline bool QueryGraph::IsNECRepresentation(Vertex v) const {
  return NEC_map_[v] == v;
}
inline Label QueryGraph::GetMaxLabel() const { return max_label_; }
}  // namespace daf
#endif  // QUERY_GRAPH_H_