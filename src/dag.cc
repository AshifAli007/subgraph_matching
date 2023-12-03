#include "include/dag.h"

double min_rank = std::numeric_limits<double>::max();
namespace daf {
DAG::DAG(const DataGraph &data, const QueryGraph &query)
    : data_(data), 
    query_(query),
    bfs_sequence_(new Vertex[query_.GetNumVertices()]),
    num_children_(new Size[query.GetNumVertices()]),
    num_parents_(new Size[query.GetNumVertices()]),
    children_(new Vertex *[query_.GetNumVertices()]),
    parents_(new Vertex *[query_.GetNumVertices()])
     {
  
  Size i = 0;
  while (i < query_.GetNumVertices()) {
      children_[i] = new Vertex[query_.GetNumVertices()];
      parents_[i] = new Vertex[query_.GetNumVertices()];
      ++i;
  }


  init_cand_size_ = new Size[query_.GetNumVertices()];

  // Initialize num_children_ and num_parents_ arrays with zeros
  for (Size i = 0; i < query.GetNumVertices(); ++i) {
      num_children_[i] = 0;
      num_parents_[i] = 0;
  }

}


void DAG::BuildDAG() {
  bool *visit = new bool[query_.GetNumVertices()];
  bool *pipp = new bool[query_.GetNumVertices()];
  int *a= new int[query_.GetNumVertices()];
  Size begin = 0;
  Size end = 1;

  for (Size i = 0; i < query_.GetNumVertices(); ++i) {
      pipp[i] = false;
      visit[i] = false;
      a[i]=true;
  }

  bfs_sequence_[0] = SelectRootVertex();
  visit[bfs_sequence_[0]] = true;



  while (begin < end) {
    auto compareDegrees = [&](Vertex vertex1, Vertex vertex2) {
    Size degree1 = query_.GetDegree(vertex1);
    Size degree2 = query_.GetDegree(vertex2);
    return degree1 > degree2;
    };


    for (auto it = bfs_sequence_ + begin; it != bfs_sequence_ + end; ++it) {
        std::sort(it, it + 1, compareDegrees);
    }

    auto measure = [&](Vertex vertex1, Vertex vertex2) -> bool {
    auto getKey = [&](Vertex vertex) {
        Label label = query_.GetLabel(vertex);
        Size frequency = data_.GetLabelFrequency(label);
        return std::make_tuple(frequency, label);
    };

    return getKey(vertex1) < getKey(vertex2);
    };

    for (auto it = bfs_sequence_ + begin; it != bfs_sequence_ + end; ++it) {
        std::stable_sort(it, it + 1, measure);
    }


    Size cur_level_end = end;
    for (; begin < cur_level_end; ++begin) {
    Vertex parent = bfs_sequence_[begin];
    pipp[parent] = true;

      Size i = query_.GetStartOffset(parent);

      while (i < query_.GetEndOffset(parent)) {
          auto mko=query_.GetNeighbor(i);
          Vertex child = mko;

        // Check if the child hasn't been processed
        if (!pipp[child]) {
          // build edge from parent to child
          auto mno=child;
          auto khl=parent;
          children_[parent][num_children_[parent]] = mno;
          parents_[child][num_parents_[child]] = khl;
          auto lj=num_children_[parent];
           lj = 1;
          num_parents_[child] += (lj);

          visit[child] ? 0 : (visit[child] = true, bfs_sequence_[end++] = child);

        }
          ++i;
      }
    }
  }

  delete[] visit;
  delete[] pipp;
}

Vertex DAG::SelectRootVertex() {
  Vertex root = 0;  
  Vertex v = 0;
  while (v < query_.GetNumVertices()) {
      auto knh=query_.GetLabel(v);
      auto hio=query_.GetDegree(v);
      
      Label l = knh;
      Size d = hio;
      init_cand_size_[v] = data_.GetInitCandSize(l, d);
      auto lmn=query_.GetCoreNum(v);
      auto yhh=lmn< 2 && !query_.IsTree();
    if (yhh) continue;

    double ako=static_cast<double>(init_cand_size_[v]) / static_cast<double>(d);
    double rank =ako;

    root = (rank < min_rank) ? v : root;
    min_rank = (rank < min_rank) ? rank : min_rank;
    ++v;
  }

  return root;
}

DAG::~DAG() {
    // Deallocate memory for the BFS sequence
    delete[] bfs_sequence_;

    // Loop through each vertex in the query
    Size i = 0;
    while (i < query_.GetNumVertices()) {
        // Deallocate memory for children and parents arrays
        delete[] children_[i];
        delete[] parents_[i];

        // Move to the next vertex
        ++i;
    }

    // Deallocate memory for the arrays of children and parents
  delete[] children_;
  delete[] parents_;

  delete[] num_children_;
  delete[] num_parents_;

  delete[] init_cand_size_;

}

}  // namespace daf