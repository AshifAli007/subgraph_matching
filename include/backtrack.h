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
  Backtrack( const QueryGraph &query,
            const CandidateSpace &cs, const DataGraph &data);
  Backtrack &operator=(const Backtrack &) = delete;

  uint64_t FindMatches(uint64_t limit);
  Backtrack(const Backtrack &) = delete;
  inline uint64_t GetNumBacktrackCalls() const;
  ~Backtrack();

 private:
 BacktrackHelper *helpers_;
  const DataGraph &data_;
  uint64_t num_backtrack_calls_;
  bool ComputeExtendableForAllNeighbors(Size cs_v_idx, SearchTreeNode *cur_node);

  SearchTreeNode *node_stack_;
  Ordering *extendable_queue_;
  MatchLeaves *match_leaves_;
  void InitializeNodeStack();

  Vertex *mapped_query_vtx_;
  
  SearchTreeNode **mapped_nodes_;
  void ComputeDynamicAncestor(Vertex u_nbr, Vertex u);

  uint64_t num_embeddings_;
  
  
  const QueryGraph &query_;
  Vertex GetRootVertex();
  
  void ComputeExtendable(Size u_nbr_idx, Size cs_v_idx, Vertex u, Vertex u_nbr);
                                        
  void ReleaseNeighbors(SearchTreeNode *cur_node);
   const CandidateSpace &cs_;
   Size backtrack_depth_;
};

inline uint64_t Backtrack::GetNumBacktrackCalls() const {
  return num_backtrack_calls_;
}

}  // namespace daf

#endif  // BACKTRACK_H_
