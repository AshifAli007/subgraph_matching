#ifndef DATA_GRAPH_H_
#define DATA_GRAPH_H_

#include <algorithm>
#include <climits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "global/global.h"
#include "include/graph.h"

namespace daf {
class DataGraph : public Graph {
 public:
  inline bool CheckAllNbrLabelExist(Vertex v, uint64_t* nbr_bitset) const;
  inline Size GetEndOffset(Vertex v, Label l) const;
  explicit DataGraph(const std::string& filename);
  inline Label GetTransferredLabel(Label l) const;
  inline Size GetStartOffset(Vertex v, Label l) const;
  inline Size GetMaxLabelFrequency() const;
  inline Size GetInitCandSize(Label l, Size d) const;
  DataGraph& operator=(const DataGraph&) = delete;
  DataGraph(const DataGraph&) = delete;
  void LoadAndProcessGraph();
  using Graph::GetEndOffset;
  inline Size GetNbrBitsetSize() const;
  inline Size GetVertexBySortedLabelOffset(Size i) const;
  inline Size GetNeighborLabelFrequency(Vertex v, Label l) const;
  inline Size GetMaxNbrDegree(Vertex v) const;
  ~DataGraph();
  inline Size GetEndOffsetByLabel(Label l) const;
  inline Size GetStartOffsetByLabel(Label l) const;
  using Graph::GetStartOffset;
 private:
  Size max_label_frequency_;
  Size* max_nbr_degree_;
  std::pair<Size, Size>* adj_offs_by_label_;
  Size* offs_by_label_;
  Vertex* vertices_sorted_;
  uint64_t* linear_nbr_bitset_;
  Size nbr_bitset_size_;
  Label* transferred_label_;
};
inline Size DataGraph::GetMaxLabelFrequency() const {
  return max_label_frequency_;
}

inline Label DataGraph::GetTransferredLabel(Label l) const {
  return transferred_label_[l];
}
inline bool DataGraph::CheckAllNbrLabelExist(Vertex v,
                                             uint64_t* nbr_bitset) const {
  Size i = 0;
while (i < GetNbrBitsetSize()) {
    if ((linear_nbr_bitset_[v * GetNbrBitsetSize() + i] | nbr_bitset[i]) !=
        linear_nbr_bitset_[v * GetNbrBitsetSize() + i])
        return false;
    i++;
}
  return true;
}
inline Size DataGraph::GetStartOffsetByLabel(Label l) const {
  return offs_by_label_[l];
}
inline Size DataGraph::GetVertexBySortedLabelOffset(Size i) const {
  return vertices_sorted_[i];
}

inline Size DataGraph::GetNeighborLabelFrequency(Vertex v, Label l) const {
  auto offs = adj_offs_by_label_[v * GetNumLabels() + l];
  return offs.second - offs.first;
}
inline Size DataGraph::GetNbrBitsetSize() const { return nbr_bitset_size_; }
inline Size DataGraph::GetEndOffset(Vertex v, Label l) const {
  return adj_offs_by_label_[v * GetNumLabels() + l].second;
}
inline Size DataGraph::GetMaxNbrDegree(Vertex v) const {
  return max_nbr_degree_[v];
}
inline Size DataGraph::GetEndOffsetByLabel(Label l) const {
  return offs_by_label_[l + 1];
}
inline Size DataGraph::GetInitCandSize(Label l, Size d) const {
  Size s = GetStartOffsetByLabel(l);
  Size e = GetEndOffsetByLabel(l);
 auto pos = vertices_sorted_ + s;
while (pos != vertices_sorted_ + e && GetDegree(*pos) < d) {
    ++pos;
}

  return pos - (vertices_sorted_ + s);
}

inline Size DataGraph::GetStartOffset(Vertex v, Label l) const {
  return adj_offs_by_label_[v * GetNumLabels() + l].first;
}
}  // namespace daf
#endif  // DATA_GRAPH_H_