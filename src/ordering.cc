#include "include/ordering.h"

namespace daf {

Ordering::Ordering(Size query_vertex_count)
    : num_query_vertices_(query_vertex_count),
      extendable_queue_(new Vertex[query_vertex_count]),
      extendable_queue_size_(0),
      weights_(new Size[query_vertex_count]) {
  std::fill(weights_, weights_ + query_vertex_count,
            std::numeric_limits<Size>::max());
}

Ordering::~Ordering() {
    // Use smart pointers to manage dynamic arrays
    std::unique_ptr<Size[]> extendable_queue_ptr(extendable_queue_);
    std::unique_ptr<Size[]> weights_ptr(weights_);
    
    // Arrays are automatically deleted when smart pointers go out of scope
}

void Ordering::Insert(Vertex vertex, Size weight) {
  extendable_queue_[extendable_queue_size_] = vertex;
  extendable_queue_size_++;
  weights_[vertex] = weight;
}

void Ordering::UpdateWeight(Vertex vertex, Size weight) { weights_[vertex] = weight; }

void Ordering::Remove(Vertex vertex) {
  Size idx = 0;
  while (idx < extendable_queue_size_) {
   
    auto it = std::find(extendable_queue_, extendable_queue_ + extendable_queue_size_, vertex);
    if (it != extendable_queue_ + extendable_queue_size_) {
      Size idx = std::distance(extendable_queue_, it);
      weights_[vertex] = std::numeric_limits<Size>::max();
      Size temp = extendable_queue_[idx];
      extendable_queue_[idx] = extendable_queue_[extendable_queue_size_ - 1];
      extendable_queue_[extendable_queue_size_ - 1] = temp;
      
      extendable_queue_size_--;
}
    idx++;
  }
}

bool Ordering::Exists(Vertex vertex) {
  return weights_[vertex] != std::numeric_limits<Size>::max();
}

Vertex Ordering::PopMinWeight() {
  Size min_weight_idx = 0;
  Size i = 1;

  while (i < extendable_queue_size_) {
    if (weights_[extendable_queue_[i]] <
        weights_[extendable_queue_[min_weight_idx]]) {
      min_weight_idx = i;
    }
    i++;
  }
  Vertex min_weight_vertex = extendable_queue_[min_weight_idx];
  weights_[min_weight_vertex] = std::numeric_limits<Size>::max();

Vertex temp = extendable_queue_[min_weight_idx];
extendable_queue_[min_weight_idx] = extendable_queue_[extendable_queue_size_ - 1];
extendable_queue_[extendable_queue_size_ - 1] = temp;

  return min_weight_vertex;
}
}  // namespace daf
