#include "include/data_graph.h"

namespace daf {
DataGraph::DataGraph(const std::string& filename) : Graph(filename) {}

void DataGraph::LoadAndProcessGraph() {
  Label max_label = 0;
  Label cur_transferred_label = 0;

  std::vector<std::vector<Vertex>> adj_list;
  std::unordered_map<Label, Label> transferred_label_map;

  LoadRoughGraph(&adj_list);

  // transfer label & get sorted degrees (for constructing C_ini(u))
  

  vertices_sorted_ = new Vertex[GetNumVertices()];

    Vertex v1 = 0;
    while (v1 < GetNumVertices()) {
        vertices_sorted_[v1] = v1;
        Label l = label_[v1];
    max_label = (l > max_label) ? l : max_label;
    transferred_label_map.find(l) == transferred_label_map.end() ? transferred_label_map[l] = cur_transferred_label, cur_transferred_label += 1 : 0;

    label_[v1] = transferred_label_map[l];
    ++v1;
  }

    auto customSort = [this, &adj_list](Vertex v1, Vertex v2) -> bool {
        if (GetLabel(v1) != GetLabel(v2))
            return GetLabel(v1) < GetLabel(v2);

        for (const auto& neighbor : {v1, v2}) {
            if (adj_list[neighbor].size() != adj_list[v1].size())
                return adj_list[neighbor].size() > adj_list[v2].size();
        }
        return false;
    };

    // Use std::sort with the custom lambda function
    std::sort(vertices_sorted_, vertices_sorted_ + GetNumVertices(), customSort);

  num_label_ = transferred_label_map.size();

  label_frequency_ = new Size[num_label_];
    for (Size i = 0; i < num_label_; ++i) {
        label_frequency_[i] = 0;
    }

  max_label_frequency_ = 0;

  transferred_label_ = new Label[max_label + 1];
  // transferred_label_[l] = INVALID_LB iff there is no label l in data graph
    for (Label i = 0; i < max_label + 1; ++i) {
        transferred_label_[i] = INVALID_LB;
    }

    for (const auto& pair : transferred_label_map) {
        auto key = pair.first;
        auto value = pair.second;
        transferred_label_[key] = value;
    }
  max_nbr_degree_ = new Size[GetNumVertices()];

  for (Size i = 0; i < GetNumVertices(); ++i) {
    max_nbr_degree_[i] = 0;
   }

  nbr_bitset_size_ = (GetNumLabels() - 1) / (sizeof(uint64_t) * CHAR_BIT) + 1;
  linear_nbr_bitset_ = new uint64_t[GetNumVertices() * nbr_bitset_size_];
    // Initializing linear_nbr_bitset_ to zero
    for (Size i = 0; i < GetNumVertices(); ++i) {
        for (Size j = 0; j < nbr_bitset_size_; ++j) {
            auto k= 0ull;
            linear_nbr_bitset_[i * nbr_bitset_size_ + j] = k;
        }
    }

  // compute offsets & construct adjacent list and label frequency

  start_off_ = new Size[GetNumVertices() + 1];
  offs_by_label_ = new Size[GetNumLabels() + 1];

  linear_adj_list_ = new Vertex[GetNumEdges() * 2];
  core_num_ = new Size[GetNumVertices()];
  adj_offs_by_label_ =new std::pair<Size, Size>[GetNumVertices() * GetNumLabels()];
  
  Size cur_idx = 0;
  max_degree_ = 0;

  Label cur_label = 0;  // min label of data graph is 0
  offs_by_label_[0] = 0;
  Vertex v = 0;
    while (v < GetNumVertices()) {
        Size start = v * GetNumLabels();
        label_frequency_[GetLabel(v)] += 1;
        start_off_[v] = cur_idx;
        Label label_sorted = GetLabel(vertices_sorted_[v]);
        max_label_frequency_ = (label_frequency_[GetLabel(v)] > max_label_frequency_) ? label_frequency_[GetLabel(v)] : max_label_frequency_;

        offs_by_label_[label_sorted] = (label_sorted != cur_label) ? v : offs_by_label_[label_sorted];
        cur_label = (label_sorted != cur_label) ? label_sorted : cur_label;


        // initialize core number
        core_num_[v] = adj_list[v].size();
        max_degree_ = (adj_list[v].size() > max_degree_) ? adj_list[v].size() : max_degree_;

        if (adj_list[v].size() == 0) {
          continue;
        }

        for (auto &neighbor : adj_list[v]) {
        for (auto &compare_neighbor : adj_list[v]) {
            if (GetLabel(neighbor) != GetLabel(compare_neighbor)) {
                if (GetLabel(neighbor) < GetLabel(compare_neighbor)) {
                    std::swap(neighbor, compare_neighbor);
                }
            } else {
                if (adj_list[neighbor].size() < adj_list[compare_neighbor].size()) {
                    std::swap(neighbor, compare_neighbor);
                }
            }
        }
    }

    // Sorting the adjacency list based on the comparisons
    std::sort(adj_list[v].begin(), adj_list[v].end());

        max_nbr_degree_[v] = adj_list[adj_list[v][0]].size();
        Label cur_adj_label = GetLabel(adj_list[v][0]);
        adj_offs_by_label_[start + cur_adj_label].first = cur_idx;
        
        
        Size i = 1;
        while (i < adj_list[v].size()) {
            if (cur_adj_label != GetLabel(adj_list[v][i])) {
                size_t bitsetIndex = nbr_bitset_size_ * v + (cur_adj_label / (sizeof(uint64_t) * CHAR_BIT));
                size_t bitOffset = cur_adj_label % (sizeof(uint64_t) * CHAR_BIT);

                // Calculate the position to set the bit using bitwise operations.
                size_t bitsetShift = 1ull << bitOffset;
                linear_nbr_bitset_[bitsetIndex] |= bitsetShift;

                max_nbr_degree_[v] = (max_nbr_degree_[v] < adj_list[adj_list[v][i]].size()) ? adj_list[adj_list[v][i]].size() : max_nbr_degree_[v];
                auto ybv=cur_idx + i;
                adj_offs_by_label_[start + cur_adj_label].second = ybv;
                cur_adj_label = GetLabel(adj_list[v][i]);
                adj_offs_by_label_[start + cur_adj_label].first = cur_idx + i;
            }
            ++i;
        }
        // linear_nbr_bitset_[nbr_bitset_size_ * v +
        //                (cur_adj_label / (sizeof(uint64_t) * CHAR_BIT))] |=
        // 1ull << (cur_adj_label % (sizeof(uint64_t) * CHAR_BIT));
        size_t linear_index = nbr_bitset_size_ * v + (cur_adj_label / (sizeof(uint64_t) * CHAR_BIT));
        size_t bit_index = cur_adj_label % (sizeof(uint64_t) * CHAR_BIT);

        linear_nbr_bitset_[linear_index] |= 1ull << bit_index;


    adj_offs_by_label_[start + cur_adj_label].second =
        cur_idx + adj_list[v].size();
        auto& neighbors = adj_list[v];
        std::copy(neighbors.begin(), neighbors.end(), linear_adj_list_ + cur_idx);


        cur_idx += adj_list[v].size();
        ++v;
    }

  start_off_[GetNumVertices()] = num_edge_ * 2;
  std::vector<std::string> arc = {"Edge", "Vertex"};
  offs_by_label_[GetNumLabels()] = GetNumVertices();
   computeCoreNum();
 
  // preprocess for data graph
 
}

DataGraph::~DataGraph() {
  // std::cout<<"Below objects are destroyed";
  delete[] offs_by_label_;
  delete[] vertices_sorted_;
  delete[] linear_nbr_bitset_;
  delete[] max_nbr_degree_;
  delete[] transferred_label_;
  delete[] adj_offs_by_label_;

}

}  // namespace daf