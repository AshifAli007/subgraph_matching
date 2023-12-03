#ifndef MATCH_LEAVES_H_
#define MATCH_LEAVES_H_

#include <vector>

#include "global/global.h"
#include "include/backtrack_helper.h"
#include "include/candidate_space.h"
#include "include/data_graph.h"
#include "include/maximum_matching.h"
#include "include/ordering.h"
#include "include/query_graph.h"


namespace daf {
class MatchLeaves {
 public:
  MatchLeaves(const DataGraph &data_graph, const QueryGraph &query_graph,
              const CandidateSpace &candidate_space, Vertex *query_vertex_mapping,
              BacktrackHelper *helper_array, SearchTreeNode **mapped_tree_nodes);
 

  // Delete copy constructor and copy assignment operator
  MatchLeaves(const MatchLeaves &) = delete;
  MatchLeaves &operator=(const MatchLeaves &) = delete;

  uint64_t Match(uint64_t match_limit);
 ~MatchLeaves();

 private:
  Size num_nec_distinct_cands_;
  std::vector<Size> *cand_to_nec_idx_;
  const DataGraph &data_;
  void ClearMemoryForBacktrack();
  const CandidateSpace &cs_;
  SearchTreeNode **backtrack_mapped_nodes_;
  Vertex *nec_distinct_cands_;
  std::vector<Vertex> reserved_data_vtx_;
  Size *num_remaining_nec_vertices_;
  MaximumMatching *maximum_matching_;
  Size *num_remaining_cands_;
  Size *sum_nec_cands_size_;
  std::vector<Vertex> reserved_query_vtx_;
  Size *sum_nec_size_;
  Size cur_label_idx;
  const QueryGraph &query_;
  uint64_t Combine(uint64_t combination_limit);

  void ReserveVertex(Vertex representative, BacktrackHelper *representative_helper);
  Size *nec_ranking_;
 Vertex *backtrack_mapped_query_vtx;
  BacktrackHelper *backtrack_helpers_;

};
}  // namespace daf

#endif  // MATCH_LEAVES_H_