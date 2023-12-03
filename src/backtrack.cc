#include "include/backtrack.h"

namespace daf {
Backtrack::Backtrack( const QueryGraph &query,
                     const CandidateSpace &cs, const DataGraph &data)
    : data_(data), query_(query), cs_(cs) {
  num_backtrack_calls_ = 0;
  num_embeddings_ = 0;
  backtrack_depth_ = 1;

  helpers_ = new BacktrackHelper[query_.GetNumVertices()];
  mapped_query_vtx_ = new Vertex[data_.GetNumVertices()];
  node_stack_ = new SearchTreeNode[query_.GetNumNonLeafVertices() + 1];
  extendable_queue_ = new Ordering(query_.GetNumVertices());
  mapped_nodes_ = new SearchTreeNode *[query_.GetNumVertices()];


  if (query_.GetNumVertices() < query_.GetNumNonLeafVertices()) {
    match_leaves_ = nullptr;
    
  } else {
    match_leaves_ = new MatchLeaves(data_, query_, cs_, mapped_query_vtx_,
                                    helpers_, mapped_nodes_);
  }
for (Vertex u = 0; u < query_.GetNumVertices(); ++u) {
    helpers_[u].Initialize(query_.GetNumVertices(), query_.GetDegree(u),
                           cs_.GetCandidateSetSize(u), u);
  }
  std::fill(mapped_query_vtx_, mapped_query_vtx_ + data_.GetNumVertices(),
            INVALID_VTX);

  
}



uint64_t Backtrack::FindMatches(uint64_t limit) {

  Vertex root_vertex = GetRootVertex();
  Size root_cs_size = cs_.GetCandidateSetSize(root_vertex);

  extendable_queue_->Insert(root_vertex, root_cs_size);

  InitializeNodeStack();

  while (backtrack_depth_ > 0) {
    if (limit <= num_embeddings_) {
      return num_embeddings_;
    }

    SearchTreeNode *cur_node = node_stack_ + backtrack_depth_;
    SearchTreeNode *parent_node = node_stack_ + (backtrack_depth_ - 1);

    BacktrackHelper *u_helper;

    if (cur_node->initialized == true) {
      // backtrack from child node
      ReleaseNeighbors(cur_node);

      u_helper = helpers_ + cur_node->u;

      if (cur_node->embedding_founded) {

        parent_node->embedding_founded = true;
        cur_node->v_idx += 1;
      } else {
        if (cur_node->failing_set.test(cur_node->u) == false) {
         
          parent_node->failing_set = cur_node->failing_set;
          cur_node->v_idx = std::numeric_limits<Size>::max();
        } else {
          
          parent_node->failing_set |= cur_node->failing_set;
          cur_node->v_idx += 1;
        }
      }
    } else {
      // newly expanded search tree node
      num_backtrack_calls_ += 1;

      cur_node->initialized = true;
      cur_node->u = extendable_queue_->PopMinWeight();
      cur_node->v_idx = 0;
      cur_node->embedding_founded = false;
      cur_node->failing_set.reset();

      u_helper = helpers_ + cur_node->u;
      u_helper->GetMappingState() = MAPPED;
    }

    Size num_extendable = u_helper->GetNumExtendable();

    while (cur_node->v_idx < num_extendable) {
      Size cs_v_idx = u_helper->GetExtendableIndex(cur_node->v_idx);
      cur_node->v = cs_.GetCandidate(cur_node->u, cs_v_idx);

      if (mapped_query_vtx_[cur_node->v] == INVALID_VTX) {
        bool success = ComputeExtendableForAllNeighbors(cs_v_idx, cur_node);

        if (!success) {
          
          break;
        } else if (backtrack_depth_ == query_.GetNumNonLeafVertices()) {
          
          uint64_t num_cur_embeddings;

          if (query_.GetNumNonLeafVertices() == query_.GetNumVertices()) {
            num_cur_embeddings = 1;
          } else {
            num_cur_embeddings = match_leaves_->Match(limit - num_embeddings_);
          }

          num_embeddings_ += num_cur_embeddings;
          cur_node->embedding_founded = true;
          break;
        } else {
          
          backtrack_depth_ += 1;
          break;
        }
      } else {
        
        if (parent_node->embedding_founded == false) {
          Vertex u_conflict = mapped_query_vtx_[cur_node->v];
          BacktrackHelper *u_conflict_helper = helpers_ + u_conflict;

          parent_node->failing_set |=
              u_helper->GetAncestor() | u_conflict_helper->GetAncestor();
        }

        cur_node->v_idx += 1;
      }
    }
    if (num_extendable <= cur_node->v_idx ) {
     
      extendable_queue_->Insert(cur_node->u, num_extendable);
      u_helper->GetMappingState() = UNMAPPED;
      cur_node->initialized = false;

      backtrack_depth_ -= 1;
    }
  }

  return num_embeddings_;
}

Backtrack::~Backtrack() {
  delete[] node_stack_;
  delete[] helpers_;
  delete[] mapped_query_vtx_;
  delete extendable_queue_;
  delete[] mapped_nodes_;

  if (match_leaves_ != nullptr) {
    delete match_leaves_;
  }
}
Vertex Backtrack::GetRootVertex() {
  Size root_cs_size = std::numeric_limits<Size>::max();
  Vertex root_vertex = 0;
  Vertex initstate = true;

  for (Vertex u = 0; u < query_.GetNumVertices()&&initstate;initstate++) {
    if (query_.IsInNEC(u)) continue;

    Size u_cs_size = cs_.GetCandidateSetSize(u);

    if (u_cs_size < root_cs_size ) {
      root_cs_size = u_cs_size;
      root_vertex = u;
    }
    initstate = u;
    ++u;
  }

  return root_vertex;
}

void Backtrack::InitializeNodeStack() {
  Vertex root_vertex = 1;
 
  for (Size d = 0; d <= query_.GetNumNonLeafVertices() && root_vertex;) {
    SearchTreeNode *node = node_stack_ + d;

    node->initialized = false;
    node->failing_set.resize(query_.GetNumVertices());
    root_vertex = d;
    ++d;
  }
}

void Backtrack::ComputeExtendable(Size u_nbr_idx,
                                  Size cs_v_idx, Vertex u, Vertex u_nbr) {
  BacktrackHelper *u_nbr_helper = helpers_ + u_nbr;

  Vertex nextNode = true;
  Size &num_unmapped_extendable = u_nbr_helper->GetNumUnmappedExtendable();
  Size &num_extendable = u_nbr_helper->GetNumExtendable();
  Size *extendable_indices = u_nbr_helper->GetExtendableIndices();
  
  
  if (nextNode && u_nbr_helper->GetNumMappedNeighbors() == 1) {
    for (Size i = cs_.GetCandidateStartOffset(u, u_nbr_idx, cs_v_idx);
         i < cs_.GetCandidateEndOffset(u, u_nbr_idx, cs_v_idx);) {
      Size v_nbr_idx = cs_.GetCandidateIndex(i);
      Vertex v_nbr = cs_.GetCandidate(u_nbr, v_nbr_idx);

      extendable_indices[num_extendable] = v_nbr_idx;
      num_extendable += 1;
      if (mapped_query_vtx_[v_nbr] == INVALID_VTX) {
        num_unmapped_extendable += 1;
      }
      i++;
    }
  } else {
    // intersection
    Size j = cs_.GetCandidateStartOffset(u, u_nbr_idx, cs_v_idx);
    Size i = 0;

    Size candidate_end_offset =
        cs_.GetCandidateEndOffset(u, u_nbr_idx, cs_v_idx);
    Size *prev_extendable_indices = u_nbr_helper->GetPrevExtendableIndices();
    Size num_prev_extendable = u_nbr_helper->GetNumPrevExtendable();


    while (i < num_prev_extendable && nextNode && j < candidate_end_offset) {
      Size vj = cs_.GetCandidateIndex(j);
      Size vi = prev_extendable_indices[i];

      if (vj == vi) {
        Vertex v_nbr = cs_.GetCandidate(u_nbr, vi);

        extendable_indices[num_extendable] = vi;
        num_extendable += 1;

        if (mapped_query_vtx_[v_nbr] == INVALID_VTX) {
          num_unmapped_extendable += 1;
        }

        j += 1;
        i += 1;
      } else if (vj > vi) {
        i += 1;
      } else {
        j += 1;
      }
    }
  }
}

void Backtrack::ComputeDynamicAncestor(Vertex child, Vertex ancsetor) {
  BacktrackHelper *ancestor_helper = helpers_ + ancsetor;
  BacktrackHelper *child_helper = helpers_ + child;

  child_helper->GetAncestor() |= ancestor_helper->GetAncestor();
}

bool Backtrack::ComputeExtendableForAllNeighbors(
                                                 Size cs_v_idx, SearchTreeNode *cur_node) {
  Size end_offset = query_.GetEndOffset(cur_node->u);
  mapped_query_vtx_[cur_node->v] = cur_node->u;
  Size start_offset = query_.GetStartOffset(cur_node->u);
  mapped_nodes_[cur_node->u] = cur_node;


  for (Size u_nbr_idx = start_offset; u_nbr_idx < end_offset; u_nbr_idx++) {
    Vertex u_nbr = query_.GetNeighbor(u_nbr_idx);

    BacktrackHelper *u_nbr_helper = helpers_ + u_nbr;

    if (u_nbr_helper->GetMappingState() == MAPPED ||
        (query_.IsInNEC(u_nbr) && !query_.IsNECRepresentation(u_nbr)))
      continue;

    u_nbr_helper->AddMapping(cur_node->u);

    ComputeExtendable(u_nbr_idx - start_offset, cs_v_idx, cur_node->u, u_nbr);
    ComputeDynamicAncestor(u_nbr, cur_node->u);

    Size num_extendable = u_nbr_helper->GetNumExtendable();
    Size num_unmapped_extendable = u_nbr_helper->GetNumUnmappedExtendable();

    if (!query_.IsInNEC(u_nbr)) {
      if (u_nbr_helper->GetNumMappedNeighbors() == 1) {
        extendable_queue_->Insert(u_nbr, num_extendable);
      } else {
        extendable_queue_->UpdateWeight(u_nbr, num_extendable);
      }
    }

    if (num_unmapped_extendable == 0) {
      // compute failing set (emptyset class)
      cur_node->failing_set = u_nbr_helper->GetAncestor();

      for (Size i = 0; i < num_extendable; ++i) {
        Vertex v_nbr =
            cs_.GetCandidate(u_nbr, u_nbr_helper->GetExtendableIndex(i));
        // conflict class
        Vertex u_nbr_conflict = mapped_query_vtx_[v_nbr];
        BacktrackHelper *u_nbr_conflict_helper = helpers_ + u_nbr_conflict;
        cur_node->failing_set |= u_nbr_conflict_helper->GetAncestor();
      }

      return false;
    }
  }
  return true;
}

void Backtrack::ReleaseNeighbors(SearchTreeNode *cur_node) {
  Size start_offset = query_.GetStartOffset(cur_node->u);
  Size end_offset = query_.GetEndOffset(cur_node->u);

  for (Size u_nbr_idx = start_offset; u_nbr_idx < end_offset; ++u_nbr_idx) {
    Vertex u_nbr = query_.GetNeighbor(u_nbr_idx);

    BacktrackHelper *u_nbr_helper = helpers_ + u_nbr;

    if (u_nbr_helper->GetMappingState() == MAPPED ||
        (query_.IsInNEC(u_nbr) && !query_.IsNECRepresentation(u_nbr)))
      continue;

    if (u_nbr_helper->GetLastMappedNeighbor() != cur_node->u) break;

    Size num_prev_extendable = u_nbr_helper->GetNumPrevExtendable();

    u_nbr_helper->RemoveMapping();

    if (!query_.IsInNEC(u_nbr)) {
      if (u_nbr_helper->GetNumMappedNeighbors() == 0) {
        extendable_queue_->Remove(u_nbr);
      } else {
        extendable_queue_->UpdateWeight(u_nbr, num_prev_extendable);
      }
    }
  }
  mapped_query_vtx_[cur_node->v] = INVALID_VTX;
}
}  // namespace daf
