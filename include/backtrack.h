#ifndef BACKTRACK_H_
#define BACKTRACK_H_

#include <boost/dynamic_bitset.hpp>

#include "global/global.h"
#include "include/backtrack_helper.h"
#include "include/candidate_space.h"
#include "include/data_graph.h"
#include "include/match_leaves.h"
#include "include/ordering.h"
#include "include/query_graph.h"

namespace daf {
struct SearchTreeNode;

class Backtrack {
 public:
  Backtrack(const DataGraph &data, const QueryGraph &query,
            const CandidateSpace &cs);
  inline uint64_t GetNumBacktrackCalls() const;
  Backtrack(const Backtrack &) = delete;
  uint64_t FindMatches(uint64_t limit);
  Backtrack &operator=(const Backtrack &) = delete;
  ~Backtrack();

 private:
    const DataGraph &data_;
    MatchLeaves *match_leaves_;
    uint64_t num_embeddings_;
    const CandidateSpace &cs_;
    Vertex *mapped_query_vtx_;
    Size backtrack_depth_;
    const QueryGraph &query_;
    SearchTreeNode *node_stack_;
    BacktrackHelper *helpers_;
    Ordering *extendable_queue_;
    uint64_t num_backtrack_calls_;
    SearchTreeNode **mapped_nodes_;
    void ComputeExtendable(Vertex u, Vertex u_nbr, Size u_nbr_idx, Size cs_v_idx);
    void ReleaseNeighbors(SearchTreeNode *cur_node);
    void ComputeDynamicAncestor(Vertex u, Vertex u_nbr);
    void InitializeNodeStack();
    Vertex GetRootVertex();   
    bool ComputeExtendableForAllNeighbors(SearchTreeNode *cur_node,
                                          Size cs_v_idx);
};

inline uint64_t Backtrack::GetNumBacktrackCalls() const {
  return num_backtrack_calls_;
}

}  // namespace daf

#endif  // BACKTRACK_H_
