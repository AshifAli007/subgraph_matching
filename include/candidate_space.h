#ifndef CANDIDATE_SPACE_H_
#define CANDIDATE_SPACE_H_

#include <utility>

#include "global/global.h"
#include "include/dag.h"
#include "include/data_graph.h"
#include "include/query_graph.h"

namespace daf {
class CandidateSpace {
 public:
  inline Size GetCandidateEndOffset(Vertex u, Size u_adj_idx, Size v_idx) const;
  bool BuildCS();
  CandidateSpace(const DataGraph &data, const QueryGraph &query,
                 const DAG &dag);
  inline Size GetCandidateIndex(Size idx) const;
  CandidateSpace &operator=(const CandidateSpace &) = delete;
  inline Size GetCandidateSetSize(Vertex u) const;
  inline Size GetCandidateStartOffset(Vertex u, Size u_adj_idx,
                                      Size v_idx) const;
  CandidateSpace(const CandidateSpace &) = delete;
  ~CandidateSpace();
  inline Vertex GetCandidate(Vertex u, Size v_idx) const;
 private:
  void ComputeNbrInformation(Vertex u, Size *max_nbr_degree,
                             uint64_t *label_set);
  const DataGraph &data_;
  QueryDegree *num_visit_cs_;
  bool FilterByTopDown();
  Size *candidate_set_size_;
  Vertex *linear_cs_adj_list_;
  bool FilterByBottomUp();
  Size *cand_to_cs_idx_;
  Size ***candidate_offsets_;
  Size num_cs_edges_;
  bool FilterByTopDownWithInit();
  void ConstructCS();
  Size num_visitied_candidates_;
  const DAG &dag_;
  bool InitRootCandidates();
  Vertex **candidate_set_;
  Vertex *visited_candidates_;
  const QueryGraph &query_;
};
  inline Size CandidateSpace::GetCandidateSetSize(Vertex u) const {
    return candidate_set_size_[u];
  }
  inline Size CandidateSpace::GetCandidateStartOffset(Vertex u, Size u_adj_idx,
                                                      Size v_idx) const {
    return candidate_offsets_[u][u_adj_idx][v_idx];
  }
  inline Size CandidateSpace::GetCandidateIndex(Size idx) const {
    return linear_cs_adj_list_[idx];
  }
  inline Vertex CandidateSpace::GetCandidate(Vertex u, Size v_idx) const {
    return candidate_set_[u][v_idx];
  }
  inline Size CandidateSpace::GetCandidateEndOffset(Vertex u, Size u_adj_idx,
                                                    Size v_idx) const {
    return candidate_offsets_[u][u_adj_idx][v_idx + 1];
  }

}  // namespace daf

#endif  // CANDIDATE_SPACE_H_