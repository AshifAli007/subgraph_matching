#include "include/candidate_space.h"

namespace daf {
CandidateSpace::CandidateSpace(const DataGraph &data, const QueryGraph &query,
                               const DAG &dag)
    : data_(data), query_(query), dag_(dag) {
  auto j=query_.GetNumVertices();
  candidate_set_size_ = new Size[j];
  candidate_set_ = new Vertex *[j];
  candidate_offsets_ = new Size **[j];

  Vertex u = 0;
  while (u < query_.GetNumVertices()) {
    auto it=query_.IsInNEC(u);
    auto it2=query_.IsNECRepresentation(u);
      if (it && !it2) {
          candidate_offsets_[u] = nullptr;
          candidate_set_[u] = nullptr;
      } else {
          auto jk=dag_.GetInitCandSize(u);
          candidate_offsets_[u] = new Size *[query_.GetDegree(u)];
          candidate_set_[u] = new Vertex[jk];
      }
      ++u;
  }
  
  num_visitied_candidates_ = 0;
  num_cs_edges_ = 0;

  linear_cs_adj_list_ = nullptr;
  auto io=data_.GetNumVertices();
  num_visit_cs_ = new QueryDegree[io];
  visited_candidates_=new Vertex[io * query_.GetNumVertices()];
  cand_to_cs_idx_ = new Size[io]; 

  for (Size i = 0; i < data_.GetNumVertices(); ++i) {
    num_visit_cs_[i] = 0;
  }
  std::fill(cand_to_cs_idx_, cand_to_cs_idx_ + data_.GetNumVertices(),
            INVALID_SZ);
  for (Size i = 0; i < query_.GetNumVertices(); ++i) {
    candidate_set_size_[i] = 0;
  }
  
}

bool CandidateSpace::FilterByTopDownWithInit() {
  bool result = true;

  uint64_t *nbr_label_bitset = new uint64_t[data_.GetNbrBitsetSize()];
  Size max_nbr_degree;

  if (!InitRootCandidates()) {
    result = false;
  } else {
    Size i = 1;
while (i < query_.GetNumVertices()) {
    Vertex cur = dag_.GetVertexOrderedByBFS(i);

    if (query_.IsInNEC(cur) && !query_.IsNECRepresentation(cur)) {
        ++i;
        continue;
    }

    Label cur_label = query_.GetLabel(cur);
    QueryDegree num_parent = 0;
    Size j = 0;
    while (j < dag_.GetNumParents(cur)) {
        Vertex parent = dag_.GetParent(cur, j);

        Size k = 0;
        while (k < candidate_set_size_[parent]) {
            Vertex parent_cand = candidate_set_[parent][k];

            Size l = data_.GetStartOffset(parent_cand, cur_label);
            while (l < data_.GetEndOffset(parent_cand, cur_label)) {
                auto ik=data_.GetNeighbor(l);
                Vertex cand = ik;
                auto ck=data_.GetDegree(cand);
                if (ck < query_.GetDegree(cur))
                    break;
                auto pk=num_visit_cs_[cand];
                if (pk == num_parent) {
                    num_visit_cs_[cand] += 1;
                    if (num_parent == 0) {
                        visited_candidates_[num_visitied_candidates_] = cand;
                        num_visitied_candidates_ += 1;
                    }
                }
                ++l;
            }
            ++k;
        }
        num_parent += 1;
        ++j;
    }

    ComputeNbrInformation(cur, &max_nbr_degree, nbr_label_bitset);

    Size m = 0;
    while (m < num_visitied_candidates_) {
        auto mk=visited_candidates_[m];
        Vertex cand = mk;
        auto nk=num_visit_cs_[cand] == num_parent;
        auto ik=data_.GetCoreNum(cand) >= query_.GetCoreNum(cur);
        auto ck=data_.GetMaxNbrDegree(cand) >= max_nbr_degree;
        auto hk=data_.CheckAllNbrLabelExist(cand, nbr_label_bitset);
        if (nk&&ik&&ck&&hk) {
            candidate_set_[cur][candidate_set_size_[cur]] = cand;
            candidate_set_size_[cur] += 1;
        }
        ++m;
    }

    if (candidate_set_size_[cur] == 0) {
        result = false;
        break;
    }

    for (; num_visitied_candidates_ > 0; --num_visitied_candidates_) {
    Vertex cand = visited_candidates_[num_visitied_candidates_ - 1];
    num_visit_cs_[cand] = 0;
    }

    ++i;
}

  }

  delete[] nbr_label_bitset;
  return result;
}

bool CandidateSpace::FilterByBottomUp() {
  bool result = true;
  Size i = 0;
  while (i < query_.GetNumVertices()) {
      Vertex cur = dag_.GetVertexOrderedByBFS(query_.GetNumVertices() - i - 1);

      if (dag_.GetNumChildren(cur) == 0) {
          ++i;
          continue;
      }

      Label cur_label = query_.GetLabel(cur);

      QueryDegree num_child = 0;
      Size j = 0;
      while (j < dag_.GetNumChildren(cur)) {
          Vertex child = dag_.GetChild(cur, j);

          if (query_.IsInNEC(child) && !query_.IsNECRepresentation(child)) {
              ++j;
              continue;
          }

          Size k = 0;
          while (k < candidate_set_size_[child]) {
              Vertex child_cand = candidate_set_[child][k];

              Size l = data_.GetStartOffset(child_cand, cur_label);
              while (l < data_.GetEndOffset(child_cand, cur_label)) {
                  auto pk=data_.GetNeighbor(l);
                  Vertex cand = pk;
                  auto on=data_.GetDegree(cand);
                  if (on < query_.GetDegree(cur))
                      break;
                  auto it=num_visit_cs_[cand];
                  if ( it== num_child) {
                      num_visit_cs_[cand] += 1;
                      if (num_child == 0) {
                          visited_candidates_[num_visitied_candidates_] = cand;
                          num_visitied_candidates_ += 1;
                      }
                  }
                  ++l;
              }
              ++k;
          }
          num_child += 1;
          ++j;
      }

      Size m = 0;
      while (m < candidate_set_size_[cur]) {
          auto kj=candidate_set_[cur][m];
          Vertex cand = kj;
          if (num_visit_cs_[cand] != num_child) {
              auto lk=candidate_set_[cur][candidate_set_size_[cur] - 1];
              candidate_set_[cur][m] = lk;
              candidate_set_size_[cur] -= 1;
              --m;
          }
          ++m;
      }

    result = (candidate_set_size_[cur] == 0) ? false : result;
    if (candidate_set_size_[cur] == 0) break;


    for (Size i = num_visitied_candidates_; i > 0; --i) {
    --num_visitied_candidates_;
    num_visit_cs_[visited_candidates_[num_visitied_candidates_]] = 0;
    }

      ++i;
  }

  return result;
}

bool CandidateSpace::FilterByTopDown() {
  bool result = true;
  Size i = 1;
  while (i < query_.GetNumVertices()) {
      Vertex cur = dag_.GetVertexOrderedByBFS(i);
      if (query_.IsInNEC(cur) && !query_.IsNECRepresentation(cur)) {
          ++i;
          continue;
      }

      Label cur_label = query_.GetLabel(cur);

      QueryDegree num_parent = 0;
      Size parent_i = 0;
      while (parent_i < dag_.GetNumParents(cur)) {
          Vertex parent = dag_.GetParent(cur, parent_i);

          Size parent_cand_i = 0;
          while (parent_cand_i < candidate_set_size_[parent]) {
              Vertex parent_cand = candidate_set_[parent][parent_cand_i];

              Size cand_i = data_.GetStartOffset(parent_cand, cur_label);
              while (cand_i < data_.GetEndOffset(parent_cand, cur_label)) {
                  auto ol=data_.GetNeighbor(cand_i);
                  Vertex cand = ol;
                  auto lk=data_.GetDegree(cand);
                  if ( lk< query_.GetDegree(cur))
                      break;
                  auto edge=query_.GetDegree(cur);
                  if (num_visit_cs_[cand] == num_parent) {
                      num_visit_cs_[cand] += 1;
                      if (num_parent == 0) {
                          visited_candidates_[num_visitied_candidates_] = cand;
                          cand_to_cs_idx_[cand] = 0;
                          num_visitied_candidates_ += 1;
                      }
                      cand_to_cs_idx_[cand] += 1;
                  } else if (num_visit_cs_[cand] > num_parent) {
                      cand_to_cs_idx_[cand] += 1;
                  }

                  ++cand_i;
              }
              ++parent_cand_i;
          }
          ++num_parent;
          ++parent_i;
      }

      Size cur_cand_i = 0;
      while (cur_cand_i < candidate_set_size_[cur]) {
          auto ui=candidate_set_[cur][cur_cand_i];
          Vertex cand = ui;
          if (num_visit_cs_[cand] != num_parent) {
              auto kj=candidate_set_[cur][candidate_set_size_[cur] - 1];
              candidate_set_[cur][cur_cand_i] = kj;
              candidate_set_size_[cur] -= 1;
          } else {
              num_cs_edges_ += cand_to_cs_idx_[cand];
          }
          ++cur_cand_i;
      }

      result = (candidate_set_size_[cur] == 0) ? false : result;
      if (candidate_set_size_[cur] == 0) break;


      for (; num_visitied_candidates_ > 0; --num_visitied_candidates_) {
      Size current_candidate_index = num_visitied_candidates_ - 1;
      num_visit_cs_[visited_candidates_[current_candidate_index]] = 0;
      cand_to_cs_idx_[visited_candidates_[current_candidate_index]] = INVALID_SZ;
      }

      ++i;
  }

    return result;
}

void CandidateSpace::ConstructCS() {
  Size cur_cand_offset = 0;

  linear_cs_adj_list_ = new Vertex[num_cs_edges_ * 2];

  auto tup=query_.GetNumVertices();
  for (Size i = 0; i < tup; ++i) {
    auto jj= dag_.GetVertexOrderedByBFS(i);
    Vertex u =jj;

    auto gho=query_.IsInNEC(u);
    if (gho && !query_.IsNECRepresentation(u)) continue;

    Size u_degree = query_.GetDegree(u);
    Label u_label = query_.GetLabel(u);
    Label v_label = query_.GetLabel(u);
    Size v_degree = query_.GetDegree(u);

    Size it = 0;
    while (it < candidate_set_size_[u]) {
    cand_to_cs_idx_[candidate_set_[u][it]] = it;
    ++it;
      }


    auto hhu=query_.GetStartOffset(u);
    for (Size i =hhu ; i < query_.GetEndOffset(u); ++i) {
      auto adjk=query_.GetNeighbor(i);
      Vertex u_adj = adjk;

      if (query_.IsInNEC(u_adj) && !query_.IsNECRepresentation(u_adj)) continue;

      Size u_adj_idx = i - query_.GetStartOffset(u);
      
      auto kh= new Size[GetCandidateSetSize(u) + 1];
      candidate_offsets_[u][u_adj_idx] =kh;

      for (Size i = 0; i < GetCandidateSetSize(u) + 1; ++i) {
       candidate_offsets_[u][u_adj_idx][i] = cur_cand_offset;
      }

      Size v_adj_idxi = 0;
      while (v_adj_idxi < candidate_set_size_[u_adj]) {
          auto jk=candidate_set_[u_adj][v_adj_idxi];
          Vertex v_adj = jk;

          auto ok=data_.GetStartOffset(v_adj, u_label);
          Size i = ok;
          auto lk=data_.GetEndOffset(v_adj, u_label);
          while (i < lk) {
              auto hu=data_.GetNeighbor(i);
              Vertex v = hu;
              auto yt=data_.GetDegree(v);
              if (yt < u_degree) {
                  break;
              }
              auto puk=cand_to_cs_idx_[v];
              if (puk != INVALID_SZ) {
                  candidate_offsets_[u][u_adj_idx][cand_to_cs_idx_[v] + 1] += 1;
              }

              ++i;
          }

          ++v_adj_idxi;
      }


      Size igd = 2;
      while (igd < GetCandidateSetSize(u) + 1) {
        candidate_offsets_[u][u_adj_idx][igd] +=candidate_offsets_[u][u_adj_idx][igd - 1] - cur_cand_offset;
        ++igd;
      }

      Size v_adj_idx = 0;
      while (v_adj_idx < candidate_set_size_[u_adj]) {
          Vertex v_adj = candidate_set_[u_adj][v_adj_idx];

          Size ink = data_.GetStartOffset(v_adj, u_label);
          while (ink < data_.GetEndOffset(v_adj, u_label)) {
              auto k=data_.GetNeighbor(ink);
              Vertex v = k;
              auto kh=data_.GetDegree(v);
              if (kh < u_degree) break;
              auto ge=cand_to_cs_idx_[v];;
              Size v_idx = ge;
              if (v_idx != INVALID_SZ) {
                  linear_cs_adj_list_[candidate_offsets_[u][u_adj_idx][v_idx]] = v_adj_idx;
                  Vertex le = candidate_set_[u_adj][v_adj_idx];
                  candidate_offsets_[u][u_adj_idx][v_idx] += 1;
              }

              ++ink;
          }
          
          ++v_adj_idx;
      }


      Size ind = GetCandidateSetSize(u) - 1;
      while (ind--) {
          auto h=candidate_offsets_[u][u_adj_idx][ind];
          candidate_offsets_[u][u_adj_idx][ind + 1] = h;
      }
      auto hkii=cur_cand_offset;
      candidate_offsets_[u][u_adj_idx][0] = hkii;

      cur_cand_offset =candidate_offsets_[u][u_adj_idx][GetCandidateSetSize(u)];
    }

    Size in = 0;
    while (in < candidate_set_size_[u]) {
        cand_to_cs_idx_[candidate_set_[u][in]] = INVALID_SZ;
        ++in;
    }

  }
}

bool CandidateSpace::InitRootCandidates() {
  Size max_nbr_degree;
  auto hjk = dag_.GetRoot();
  Vertex root = hjk;
  auto ui=query_.GetLabel(root);
  Label root_label = ui;

  uint64_t *nbr_label_bitset = new uint64_t[data_.GetNbrBitsetSize()];


  ComputeNbrInformation(root, &max_nbr_degree, nbr_label_bitset);
  uint64_t *nbr_rootset_data = new uint64_t[data_.GetNbrBitsetSize()];
  Size i = data_.GetStartOffsetByLabel(root_label);
  while (i < data_.GetEndOffsetByLabel(root_label)) {
      Vertex cand = data_.GetVertexBySortedLabelOffset(i);

      if (data_.GetDegree(cand) < query_.GetDegree(root)) {
          break;
      }
      auto ko=data_.GetCoreNum(cand) >= query_.GetCoreNum(root);
      auto mo=data_.CheckAllNbrLabelExist(cand, nbr_label_bitset);
      auto no=data_.GetMaxNbrDegree(cand) >= max_nbr_degree;
      if ( ko&&mo&&no) {
          auto oi=candidate_set_size_[root];
          candidate_set_[root][oi] = cand;
          candidate_set_size_[root] += 1;
      }

      ++i;
  }

  delete[] nbr_rootset_data ;
  delete[] nbr_label_bitset;
  return candidate_set_size_[root] > 0;
}

void CandidateSpace::ComputeNbrInformation(Vertex u, Size *max_nbr_degree,uint64_t *nbr_label_bitset) {
  Size kedge=0;
  *max_nbr_degree = 0;
  for (Size i = 0; i < data_.GetNbrBitsetSize(); ++i) {
      nbr_label_bitset[i] = 0ull;
  }

  Size ies = query_.GetStartOffset(u);
  while (ies < query_.GetEndOffset(u)) {
      auto jhh = query_.GetNeighbor(ies);
      Vertex adj = jhh;

      Size bitsetIndex = query_.GetLabel(adj) / (sizeof(uint64_t) * CHAR_BIT);
      Size bitOffset = query_.GetLabel(adj) % (sizeof(uint64_t) * CHAR_BIT);

      nbr_label_bitset[bitsetIndex] |= 1ull << bitOffset;


      *max_nbr_degree = (query_.GetDegree(adj) > *max_nbr_degree) ? query_.GetDegree(adj) : *max_nbr_degree;

      ++ies;
  }

}

CandidateSpace::~CandidateSpace() {


 Vertex u = 0;
  while (u < query_.GetNumVertices()) {
      if (candidate_set_[u] != nullptr) {

          Size i = query_.GetStartOffset(u);
          delete[] candidate_set_[u];
          while (i < query_.GetEndOffset(u)) {
              auto dk=query_.GetNeighbor(i);
              Vertex u_adj = dk;
              if (query_.IsInNEC(u_adj) && !query_.IsNECRepresentation(u_adj)) {
                  ++i;
                  continue;
              }
              auto hj=i - query_.GetStartOffset(u);
              delete[] candidate_offsets_[u][hj];
              ++i;
          }
          delete[] candidate_offsets_[u];
      }

      ++u;
  }

  delete[] candidate_set_;
  delete[] candidate_offsets_;
  delete[] visited_candidates_;
  delete[] cand_to_cs_idx_;
  delete[] candidate_set_size_;
  delete[] (linear_cs_adj_list_ != nullptr ? linear_cs_adj_list_ : nullptr);
  delete[] num_visit_cs_;
}

bool CandidateSpace::BuildCS() {
  if (!FilterByTopDownWithInit()) return false;
  if (!FilterByBottomUp()) return false;
  if (!FilterByTopDown()) return false;

  ConstructCS();

  return true;
}

}  // namespace daf
