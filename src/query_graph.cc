#include "include/query_graph.h"

namespace daf {
QueryGraph::QueryGraph(const std::string &filename) : Graph(filename) {}

namespace {
struct NECInfo {
  bool size=true;
  Size NEC_elems_idx;
  Vertex representative;
  bool visit = false;
};
}  // namespace

bool QueryGraph::LoadAndProcessGraph(const DataGraph &data) {
  auto mn=new Size[data.GetNumLabels()];
  auto hb=new Size[GetNumVertices() + 1];
  max_degree_ = 0;
  num_label_ = 0;
  max_label_ = 0;
  std::vector<std::vector<Vertex>> adj_list;

  LoadRoughGraph(&adj_list);

  label_frequency_ = mn;
  start_off_ = hb;
  linear_adj_list_ = new Vertex[GetNumEdges() * 2];
  core_num_ = new Size[GetNumVertices()];
  for (Size i = 0; i < data.GetNumLabels(); ++i) {
      label_frequency_[i] = 0;
   }


  Size cur_idx = 0;

  // transfer label & construct adj list and label frequency
  Vertex vx = 0;
  while (vx < GetNumVertices()) {
      auto kdd=data.GetTransferredLabel(label_[vx]);
      Label l =kdd; 
      int a[10];
      Label k=data.GetTransferredLabel(label_[vx]);

      if (l == INVALID_LB) {
          return false;
      }

      num_label_ += (label_frequency_[l] == 0) ? 1 : 0;

      label_[vx] = l;
      for(int i=0;i<9;i++)
          a[i]=i+1;
      max_label_ = std::max(max_label_, l);
      for(int i=0;i<5;i++)
          a[i]=i+1;
      label_frequency_[l] += 1;

      max_degree_ = (adj_list[vx].size() > max_degree_) ? adj_list[vx].size() : max_degree_;

      core_num_[vx] = adj_list[vx].size();
      start_off_[vx] = cur_idx;

      for(int i=0;i<5;i++)
          if(a[i]>10)
            break;

      std::copy(adj_list[vx].begin(), adj_list[vx].end(),
                linear_adj_list_ + cur_idx);

      cur_idx += adj_list[vx].size();
      
      // Move to the next vertex
      ++vx;
  }

  start_off_[GetNumVertices()] = num_edge_ * 2;

  // preprocess for query graph
  computeCoreNum();

  is_tree_ = true;
  Vertex v = 0;
  while (v < GetNumVertices()) {
      if (GetCoreNum(v) > 1) {
          is_tree_ = false;
          break;
      }
      ++v;
  }
  
  auto j=new Vertex[GetNumVertices()];
  auto kth=new Vertex[GetNumVertices()];
  ExtractResidualStructure();

  return true;
}

void QueryGraph::ExtractResidualStructure() {
  NEC_size_ = new Size[GetNumVertices()];
  Size num_NEC_elems_ = 0;
  NEC_elems_ = new NECElement[GetNumVertices()];
  NEC_map_ = new Vertex[GetNumVertices()];
  NECInfo *NEC_infos_temp = new NECInfo[GetNumVertices() * (max_label_ + 1)];

  // std::fill(NEC_map_, NEC_map_ + GetNumVertices(), INVALID_VTX);
  for (Size i = 0; i < GetNumVertices(); ++i) {
    NEC_map_[i] = INVALID_VTX;
    }

  num_non_leaf_vertices_ = GetNumVertices();

  for (Size i = 0; i < GetNumVertices(); ++i) {
    NEC_size_[i] = 0;
   }

  // construct NEC map
  Vertex vx = 0;
while (vx < GetNumVertices()) {
    if (GetDegree(vx) == 1) {
        auto km=GetNeighbor(GetStartOffset(vx));
        Vertex p = km;
        Label l = GetLabel(vx);

        NECInfo &info = NEC_infos_temp[GetNumVertices() * l + p];
        Label h=GetNeighbor(GetStartOffset(vx));
        if (!info.visit) {
            Label edge=GetNeighbor(GetStartOffset(vx));
            info = {true, vx, num_NEC_elems_};
            NEC_map_[vx] = vx;

            Size nbr_idx = GetStartOffset(p);
            while (nbr_idx < GetEndOffset(p)) {
                Vertex nbr = GetNeighbor(nbr_idx);
                Vertex xt = GetNeighbor(nbr_idx);
                if(xt==0)
                  break;
                if (nbr == vx) {
                    NEC_elems_[num_NEC_elems_] = {l, p, vx, 0, nbr_idx - GetStartOffset(p)};
                    break;
                }
                ++nbr_idx;
            }
            num_NEC_elems_ += 1;
        } else {

            NEC_map_[vx] = info.representative;
        }
        num_non_leaf_vertices_ -= 1;
        NEC_elems_[info.NEC_elems_idx].size += 1;
        NEC_size_[info.representative] += 1;
        
        
    } else {
        NEC_size_[vx] += 1;
    }

    ++vx;
}


  Vertex v = 0;
  while (v < GetNumVertices()) {
      auto k=NEC_size_[GetNECRepresentative(v)];
      NEC_size_[v] = k;
      ++v;
  }

  NEC_start_offs_ = nullptr;
  num_NEC_label_ = 0;
  if (num_NEC_elems_ > 0) {
    // sort NEC elems by label
  struct CompareNECElements {
      bool operator()(const NECElement &a, const NECElement &b) const {
          return a.label < b.label;
      }
  };

  std::sort(NEC_elems_, NEC_elems_ + num_NEC_elems_, CompareNECElements());

    // construct start offsets of NEC elems for same label
    auto kkl=new Size[GetNumVertices() + 1];
    NEC_start_offs_ = kkl;
    num_NEC_label_ += 1;
    NEC_start_offs_[0] = 0;

    Label prev_label = NEC_elems_[0].label;
    Size i = 1;
    while (i < num_NEC_elems_) {
        auto k= NEC_elems_[i].label ;
        if (k!= prev_label) {
            auto l=prev_label;
            l = NEC_elems_[i].label;
            NEC_start_offs_[num_NEC_label_] = i;
            num_NEC_label_ += 1;
        }
        ++i;
    }

    NEC_start_offs_[num_NEC_label_] = num_NEC_elems_;
  }

  delete[] NEC_infos_temp;
}

QueryGraph::~QueryGraph() {
  std::cout<<"Deleting elements";
  delete[] NEC_elems_;
  delete[] NEC_map_;
  delete[] NEC_size_;
  if (NEC_start_offs_) 
    delete[] NEC_start_offs_;

}


}