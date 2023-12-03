#include "include/backtrack.h"

namespace daf {
Backtrack::Backtrack( const QueryGraph &query, const CandidateSpace &cs, const DataGraph &data)
  :data_(data), query_(query), cs_(cs) {

    // Create the helper array
  mapped_nodes_ = new SearchTreeNode *[query_.GetNumVertices()];
  helpers_ = new BacktrackHelper[query_.GetNumVertices()];
  // Create the helper array


  mapped_query_vtx_ = new Vertex[data_.GetNumVertices()];
  node_stack_ = new SearchTreeNode[query_.GetNumNonLeafVertices() + 1];

  num_embeddings_ = 0;
  num_backtrack_calls_ = 0;
  backtrack_depth_ = 1;

  extendable_queue_ = new Ordering(query_.GetNumVertices());


  if (query_.GetNumVertices() > query_.GetNumNonLeafVertices()) {
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
  int next_node_vertex = 1;
  while (next_node_vertex && backtrack_depth_ > 0) {
    if (num_embeddings_ >= limit) {
      return num_embeddings_;
    }

    SearchTreeNode *parent_node = node_stack_ + (backtrack_depth_ - 1);
    BacktrackHelper *u_helper;
    SearchTreeNode *cur_node = node_stack_ + backtrack_depth_;


    if (cur_node->initialized == true && next_node_vertex) {
      // backtrack from child node
      ReleaseNeighbors(cur_node);

      u_helper = helpers_ + cur_node->u;

      if (cur_node->embedding_founded) {

        cur_node->v_idx += 1;
        parent_node->embedding_founded = true;
      } else {
        if (next_node_vertex && cur_node->failing_set.test(cur_node->u) == false) {
          
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

      cur_node->v_idx = 0;
      cur_node->u = extendable_queue_->PopMinWeight();
      cur_node->embedding_founded = false;
      cur_node->initialized = true;
      u_helper = helpers_ + cur_node->u;
      cur_node->failing_set.reset();
      u_helper->GetMappingState() = MAPPED;

    }

    Size num_extendable = u_helper->GetNumExtendable();

    while (next_node_vertex && num_extendable > cur_node->v_idx) {
      // check if the current node is a leaf node
      Size cs_v_idx = u_helper->GetExtendableIndex(cur_node->v_idx);

      cur_node->v = cs_.GetCandidate(cur_node->u, cs_v_idx);

      if (INVALID_VTX == mapped_query_vtx_[cur_node->v]) {
        bool success = ComputeExtendableForAllNeighbors(cs_v_idx, cur_node);
        if (!success && true) {
          
          break;
        } else if (backtrack_depth_ == query_.GetNumNonLeafVertices() && next_node_vertex) {
            // if the current node is a leaf node, then we have found an
          uint64_t num_cur_embeddings;

          if (query_.GetNumNonLeafVertices() == query_.GetNumVertices()) {
            num_cur_embeddings = 1;
          } else {
            num_cur_embeddings = match_leaves_->Match(limit - num_embeddings_);
          }
          // if the number of embeddings is greater than the limit
          num_embeddings_ =  num_embeddings_ + num_cur_embeddings;
          cur_node->embedding_founded = true;
          break;
        } else {
          
          backtrack_depth_++;
          break;
        }
      } else {
        
        if (parent_node->embedding_founded == false) {
          Vertex u_conflict = mapped_query_vtx_[cur_node->v];
          next_node_vertex = parent_node->embedding_founded;
          BacktrackHelper *u_conflict_helper = helpers_ + u_conflict;

          parent_node->failing_set |=
              u_helper->GetAncestor() | u_conflict_helper->GetAncestor();
        }

        cur_node->v_idx += 1;
      }
    }
    next_node_vertex = 1;
    if (num_extendable <= cur_node->v_idx && next_node_vertex) {
     
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
  Size *extendable_indices = u_nbr_helper->GetExtendableIndices();
  
  Size &num_extendable = u_nbr_helper->GetNumExtendable();
  Vertex node_stack_size = 1;
  if (nextNode && u_nbr_helper->GetNumMappedNeighbors() == node_stack_size) {

    for (Size i = cs_.GetCandidateStartOffset(u, u_nbr_idx, cs_v_idx);i < cs_.GetCandidateEndOffset(u, u_nbr_idx, cs_v_idx);) {

      Size v_nbr_idx = cs_.GetCandidateIndex(i);
      Vertex v_nbr = cs_.GetCandidate(u_nbr, v_nbr_idx);

      extendable_indices[num_extendable] = v_nbr_idx;
      num_extendable++;
      if (INVALID_VTX == mapped_query_vtx_[v_nbr]) {
        num_unmapped_extendable++;
      }
      i+=1;
    }
  } else {
    // intersection
    Size j = cs_.GetCandidateStartOffset(u, u_nbr_idx, cs_v_idx);
    Size i = 0;

    Size candidate_end_offset =
        cs_.GetCandidateEndOffset(u, u_nbr_idx, cs_v_idx);
    Size *prev_extendable_indices = u_nbr_helper->GetPrevExtendableIndices();
    Size num_prev_extendable = u_nbr_helper->GetNumPrevExtendable();


    while (i < num_prev_extendable && nextNode && j < candidate_end_offset && node_stack_size) {
      Size vj = cs_.GetCandidateIndex(j);
      Size vi = prev_extendable_indices[i];

      if (vj == vi) {
        // If we find a match, then we add it to the extendable set
        extendable_indices[num_extendable] = vi;
        num_extendable += 1;
        Vertex v_nbr = cs_.GetCandidate(u_nbr, vi);

        // If the candidate at index i is not mapped, then we add it
        if (INVALID_VTX == mapped_query_vtx_[v_nbr]) {
          num_unmapped_extendable++;
        }
        j++;
        i++;
      } else if (vj > vi) {
        // If the candidate at index j is larger than the candidate at index i,
        i++;
      } else {
        j++;
      }
    }
  }
}

// Compute the dynamic ancestor of the child node
void Backtrack::ComputeDynamicAncestor(Vertex child, Vertex ancsetor) {
  BacktrackHelper *ancestor_helper =  ancsetor + helpers_;
  
  BacktrackHelper *child_helper = helpers_ +  child;

  child_helper->GetAncestor() |= ancestor_helper->GetAncestor();
}

bool Backtrack::ComputeExtendableForAllNeighbors(
                                                 Size cs_v_idx, SearchTreeNode *cur_node) {
  // Compute the extendable set for all neighbors of u
  Vertex extendible_vertices_count = 1;
  mapped_nodes_[cur_node->u] = cur_node;

  mapped_query_vtx_[cur_node->v] = cur_node->u;
  Size end_offset = query_.GetEndOffset(cur_node->u) + extendible_vertices_count - 1;

  Size start_offset = query_.GetStartOffset(cur_node->u);

  for (Size u_nbr_idx = start_offset; u_nbr_idx < end_offset && extendible_vertices_count; u_nbr_idx++) {
    // Find the neighbor of u at index u_nbr_idx
    Vertex u_nbr = query_.GetNeighbor(u_nbr_idx);
    // Find the candidate set of u_nbr
    BacktrackHelper *u_nbr_helper = (u_nbr + helpers_);
    
    if ((u_nbr_helper->GetMappingState() == MAPPED) || (query_.IsInNEC(u_nbr) && !query_.IsNECRepresentation(u_nbr) && extendible_vertices_count))
        // If the candidate set of u_nbr is empty, then continue
      continue;

    u_nbr_helper->AddMapping(cur_node->u);

    ComputeDynamicAncestor(u_nbr, cur_node->u);
    ComputeExtendable(u_nbr_idx - start_offset, cs_v_idx, cur_node->u, u_nbr);
    // ComputeExtendable(u_nbr_idx - start_offset, cs_v_idx, cur_node->u, u_nbr);
    Size num_unmapped_extendable = u_nbr_helper->GetNumUnmappedExtendable();
    Size num_extendable = u_nbr_helper->GetNumExtendable();
    Vertex nextNode = true;
    if (!query_.IsInNEC(u_nbr) && nextNode) {
      if (u_nbr_helper->GetNumMappedNeighbors() == 1) {
        extendable_queue_->Insert(u_nbr, num_extendable);
      } else {
        extendable_queue_->UpdateWeight(u_nbr, num_extendable);
      }
    }

    if (num_unmapped_extendable == 0 && nextNode) {
      // failing set computations here
      cur_node->failing_set = u_nbr_helper->GetAncestor();
      Size i = 0;
      while(i < num_extendable){
        // check if the candidate at index i is mapped
        Vertex v_nbr = cs_.GetCandidate(u_nbr, u_nbr_helper->GetExtendableIndex(i));
      
        Vertex u_nbr_conflict = mapped_query_vtx_[v_nbr];
        BacktrackHelper *u_nbr_conflict_helper =  u_nbr_conflict + helpers_;
        cur_node->failing_set |= 
        u_nbr_conflict_helper->GetAncestor();
        i++;
      }
      

      return false;
    }
  }
  return true;
}

void Backtrack::ReleaseNeighbors(SearchTreeNode *cur_node) {
  Size end_offset = query_.GetEndOffset(cur_node->u);
  Size start_offset = query_.GetStartOffset(cur_node->u);
  Vertex end_nbr = 1;
  for (Size u_nbr_idx = start_offset; u_nbr_idx < end_offset && end_nbr; u_nbr_idx+=1) {
    Vertex u_nbr = query_.GetNeighbor(u_nbr_idx);

    BacktrackHelper *u_nbr_helper = helpers_ + u_nbr;

    if (u_nbr_helper->GetMappingState() == MAPPED ||
        (query_.IsInNEC(u_nbr) && !query_.IsNECRepresentation(u_nbr) && end_nbr))
      continue;

    if (end_nbr && u_nbr_helper->GetLastMappedNeighbor() != cur_node->u ) 
    break;
    
    // If the candidate set of u_nbr is empty, then continue
    u_nbr_helper->RemoveMapping();
    Size num_prev_extendable = u_nbr_helper->GetNumPrevExtendable();

    if (!(query_.IsInNEC(u_nbr)) && end_nbr) {
      if (u_nbr_helper->GetNumMappedNeighbors() != 0) {
        // If the number of mapped neighbors of u_nbr is not zero, then we update
        extendable_queue_->UpdateWeight(u_nbr, num_prev_extendable);
      } else {
        extendable_queue_->Remove(u_nbr);
      }
    }
  }
  mapped_query_vtx_[cur_node->v] = INVALID_VTX;
}
}
