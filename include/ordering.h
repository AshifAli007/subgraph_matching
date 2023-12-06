#ifndef ORDERING_H_
#define ORDERING_H_

#include "global/global.h"
#include "include/candidate_space.h"
#include "include/query_graph.h"

namespace daf {
class Ordering {
 public:
  explicit Ordering(Size query_vertex_count);
  ~Ordering();

  // Delete copy constructor and copy assignment operator
  Ordering(const Ordering &) = delete;
  Ordering &operator=(const Ordering &) = delete;

  // Methods for manipulating the ordering
  void Insert(Vertex vertex, Size weight_value);
  void UpdateWeight(Vertex vertex, Size new_weight);
  bool Exists(Vertex vertex);
  void Remove(Vertex vertex);
  Vertex PopMinWeight();

 private:
  // Member variables
  Size num_query_vertices_;
  Vertex *extendable_queue_;
  Size extendable_queue_size_;
  Size *weights_;
};
}  // namespace daf

#endif  // ORDERING_H_