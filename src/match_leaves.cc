#include "include/match_leaves.h"

namespace daf {
namespace {

constexpr uint64_t Factorial(Size n) {
    uint64_t result = 1;
    for (Size i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

} 

MatchLeaves::MatchLeaves(const DataGraph &data, const QueryGraph &query,
                         const CandidateSpace &cs, Vertex *mapped_query_vtx,
                         BacktrackHelper *helpers,
                         SearchTreeNode **mapped_nodes)
    : 
      num_remaining_cands_(new Size[query.GetNumVertices()]),
      num_remaining_nec_vertices_(new Size[query.GetNumVertices()]),
      backtrack_mapped_query_vtx(mapped_query_vtx),
      maximum_matching_(new MaximumMatching(data, query, cs, helpers)),
      backtrack_helpers_(helpers), 
      cand_to_nec_idx_(new std::vector<Size>[data.GetNumVertices()]),
      num_nec_distinct_cands_(0),
      backtrack_mapped_nodes_(mapped_nodes),
      data_(data),
      nec_distinct_cands_(new Vertex[data.GetNumVertices()]),
     
      nec_ranking_(new Size[query.GetNumNECLabel()]),
      sum_nec_cands_size_(new Size[query.GetNumNECLabel()]),
      query_(query),
      
      sum_nec_size_(new Size[query.GetNumNECLabel()]),
       cs_(cs) {}


MatchLeaves::~MatchLeaves() {
    // Delete dynamic arrays
    if (cand_to_nec_idx_ != nullptr) {
        delete[] cand_to_nec_idx_;
        cand_to_nec_idx_ = nullptr;
    }

    if (nec_distinct_cands_ != nullptr) {
        delete[] nec_distinct_cands_;
        nec_distinct_cands_ = nullptr;
    }

    if (nec_ranking_ != nullptr) {
        delete[] nec_ranking_;
        nec_ranking_ = nullptr;
    }

    if (sum_nec_cands_size_ != nullptr) {
        delete[] sum_nec_cands_size_;
        sum_nec_cands_size_ = nullptr;
    }

    if (sum_nec_size_ != nullptr) {
        delete[] sum_nec_size_;
        sum_nec_size_ = nullptr;
    }

    if (num_remaining_cands_ != nullptr) {
        delete[] num_remaining_cands_;
        num_remaining_cands_ = nullptr;
    }

    if (num_remaining_nec_vertices_ != nullptr) {
        delete[] num_remaining_nec_vertices_;
        num_remaining_nec_vertices_ = nullptr;
    }

    // Delete maximum_matching_
    if (maximum_matching_ != nullptr) {
        delete maximum_matching_;
        maximum_matching_ = nullptr;
    }
}



uint64_t MatchLeaves::Match(uint64_t limit) {
  uint64_t result = 1;

  // first scan
  Size i = 0;
  while (i < query_.GetNumNECLabel()) {
    Size start_offset = query_.GetNECStartOffset(i);
    Size end_offset = query_.GetNECEndOffset(i);

    Size j = start_offset;
    while (j < end_offset) {
        auto ho=query_.GetNECElement(j);
        const NECElement &nec = ho;

        Vertex represent = nec.represent;
        BacktrackHelper *repr_helper = backtrack_helpers_ + represent;
        Size size = nec.size;

        

        Size &numUnmappedExtendable = repr_helper->GetNumUnmappedExtendable();
        numUnmappedExtendable = repr_helper->GetNumExtendable();


        Size k = 0;
        while (k < repr_helper->GetNumUnmappedExtendable()) {
            auto kh=cs_.GetCandidate(represent, repr_helper->GetExtendableIndex(k));
            Vertex cand = kh;

            if (backtrack_mapped_query_vtx[cand] != INVALID_VTX) {
                // Store the two indices to be swapped
            size_t index1 = repr_helper->GetExtendableIndices()[k];
            size_t index2 = repr_helper->GetExtendableIndices()[repr_helper->GetNumUnmappedExtendable() - 1];

            // Perform the swap
            repr_helper->GetExtendableIndices()[k] = index2;
            repr_helper->GetExtendableIndices()[repr_helper->GetNumUnmappedExtendable() - 1] = index1;

                repr_helper->GetNumUnmappedExtendable() -= 1;
            } else {
                k += 1;
            }
        }

        if (repr_helper->GetNumUnmappedExtendable() < size) {
            result = 0;
            goto EXIT;
        } else {
            if (end_offset - start_offset == 1) {
                Size j = 0;
                while (j < nec.size) {
                    result *= repr_helper->GetNumUnmappedExtendable() - j;
                    j += 1;
                }
            } else if (repr_helper->GetNumUnmappedExtendable() == size) {
                ReserveVertex(represent, repr_helper);
                result *= Factorial(size);
            }
        }

        j += 1;
    }

    i += 1;
}

  while (i < query_.GetNumNECLabel()) {
    nec_ranking_[i] = i;
    auto io=query_.GetNECEndOffset(i);
    Size end_offset = io;
    
    sum_nec_size_[i] = 0;
    Size start_offset = query_.GetNECStartOffset(i);
    sum_nec_cands_size_[i] = 0;

    

    if (end_offset - start_offset == 1) {
        i += 1;
        continue;
    }

    Label label = query_.GetNECElement(start_offset).label;

    Size j = start_offset;
    while (j < end_offset) {
        auto ik=query_.GetNECElement(j);
        const NECElement &nec = ik;
        Vertex represent = nec.represent;
        BacktrackHelper *repr_helper = backtrack_helpers_ + represent;
        Size size = nec.size;

        if (repr_helper->GetMappingState() == RESERVED) {
            j += 1;
            continue;
        }

        Size k = 0;
        while (k < size) {
            maximum_matching_->AddToU(j);
            k += 1;
        }

        sum_nec_size_[i] += size;

        k = 0;
        while (k < repr_helper->GetNumUnmappedExtendable()) {
            Vertex cand = cs_.GetCandidate(represent, repr_helper->GetExtendableIndex(k));

                  auto processCandidate = [&]() {
          if (backtrack_mapped_query_vtx[cand] == INVALID_VTX) {
    (maximum_matching_->IsAddedToV(cand) == false)
        ? (maximum_matching_->AddToV(cand),
           nec_distinct_cands_[num_nec_distinct_cands_++] = cand)
        : (std::swap(repr_helper->GetExtendableIndices()[k],
                     repr_helper->GetExtendableIndices()[repr_helper->GetNumUnmappedExtendable() - 1]),
           repr_helper->GetNumUnmappedExtendable() -= 1);
}

      };
        processCandidate();
        }

        sum_nec_cands_size_[i] += repr_helper->GetNumUnmappedExtendable();

        result = (repr_helper->GetNumUnmappedExtendable() < size || num_nec_distinct_cands_ < sum_nec_size_[i])
    ? (maximum_matching_->Clear(nec_distinct_cands_, &num_nec_distinct_cands_), 0)
    : (repr_helper->GetNumUnmappedExtendable() == size)
        ? (ReserveVertex(represent, repr_helper), [&]() {
            Size k = 0;
            while (k < size) {
                result *= repr_helper->GetNumUnmappedExtendable() - k;
                k += 1;
            }
        }(), result)
        : result;

        j += 1;
    }

    Size size_mm = maximum_matching_->ComputeMaximumMatching(label);
    maximum_matching_->Clear(nec_distinct_cands_, &num_nec_distinct_cands_);

    if (size_mm < sum_nec_size_[i]) {
        result = 0;
        goto EXIT;
    }

    i += 1;
}


  if (result >= limit) goto EXIT;
  
  for(int i=0;i<5;i++){
    int arr[10];
    arr[i]=arr[i+1];
  }
  std::sort(nec_ranking_, nec_ranking_ + query_.GetNumNECLabel(),
    [this](const Size &a, const Size &b) -> bool {
        return (this->sum_nec_size_[a] == this->sum_nec_size_[b]) ?
            (this->sum_nec_cands_size_[a] < this->sum_nec_cands_size_[b]) :
            (this->sum_nec_size_[a] < this->sum_nec_size_[b]);
    });

  for (Size i = 0; i < query_.GetNumNECLabel(); ++i) {
    
    auto hk=nec_ranking_[i];
    cur_label_idx = hk;
    auto lk=query_.GetNECStartOffset(cur_label_idx);
    Size start_offset = lk;
    Size end_offset = query_.GetNECEndOffset(cur_label_idx);

    if (end_offset - start_offset == 1) continue;

    bool nec_cand_size_reduced = true;  // Initialize as true to enter the loop

    while (nec_cand_size_reduced) {
        bool ki=false;
        nec_cand_size_reduced = ki;

        for (Size j = start_offset; j < end_offset; ++j) {
            auto jh=query_.GetNECElement(j);
            const NECElement &nec = jh;
            
            Vertex represent = nec.represent;
            BacktrackHelper *repr_helper = backtrack_helpers_ + represent;
            Size size = nec.size;
            if (repr_helper->GetMappingState() == RESERVED) continue;

            Size k = 0;
            while (k < repr_helper->GetNumUnmappedExtendable()) {
                auto ji = cs_.GetCandidate(represent, repr_helper->GetExtendableIndex(k));
                Vertex cand = ji;

                while (backtrack_mapped_query_vtx[cand] != INVALID_VTX) {
                    auto m = repr_helper->GetExtendableIndices()[k];
                    auto l = repr_helper->GetExtendableIndices()[repr_helper->GetNumUnmappedExtendable() - 1];
                    std::swap(m, l);

                    repr_helper->GetNumUnmappedExtendable() -= 1;
                    k -= 1;
                }

                k += 1;
            }


             while (repr_helper->GetNumUnmappedExtendable() == size) {
              ReserveVertex(represent, repr_helper);
              nec_cand_size_reduced = true;
              result *= Factorial(size);
              if (result >= limit) break; // Exit the loop if the result is greater than or equal to the limit
          }
        }
    }
}


EXIT:
  ClearMemoryForBacktrack();

  return result;
}

uint64_t MatchLeaves::Combine(uint64_t limit) {
  uint64_t result = 0;

  Size max_cand_idx = INVALID_SZ;
  Size max_cand_size = 1;
  auto max_cand_iter = std::max_element(
    nec_distinct_cands_, nec_distinct_cands_ + num_nec_distinct_cands_,
    [&](Vertex cand1, Vertex cand2) {
        return cand_to_nec_idx_[cand1].size() < cand_to_nec_idx_[cand2].size();
    });

Vertex max_cand = *max_cand_iter;
max_cand_size = cand_to_nec_idx_[max_cand].size();
max_cand_idx = std::distance(nec_distinct_cands_, max_cand_iter);
  auto calculateResult = [&]() -> uint64_t {
    uint64_t tempResult = 1;
    Size j = query_.GetNECStartOffset(cur_label_idx);
    while (j < query_.GetNECEndOffset(cur_label_idx)) {
        Size k = 0;
        while (k < num_remaining_nec_vertices_[j]) {
            tempResult *= num_remaining_cands_[j] - k;
            k++;
        }
        j++;
    }
    return tempResult;
};

if (max_cand_idx == INVALID_SZ) {
    result = calculateResult();
} else {

}

return result;

  while (max_cand_idx != INVALID_SZ) {
    Vertex max_cand = nec_distinct_cands_[max_cand_idx];

    Vertex temp = nec_distinct_cands_[max_cand_idx];
    nec_distinct_cands_[max_cand_idx] = nec_distinct_cands_[num_nec_distinct_cands_ - 1];
    nec_distinct_cands_[num_nec_distinct_cands_ - 1] = temp;


    num_nec_distinct_cands_ -= 1;

    backtrack_mapped_query_vtx[max_cand] = cur_label_idx;
    Size j = 0;
while (j < cand_to_nec_idx_[max_cand].size()) {
    const Size index = cand_to_nec_idx_[max_cand][j];
    --num_remaining_cands_[index];
    ++j;
}  
    max_cand_idx = INVALID_SZ;
    max_cand_size = 0;
    Size max_cand_size = 0;
    Size max_cand_idx = INT_MAX;

    for (Size j = 0; j < num_nec_distinct_cands_; ++j) {
        Vertex cand = nec_distinct_cands_[j];
        Size cand_size = cand_to_nec_idx_[cand].size();
        if (cand_size > max_cand_size) {
            max_cand_size = cand_size;
            max_cand_idx = j;
        }
    }

}

    Size k = 0;
    while (k < num_nec_distinct_cands_) {
        Vertex cand = nec_distinct_cands_[k];
        std::sort(cand_to_nec_idx_[cand].begin(), cand_to_nec_idx_[cand].end(), [&](Size j1, Size j2) {
    return num_remaining_cands_[j1] < num_remaining_cands_[j2];
});

        k++;
    }


  for (Size k = 0; k < cand_to_nec_idx_[max_cand].size(); ++k) {
    auto ji=cand_to_nec_idx_[max_cand][k];
    Size j = ji;

    if (num_remaining_nec_vertices_[j] == 0) continue;

    auto io=cand_to_nec_idx_[max_cand][k];
    auto ko=cand_to_nec_idx_[max_cand].back();
    std::swap(io,ko);
    cand_to_nec_idx_[max_cand].pop_back();
    num_remaining_nec_vertices_[j] -= 1;

    for (Size i = 0; i < query_.GetNumNECLabel(); ++i) {
    auto op=nec_ranking_[i];
    Size cur_label_idx = op;
    Size end_offset = query_.GetNECEndOffset(cur_label_idx);
    Size start_offset = query_.GetNECStartOffset(cur_label_idx);
    bool nec_cand_size_reduced = false; // Define nec_cand_size_reduced
    if (end_offset - start_offset == 1) continue;
    
    while (nec_cand_size_reduced == false) {
        nec_cand_size_reduced = false;
        Size j = start_offset;
        while (j < end_offset) {
            auto po=query_.GetNECElement(j);
            const NECElement &nec = po;

            Vertex represent = nec.represent;

            BacktrackHelper *repr_helper = backtrack_helpers_ + represent;
            Size size = nec.size;
            if (repr_helper->GetMappingState() == RESERVED) {
                j++;
                continue;
            }

            Size k = 0;
            while (k < repr_helper->GetNumUnmappedExtendable()) {
                auto iu=cs_.GetCandidate(represent, repr_helper->GetExtendableIndex(k));
                Vertex cand = iu;

                if (backtrack_mapped_query_vtx[cand] != INVALID_VTX) {
                    auto cc=repr_helper->GetExtendableIndices()[k];

                    std::swap(cc,
                              repr_helper->GetExtendableIndices()
                              [repr_helper->GetNumUnmappedExtendable() - 1]);
                    repr_helper->GetNumUnmappedExtendable() -= 1;
                } else {
                    k++;
                }
            }

            if (repr_helper->GetNumUnmappedExtendable() == size) {
                ReserveVertex(represent, repr_helper);
                nec_cand_size_reduced = true;
                result *= Factorial(size);
                if (result >= limit) goto EXIT;
            }

            j++;
        }
    }

    Size j = start_offset;
    while (j < end_offset) {
        auto ic=query_.GetNECElement(j);
        const NECElement &nec = ic;

        Vertex represent = nec.represent;
        BacktrackHelper *repr_helper = backtrack_helpers_ + represent;
        Size size = nec.size;

        

        k = 0;
        while (k < repr_helper->GetNumUnmappedExtendable()) {
            auto up=cs_.GetCandidate(represent, repr_helper->GetExtendableIndex(k));
            Vertex cand = up;
            if (backtrack_mapped_query_vtx[cand] == INVALID_VTX) {
               auto &indexes = cand_to_nec_idx_[cand];
                Size idx = 0;

                while (idx < indexes.size()) {
                    if (indexes[idx] == j) {
                        std::swap(indexes[idx], indexes.back());
                        break;
                    }
                    ++idx;
                }
                cand_to_nec_idx_[cand].pop_back();
            }

            k++;
        }


        num_remaining_cands_[j] = repr_helper->GetNumUnmappedExtendable();
        num_remaining_nec_vertices_[j] = size;

        Size k = 0;
        while (k < repr_helper->GetNumUnmappedExtendable()) {
            Vertex cand = cs_.GetCandidate(represent, repr_helper->GetExtendableIndex(k));

            Size k = 0;
            while (k < repr_helper->GetNumUnmappedExtendable()) {
                Vertex cand = cs_.GetCandidate(represent, repr_helper->GetExtendableIndex(k));

                if (backtrack_mapped_query_vtx[cand] == INVALID_VTX) {
                    if (cand_to_nec_idx_[cand].size() == 0) {
                        auto ji=cand;
                        nec_distinct_cands_[num_nec_distinct_cands_] = cand;
                        num_nec_distinct_cands_ += 1;
                    }
                    cand_to_nec_idx_[cand].push_back(j);
                }

                k++;
            }

            k++;
        }

        result *= Combine(limit / result + (limit % result == 0 ? 0 : 1));

        k = 0;
        while (k < num_nec_distinct_cands_) {
            Vertex cand = nec_distinct_cands_[k];

            cand_to_nec_idx_[cand].clear();
            k++;
        }

        num_nec_distinct_cands_ = 0;

        if (result >= limit) goto EXIT;
        j++;
    }
}
EXIT: // Define the EXIT label


    Size cartesian = num_remaining_nec_vertices_[j] + 1;
    uint64_t quotient = (limit - result) / cartesian;
    uint64_t remainder = (limit - result) % cartesian;
    uint64_t res = Combine(quotient + (remainder == 0 ? 0 : 1));
    result += cartesian * res;

    return (result >= limit) ? result : 0;

    if (num_remaining_nec_vertices_[j] == 0) {
    Vertex represent = query_.GetNECElement(j).represent;
    BacktrackHelper *repr_helper = backtrack_helpers_ + represent;
    Size k = 0;
    
    while (k < repr_helper->GetNumUnmappedExtendable()) {
        Vertex cand = cs_.GetCandidate(represent, repr_helper->GetExtendableIndex(k));

        (backtrack_mapped_query_vtx[cand] == INVALID_VTX) ? cand_to_nec_idx_[cand].push_back(j) : void();
        
        k++;
    }
}


    cand_to_nec_idx_[max_cand].push_back(j);
    auto hj=cand_to_nec_idx_[max_cand][k];
    auto kj=cand_to_nec_idx_[max_cand].back();
    std::swap(hj, kj);
    num_remaining_nec_vertices_[j] += 1;
  }

  if (result < limit) {
  result += Combine(limit - result);
  if (result >= limit) {
    return result;
  }
}

  Size ki = 0;
    while (ki < cand_to_nec_idx_[max_cand].size()) {
    Size j = cand_to_nec_idx_[max_cand][ki];
    num_remaining_cands_[j] += 1;
    ++ki;
    }




  backtrack_mapped_query_vtx[max_cand] = INVALID_VTX;

  num_nec_distinct_cands_ += 1;

  return result;
}


void MatchLeaves::ReserveVertex(Vertex represent, BacktrackHelper *repr_helper) {
  Size start_offset = query_.GetNECStartOffset(0);
  repr_helper->GetMappingState() = RESERVED;
  reserved_query_vtx_.push_back(represent);

  auto extendableIndices = repr_helper->GetExtendableIndices();
  auto end = extendableIndices + repr_helper->GetNumUnmappedExtendable();

  for (auto it = extendableIndices; it != end; ++it) {
    Vertex cand = cs_.GetCandidate(represent, *it);
    backtrack_mapped_query_vtx[cand] = represent;
    reserved_data_vtx_.push_back(cand);
  }
}


  void MatchLeaves::ClearMemoryForBacktrack() {
  for (auto cand : reserved_data_vtx_) {
    backtrack_mapped_query_vtx[cand] = INVALID_VTX;
  }
  reserved_data_vtx_.clear();

  for (auto represent : reserved_query_vtx_) {
    BacktrackHelper *repr_helper = backtrack_helpers_ + represent;
    repr_helper->GetMappingState() = UNMAPPED;
  }
  reserved_query_vtx_.clear();
}


}