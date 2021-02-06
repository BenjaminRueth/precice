#pragma once

#include <memory>
#include <vector>
#include "logging/Logger.hpp"
#include "mesh/BoundingBox.hpp"
#include "mesh/Edge.hpp"
#include "mesh/SharedPointer.hpp"
#include "mesh/Triangle.hpp"
#include "mesh/Vertex.hpp"

namespace precice {
namespace query {

/// Struct to hold index and distance information of the closest primitive
template <class Tag>
struct MatchType {
  double distance;
  int    index;

  MatchType() = default;
  MatchType(double d, int i)
      : distance(d), index(i){};
  constexpr bool operator<(MatchType const &other) const
  {
    return distance < other.distance;
  };
};

/// Match tags for each primitive type
using VertexMatch   = MatchType<struct VertexMatchTag>;
using EdgeMatch     = MatchType<struct EdgeMatchTag>;
using TriangleMatch = MatchType<struct TriangleTag>;

namespace impl {
class RTreeWrapper;
struct MeshIndices;
} // namespace impl

/// Class to query the index trees of the mesh
class Index {

public:
  Index(const mesh::PtrMesh &mesh);
  ~Index();

  /// Get n number of closest vertices to the given vertex
  std::vector<VertexMatch> getClosestVertex(const mesh::Vertex &sourceVertex, int n = 1);

  /// Get n number of closest edges to the given vertex
  std::vector<EdgeMatch> getClosestEdge(const mesh::Vertex &sourcesVertex, int n = 1);

  /// Get n number of closest triangles to the given vertex
  std::vector<TriangleMatch> getClosestTriangle(const mesh::Vertex &sourceVertex, int n = 1);

  /// Return all the vertices inside the box formed by vertex and radius
  std::vector<size_t> getVerticesInsideBox(const mesh::Vertex &centerVertex, double radius);

  /// Return all the vertices inside a bounding box
  std::vector<size_t> getVerticesInsideBox(const mesh::BoundingBox &bb);

  /// Clear all the cache
  static void clearCache();

  /// Clear the cache of given mesh
  static void clearCache(int meshID);

  /// Clear the cache of given mesh
  static void clearCache(mesh::Mesh &mesh);

private:
  std::unique_ptr<impl::RTreeWrapper> _rtreeWrapper;
  std::unique_ptr<impl::MeshIndices>  _cache;
  const mesh::PtrMesh                 _mesh;
  static precice::logging::Logger     _log;
};

} // namespace query
} // namespace precice