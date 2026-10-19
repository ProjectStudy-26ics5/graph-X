#ifndef GRAPHER_WAY_HPP
#define GRAPHER_WAY_HPP

#include <grapher/types.hpp>

namespace gp {

class WayEvaluator {
public:
  virtual IndicatorName name() const = 0;
  virtual IndicatorDescription description() const;
  virtual Indicator evaluate_from_osm(const osmium::Way &osm_way);
  virtual Indicator evaluate_from_graph(const Way &way, const Node &from, const Node &to);
};



class WayContext {
public:
  WayContext(WayEvaluatorList &&evaluators__);

  inline int dim() const {
    return dim_;
  }
  inline const WayEvaluator &evaluator(int i) const {
    return *evaluators_[i];
  }
  inline WayEvaluator &evaluator(int i) {
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
  WayEvaluatorList evaluators_;
  IndicatorList max_indicators_;
  IndicatorList min_indicators_;

  bgSize_t indicator_name_size_;
  std::vector<bgOffset_t> indicator_name_offs_;
  bgSize_t indicator_desc_size_;
  std::vector<bgOffset_t> indicator_desc_offs_;
};



class Way {
public:
  Way(WayContext &wc, ID id__, ID to__, double length__);

  inline ID id() const {
    return id_;
  }
  inline IDX idx() const {
    return idx_;
  }
  inline IDX reverse_idx() const {
    return reverse_idx_;
  }
  inline ID to() const {
    return to_;
  }
  inline Indicator indicator(int i) const {
    return indicators_[i];
  }
  inline double length() const {
    return length_;
  }

  friend class Graph;

private:
  void write(FILE *out, const bgHeader_t *header, const WayContext &wc, const Graph &graph) const;

  const ID id_;
  IDX idx_;
  IDX reverse_idx_;
  ID to_;
  IndicatorList indicators_;
  double length_;
};

};

#endif
