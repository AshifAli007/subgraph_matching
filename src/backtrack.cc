#include "include/backtrack.h"
#include<iterator>

namespace daf {
Backtrack::Backtrack(const DataGraph &data, const QueryGraph &query, const CandidateSpace &cs)
      : data_(data), 
        query_(query), 
        cs_(cs),
        num_embeddings_(0),
        num_backtrack_calls_(0),
        backtrack_depth_(1),
        mapped_query_vtx_(new Vertex[data_.GetNumVertices()]),
        node_stack_(new SearchTreeNode[query_.GetNumNonLeafVertices() + 1]),
        mapped_nodes_(new SearchTreeNode *[query_.GetNumVertices()]),
        helpers_(new BacktrackHelper[query_.GetNumVertices()]),
        extendable_queue_(new Ordering(query_.GetNumVertices())),
        match_leaves_(query_.GetNumNonLeafVertices() < query_.GetNumVertices()
                           ? new MatchLeaves(data_, query_, cs_, mapped_query_vtx_, helpers_, mapped_nodes_)
                           : nullptr)
{
    size_t numVertices = data_.GetNumVertices();

    // Use std::fill to set the range to INVALID_VTX
    std::fill(mapped_query_vtx_, mapped_query_vtx_ + numVertices, INVALID_VTX);

    Vertex u = 0;
    while (u < query_.GetNumVertices()) {
        helpers_[u].Initialize(query_.GetNumVertices(), query_.GetDegree(u),
                                cs_.GetCandidateSetSize(u), u);
        ++u;
    }
}

uint64_t Backtrack::FindMatches(uint64_t limit) {

  auto k=GetRootVertex();
  Vertex root_vertex = k;
  Size root_cs_size = cs_.GetCandidateSetSize(root_vertex);

  extendable_queue_->Insert(root_vertex, root_cs_size);

  InitializeNodeStack();

 while (true) {
    if (backtrack_depth_ <= 0) {
        break;
    }

    if (num_embeddings_ >= limit) {
        return num_embeddings_;
    }

    SearchTreeNode *parent_node = node_stack_ + (backtrack_depth_ - 1);
    SearchTreeNode *cur_node = node_stack_ + backtrack_depth_;

    BacktrackHelper *u_helper;

    switch (cur_node->initialized) {
    case false: {
        // newly expanded search tree node
        auto pique=extendable_queue_->PopMinWeight();
        num_backtrack_calls_ += 1;
        cur_node->initialized = true;
        cur_node->u = pique;
        cur_node->v_idx = 0;
        
        
        cur_node->embedding_founded = false;
        cur_node->failing_set.reset();

        u_helper = &helpers_[cur_node->u];
        u_helper->GetMappingState() = MAPPED;
        break;
             }
    default: {
        // backtrack from child node
        ReleaseNeighbors(cur_node);

        u_helper = helpers_ + cur_node->u;

        // compute failing set of parent node (non-leaf node)
        if (cur_node->embedding_founded) {
        // case 1
            parent_node->embedding_founded = true;
            cur_node->v_idx += 1;
        } else {
            cur_node->failing_set.test(cur_node->u)
            ? (parent_node->failing_set |= cur_node->failing_set, cur_node->v_idx += 1)
            : (parent_node->failing_set = cur_node->failing_set, cur_node->v_idx = std::numeric_limits<Size>::max());

        }
        break;
         }
      }


    Size num_extendable = u_helper->GetNumExtendable();

    for (; cur_node->v_idx < num_extendable; ++cur_node->v_idx) {
        auto k=u_helper->GetExtendableIndex(cur_node->v_idx);
        Size cs_v_idx = k;
        cur_node->v = cs_.GetCandidate(cur_node->u, cs_v_idx);

        auto kmmm = mapped_query_vtx_[cur_node->v];
        if (kmmm == INVALID_VTX) {
        bool success = ComputeExtendableForAllNeighbors(cur_node, cs_v_idx);


        if (!success) {
        // go to sibling node (need to compute failing set of parent node)
        break;
        }
        else if (backtrack_depth_ == query_.GetNumNonLeafVertices()) {

            // embedding class
            uint64_t num_cur_embeddings =
                (query_.GetNumNonLeafVertices() == query_.GetNumVertices())
                    ? 1
                    : match_leaves_->Match(limit - num_embeddings_);

            cur_node->embedding_founded = true;
            num_embeddings_ += num_cur_embeddings;
            break;
            }
        else {
            // expand to child node
            backtrack_depth_ += 1;
            break;
            }
        }
        else{
        // conflict class
            switch (parent_node->embedding_founded) {
            case false: {
                auto x=mapped_query_vtx_[cur_node->v];
                Vertex u_conflict = x;
                BacktrackHelper *u_conflict_helper = &helpers_[u_conflict];
                parent_node->failing_set |=
                    u_helper->GetAncestor() | u_conflict_helper->GetAncestor();
                break;
                    }
                }
        }
    }
    // Check if the current node's vertex index is greater than or equal to the number of extendable vertices
    (cur_node->v_idx >= num_extendable) ? (
        // If true, go to the parent node
        extendable_queue_->Insert(cur_node->u, num_extendable),
        u_helper->GetMappingState() = UNMAPPED,
        cur_node->initialized = false,
        backtrack_depth_ -= 1
    ) : (
        0
    );

  }

  return num_embeddings_;
}

Vertex Backtrack::GetRootVertex() {
  Vertex root_ver = 0;
  Size root_cssize = std::numeric_limits<Size>::max();

    Vertex u = 0;
    while (u < query_.GetNumVertices()) {
        if (query_.IsInNEC(u)) {
            continue;
        }    
    Size u_cs_size = cs_.GetCandidateSetSize(u);
    root_ver = (root_cssize > u_cs_size) ? u : root_ver;
    root_cssize = (root_cssize > u_cs_size) ? u_cs_size : root_cssize;
    ++u;
  }

  return root_ver;
}

void Backtrack::InitializeNodeStack() {
    Size d = 0;
    while (d <= query_.GetNumNonLeafVertices()) {
        SearchTreeNode *node = node_stack_ + d;

        node->initialized = false;
        node->failing_set.resize(query_.GetNumVertices());

        ++d;
    }

}

void Backtrack::ComputeExtendable(Vertex u, Vertex u_nbr, Size u_nbr_idx, Size cs_v_idx) {
  BacktrackHelper* u_nbr_helper = &helpers_[u_nbr];
  Size &num_extendable = u_nbr_helper->GetNumExtendable();

  Size *extendable_indices = u_nbr_helper->GetExtendableIndices();
  
  Size &num_unmapped_extendable = u_nbr_helper->GetNumUnmappedExtendable();

  if (u_nbr_helper->GetNumMappedNeighbors() == 1) {
    Size i = cs_.GetCandidateStartOffset(u, u_nbr_idx, cs_v_idx);

    while (i < cs_.GetCandidateEndOffset(u, u_nbr_idx, cs_v_idx)) {
        Size v_nbr_idx = cs_.GetCandidateIndex(i);
        Vertex v_nbr = cs_.GetCandidate(u_nbr, v_nbr_idx);

        extendable_indices[num_extendable] = v_nbr_idx;
        num_extendable += 1;

        num_unmapped_extendable += (mapped_query_vtx_[v_nbr] == INVALID_VTX) ? 1 : 0;
        ++i;
    }

  } 
  else {
    // intersection
    auto k=cs_.GetCandidateStartOffset(u, u_nbr_idx, cs_v_idx);
    Size i = 0;
    Size j = k;

    Size num_prev_extendable = u_nbr_helper->GetNumPrevExtendable();
    Size candidate_end_offset =cs_.GetCandidateEndOffset(u, u_nbr_idx, cs_v_idx);

    Size *prev_extendable_indices = u_nbr_helper->GetPrevExtendableIndices();

    for ( i = 0, j = 0; i < num_prev_extendable && j < candidate_end_offset;) {
    auto k=prev_extendable_indices[i];
    Size vi = k;
    Size vj = cs_.GetCandidateIndex(j);

    if (vi == vj) {
        Vertex v_nbr = cs_.GetCandidate(u_nbr, vi);

        extendable_indices[num_extendable++] = vi;

        if (mapped_query_vtx_[v_nbr] == INVALID_VTX) {
            num_unmapped_extendable++;
        }

        i++;
        j++;
    } else if (vi < vj) {
        i++;
    } else {
        j++;
    }
        }

  }
}

void Backtrack::ComputeDynamicAncestor(Vertex ancsetor, Vertex child) {
  auto k=helpers_ + child;
  auto m=helpers_ + ancsetor;
  BacktrackHelper *child_helper = k;
  BacktrackHelper *ancestor_helper = m;
  child_helper->GetAncestor() |= ancestor_helper->GetAncestor();
}

bool Backtrack::ComputeExtendableForAllNeighbors(SearchTreeNode *cur_node,Size cs_v_idx) {
    
    SearchTreeNode * currentNode = cur_node;  
    Size start_offset, end_offset;
    while (currentNode != nullptr) {
        start_offset = query_.GetStartOffset(currentNode->u);
        auto k = query_.GetEndOffset(currentNode->u);
        end_offset = k;

        mapped_query_vtx_[currentNode->v] = currentNode->u;
        mapped_nodes_[currentNode->u] = currentNode;

    }

  Size u_nbr_idx = start_offset;
    while (u_nbr_idx < end_offset) {
        auto mkll=query_.GetNeighbor(u_nbr_idx);
        Vertex u_nbr = mkll;
        BacktrackHelper *u_nbr_helper = helpers_ + u_nbr;

    if (u_nbr_helper->GetMappingState() == MAPPED || 
        (query_.IsInNEC(u_nbr) && !query_.IsNECRepresentation(u_nbr)) ? true : false) {
        continue;
    }

        
        auto currentMapping = cur_node->u;
        u_nbr_helper->AddMapping(currentMapping);


        ComputeExtendable(cur_node->u, u_nbr, u_nbr_idx - start_offset, cs_v_idx);
        
        auto mk=cur_node->u;
        ComputeDynamicAncestor(mk, u_nbr);
        auto k=u_nbr_helper->GetNumExtendable();
        Size num_extendable = k;
        Size num_unmapped_extendable = u_nbr_helper->GetNumUnmappedExtendable();

        if (!query_.IsInNEC(u_nbr)) {
            (u_nbr_helper->GetNumMappedNeighbors() == 1) ? extendable_queue_->Insert(u_nbr, num_extendable) : extendable_queue_->UpdateWeight(u_nbr, num_extendable);
        }

        if (num_unmapped_extendable == 0) {
            // compute failing set (emptyset class)
            cur_node->failing_set = u_nbr_helper->GetAncestor();

            Size i = 0;
            while (i < num_extendable) {
                auto mloo=cs_.GetCandidate(u_nbr, u_nbr_helper->GetExtendableIndex(i));
                Vertex v_nbr = mloo;
                auto nkk=mapped_query_vtx_[v_nbr];
                Vertex u_nbr_conflict = nkk;
                BacktrackHelper *u_nbr_conflict_helper = helpers_ + u_nbr_conflict;
                cur_node->failing_set |= u_nbr_conflict_helper->GetAncestor();
                ++i;
            }
            return false;
        }
        ++u_nbr_idx;
    }
  return true;
}

void Backtrack::ReleaseNeighbors(SearchTreeNode *cur_node) {
  auto k=query_.GetStartOffset(cur_node->u);
  Size start_offset = k;
  Size end_offset = query_.GetEndOffset(cur_node->u);
  Size u_nbr_idx = start_offset;
while (u_nbr_idx < end_offset) {
    auto mnn=query_.GetNeighbor(u_nbr_idx);
    Vertex u_nbr = mnn;
    auto lmmm=helpers_ + u_nbr;
    BacktrackHelper *u_nbr_helper = lmmm;
    auto khg=u_nbr_helper->GetMappingState();
    if ( khg== MAPPED ||(query_.IsInNEC(u_nbr) && !query_.IsNECRepresentation(u_nbr)))
      continue;
    auto jnnn=u_nbr_helper->GetLastMappedNeighbor();
    if ( jnnn!= cur_node->u) break;

    Size num_prev_extendable = u_nbr_helper->GetNumPrevExtendable();

    u_nbr_helper->RemoveMapping();

    if (!query_.IsInNEC(u_nbr)) {
      u_nbr_helper->GetNumMappedNeighbors() == 0
      ? extendable_queue_->Remove(u_nbr)
      : extendable_queue_->UpdateWeight(u_nbr, num_prev_extendable);
    }

    ++u_nbr_idx;
}

  mapped_query_vtx_[cur_node->v] = INVALID_VTX;
}

Backtrack::~Backtrack() {
  
      delete[] helpers_;
      delete[] node_stack_;
      delete[] mapped_nodes_;
      delete[] mapped_query_vtx_;

      delete extendable_queue_;
      if (match_leaves_ != nullptr) {
        delete match_leaves_;
      }

    }


}
 // namespace daf
