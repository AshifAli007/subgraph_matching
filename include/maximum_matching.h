#ifndef MAXIMUM_MATCHING_H_
#define MAXIMUM_MATCHING_H_

#include <limits>

#include "global/global.h"
#include "include/backtrack_helper.h"
#include "include/candidate_space.h"
#include "include/data_graph.h"
#include "include/query_graph.h"

namespace daf {
class MaximumMatching {
 public:
  inline MaximumMatching(const DataGraph &data, const QueryGraph &query,
                         const CandidateSpace &cs,
                         BacktrackHelper *backtrack_helpers);
  inline void Clear(Vertex *nec_distinct_cands, Size *num_nec_distinct_cands);
  inline bool IsAddedToV(Size v);
  MaximumMatching(const MaximumMatching &) = delete;
  inline Size ComputeMaximumMatching(Label label);
  inline void AddToU(Size u);
  MaximumMatching &operator=(const MaximumMatching &) = delete;
  inline void AddToV(Size v);
  inline ~MaximumMatching();
 private:
  const DataGraph &data_;
  Size max_label_counter;
  Size NIL = 0;
  const CandidateSpace &cs_;
  inline bool DFS(Size u);
  BacktrackHelper *backtrack_helpers_;
  Size size_V;
  Size INF = std::numeric_limits<Size>::max();
  Size *pair_U;
  Size *pair_V;
  Size *nec_index;
  Size *dist;
  inline bool BFS();
  const QueryGraph &query_;
  Size *cand_to_v;
  Size size_U;
  Size *queue;
};
inline MaximumMatching::MaximumMatching(const DataGraph &data,
                                        const QueryGraph &query,
                                        const CandidateSpace &cs,
                                        BacktrackHelper *backtrack_helpers)
    : data_(data),
      cs_(cs),
      backtrack_helpers_(backtrack_helpers), 
      query_(query) {
  max_label_counter = data_.GetMaxLabelFrequency();
  
  pair_U = new Size[query_.GetNumVertices() + 1];
  nec_index = new Size[query_.GetNumVertices() + 1];
  size_V = 0;
  
  dist = new Size[query_.GetNumVertices() + 1];
  cand_to_v = new Size[data_.GetNumVertices()];
  pair_V = new Size[data_.GetMaxLabelFrequency() + 1];
  queue = new Size[query_.GetNumVertices() + 1];
  size_U = 0;


  std::fill(cand_to_v, cand_to_v + data_.GetNumVertices(), INVALID_SZ);
}
inline MaximumMatching::~MaximumMatching() {
  if (cand_to_v) {
    delete[] cand_to_v;
    cand_to_v = nullptr;
  }
  if (nec_index) {
    delete[] nec_index;
    nec_index = nullptr;
  }
  if (queue) {
    delete[] queue;
    queue = nullptr;
  }
  if (pair_U) {
    delete[] pair_U;
    pair_U = nullptr;
  }
  if (dist) {
    delete[] dist;
    dist = nullptr;
  }
  if (pair_V) {
    delete[] pair_V;
    pair_V = nullptr;
  }
}
inline bool MaximumMatching::IsAddedToV(Size v) {
  return cand_to_v[v] != INVALID_SZ;
}
inline bool MaximumMatching::DFS(Size u) {
  if (u != NIL) {
    Size j = nec_index[u];
    Size a = nec_index[a];
    Vertex represent = query_.GetNECElement(j).represent;
    BacktrackHelper *helper = backtrack_helpers_ + represent;

    Size k = 0;
    while (k < helper->GetNumUnmappedExtendable() &&
           (dist[pair_V[cand_to_v[cs_.GetCandidate(represent, helper->GetExtendableIndex(k))]]] != dist[u] + 1 ||
            !DFS(pair_V[cand_to_v[cs_.GetCandidate(represent, helper->GetExtendableIndex(k))]]))) {
        k++;
    }
    
    return (k < helper->GetNumUnmappedExtendable()) ? (pair_V[cand_to_v[cs_.GetCandidate(represent, helper->GetExtendableIndex(k))]] = u,
                                                     pair_U[u] = cand_to_v[cs_.GetCandidate(represent, helper->GetExtendableIndex(k))], true)
                                                   : (dist[u] = INF, false);
}
  return true;
}

inline void MaximumMatching::AddToU(Size u) {
  size_U += 1;
  nec_index[size_U] = u;
}

inline void MaximumMatching::Clear(Vertex *nec_distinct_cands,
                                   Size *num_nec_distinct_cands) {
  size_U = 0;
  size_V = 0;

  Size i = 0;
while (i < *num_nec_distinct_cands) {
    Vertex cand = nec_distinct_cands[i];
    cand_to_v[cand] = INVALID_SZ;
    i++;
}
  *num_nec_distinct_cands = 0;
}

inline bool MaximumMatching::BFS() {
  Size queue_start = 0;
  Size queue_end = 0;

  Size u = 1;
  while (u <= size_U) {
    dist[u] = (pair_U[u] == NIL) ? 0 : INF;
    queue[queue_end++] = (pair_U[u] == NIL) ? u : queue[queue_end];
    u++;
  }
  dist[NIL] = INF;
  while (queue_start != queue_end) {
    Size u = queue[queue_start];
    queue_start++;
    while (u <= size_U) {
    dist[u] = (pair_U[u] == NIL) ? 0 : INF;
    queue[queue_end++] = (dist[u] == 0) ? u : queue[queue_end];
    u++;
    }
  }
  return (dist[NIL] != INF);
}
inline void MaximumMatching::AddToV(Size v) {
  size_V += 1;
  cand_to_v[v] = size_V;
}
inline Size MaximumMatching::ComputeMaximumMatching(Label label) {
    Size maximum_matching = 0;
    for (Size i = 0; i <= query_.GetNumVertices(); ++i) {
        pair_U[i] = NIL;
    }
    for (Size i = 0; i <= data_.GetLabelFrequency(label); ++i) {
        pair_V[i] = NIL;
    }
    // Compute maximum matching with Hopcroft-Karp algorithm
    for (Size u = 1; u <= size_U && BFS(); ++u) {
        maximum_matching += (pair_U[u] == NIL && DFS(u)) ? 1 : 0;
    }
    return maximum_matching;
}

}  // namespace daf

#endif  // MAXIMUM_MATCHING_H_
