#ifndef GRAPHER_GRAPH_HPP
#define GRAPHER_GRAPH_HPP

#include <grapher/types.hpp>
#include <grapher/node.hpp>
#include <grapher/way.hpp>

namespace gp {

class Graph {
public:
  Graph(const char *file__, NodeContext &nc__, WayContext &wc__);

  inline const NodeContext &nc() const {
    return nc_;
  }
  inline NodeContext &nc() {
    return nc_;
  }
  inline const WayContext &wc() const {
    return wc_;
  }
  inline WayContext &wc() {
    return wc_;
  }
  inline const std::unordered_map<ID, Node> &nodes() const {
    return nodes_;
  }
  inline bgIdx_t node_count() const {
    return nodes_.size();
  }
  inline bgIdx_t way_count() const {
    return way_count_;
  }

  bool has(ID id) const;
  const Node &get(ID id) const;
  const Node &nearest(double lon, double lat) const;

  void write(std::filesystem::path path) const;

private:
  inline std::unordered_map<ID, Node> &nodes() {
    return nodes_;
  }

  Node &get(ID id);
  void put(ID id, Node &&node);

  void construct();
  void evaluate_from_osm();
  void evaluate_from_graph();
  void validate_indicators();
  void find_max_indicators();
  void normalize();

  void resolve_idx();
  void count_ways();

  std::unordered_map<ID, Node> nodes_;
  const char *file_;
  NodeContext &nc_;
  WayContext &wc_;
  bgIdx_t way_count_;
};

};

#endif
