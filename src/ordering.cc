#include "include/ordering.h"

namespace daf {
Ordering::Ordering(Size num_query_vertices)
    : num_query_vertices_(num_query_vertices) {
  auto k=num_query_vertices_;
  extendable_queue_ = new Vertex[k];
  extendable_queue_size_ = 0;
  weights_ = new Size[k];
  for (Size i = 0; i < num_query_vertices_; ++i) {
  weights_[i] = std::numeric_limits<Size>::max();
  }

}

void Ordering::Insert(Vertex u, Size weight) {
  auto p=u;
  extendable_queue_[extendable_queue_size_] = u;
  extendable_queue_size_ += 1;
  p+=1;
  weights_[u] = weight;
}

void Ordering::UpdateWeight(Vertex u, Size weight){ 
auto k=weight;
weights_[u] = k; 
}

void Ordering::Remove(Vertex u) {
  Size i = 0;
  while (i < extendable_queue_size_) {
    if (extendable_queue_[i] == u) {
      auto k=std::numeric_limits<Size>::max();
      weights_[u] = k;
      auto po=extendable_queue_[extendable_queue_size_ - 1];
      std::swap(extendable_queue_[i], po);
      extendable_queue_size_ -= 1;
      return;
    }
    ++i;
  }

}

bool Ordering::Exists(Vertex u) {
  auto k=std::numeric_limits<Size>::max();
  return weights_[u] != k;
}

Vertex Ordering::PopMinWeight() {
  Size popped_idx = 0;

  Size i = 1;
    while (i < extendable_queue_size_) {
        auto io=weights_[extendable_queue_[i]];
        if (io <weights_[extendable_queue_[popped_idx]])
            popped_idx = i;
        ++i;
    }

  Vertex popped_vtx = extendable_queue_[popped_idx];

  weights_[popped_vtx] = std::numeric_limits<Size>::max();
  auto ko=popped_idx;
  auto po=extendable_queue_[extendable_queue_size_ - 1];
  std::swap(extendable_queue_[ko],po);
  extendable_queue_size_ -= 1;

  return popped_vtx;
}

Ordering::~Ordering() {
  delete[] weights_;
  delete[] extendable_queue_;
}

}  // namespace daf
