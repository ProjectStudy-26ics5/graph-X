#ifndef GRAPHER_NODE_HPP
#define GRAPHER_NODE_HPP

#include <grapher/types.hpp>
#include <grapher/way.hpp>

namespace gp {

class NodeEvaluator {
public:
  virtual IndicatorName name() const = 0;
  virtual IndicatorDescription description() const;
  virtual Indicator evaluate_from_osm(const osmium::Node &osm_node);
  virtual Indicator evaluate_from_graph(const Node &node);
};



class NodeContext {
public:
  NodeContext(NodeEvaluatorList &&evaluators__);

  inline int dim() const {
    return dim_;
  }
  inline const NodeEvaluator &evaluator(int i) const {
    return *evaluators_[i];
  }
  inline NodeEvaluator &evaluator(int i) {
    return *evaluators_[i];
  }
  inline Indicator max_indicator(int i) const {
    return max_indicators_[i];
  }
  inline Indicator min_indicator(int i) const {
    return min_indicators_[i];
  }

  friend class Graph;

private:
  void write(FILE *out, const bgHeader_t *header) const;

  const int dim_;
  NodeEvaluatorList evaluators_;
  IndicatorList max_indicators_;
  IndicatorList min_indicators_;
  
  bgSize_t indicator_name_size_;
  std::vector<bgOffset_t> indicator_name_offs_;
  bgSize_t indicator_desc_size_;
  std::vector<bgOffset_t> indicator_desc_offs_;
};



class Node {
public:
  Node(NodeContext &nc, ID id__, double lon__, double lat__);

  inline ID id() const {
    return id_;
  }
  inline IDX idx() const {
    return idx_;
  }
  inline const std::vector<Way> &ways() const {
    return ways_;
  }
  inline Indicator indicator(int i) const {
    return indicators_[i];
  }
  inline double lon() const {
    return lon_;
  }
  inline double lat() const {
    return lat_;
  }

  static double distance(const Node &x, const Node &y);

  friend class Graph;

private:
  inline std::vector<Way> &ways() {
    return ways_;
  }

  void add_way(Way &&way);
  void write(FILE *out, const bgHeader_t *header, const NodeContext &nc) const;

  const ID id_;
  IDX idx_;
  std::vector<Way> ways_;
  IndicatorList indicators_;
  const double lon_;
  const double lat_;
};

};

#endif
