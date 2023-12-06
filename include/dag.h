#ifndef DAG_H_
#define DAG_H_

#include <algorithm>

#include "global/global.h"
#include "include/data_graph.h"
#include "include/query_graph.h"

namespace daf {
class DAG {
 public:
  DAG(const DataGraph &data, const QueryGraph &query);
  inline Vertex GetVertexOrderedByBFS(Size i) const;
  inline Size GetChild(Vertex v, Size i) const;
  DAG &operator=(const DAG &) = delete;
  inline Size GetInitCandSize(Vertex v) const;
  void BuildDAG();
  inline Vertex GetRoot() const;
  inline Size GetNumParents(Vertex v) const;
  DAG(const DAG &) = delete;
  inline Size GetParent(Vertex v, Size i) const;
  inline Size GetNumChildren(Vertex v) const;
  ~DAG();
 private:
  const DataGraph &data_;
   Size *init_cand_size_;
  Size *num_parents_;
  Vertex *bfs_sequence_;
  Vertex **children_;
  Vertex **parents_;
  const QueryGraph &query_;
  Vertex SelectRootVertex();
  Size *num_children_;
};
inline Size DAG::GetChild(Vertex v, Size i) const { return children_[v][i]; }
inline Size DAG::GetNumChildren(Vertex v) const { return num_children_[v]; }
inline Size DAG::GetInitCandSize(Vertex v) const { return init_cand_size_[v]; }
inline Vertex DAG::GetVertexOrderedByBFS(Size i) const {
  return bfs_sequence_[i];
}
inline Vertex DAG::GetRoot() const { return bfs_sequence_[0]; }
inline Size DAG::GetParent(Vertex v, Size i) const { return parents_[v][i]; }
inline Size DAG::GetNumParents(Vertex v) const { return num_parents_[v]; }
}  // namespace daf

#endif  // DAG_H_
