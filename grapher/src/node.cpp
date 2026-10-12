#include <grapher.hpp>
#include <cstring>

namespace gp {

IndicatorDescription NodeEvaluator::description() const {
  return "<no description>";
}
Indicator NodeEvaluator::evaluate_from_osm(const osmium::Node &osm_node) {
  return 0;
}
Indicator NodeEvaluator::evaluate_from_graph(const Node &node) {
  return 0;
}



NodeContext::NodeContext(NodeEvaluatorList &&evaluators__):
  dim_(evaluators__.size()),
  evaluators_(std::move(evaluators__)),
  max_indicators_(evaluators__.size(), 0.0f),
  min_indicators_(evaluators__.size(), INFINITY),
  indicator_name_offs_(evaluators__.size()),
  indicator_desc_offs_(evaluators__.size())
{
  indicator_name_size_ = 0;
  for(int i = 0; i < dim(); i++) {
    indicator_name_offs_[i] = indicator_name_size_;
    indicator_name_size_ += std::strlen(evaluator(i).name()) + 1;
  }

  indicator_desc_size_ = 0;
  for(int i = 0; i < dim(); i++) {
    indicator_desc_offs_[i] = indicator_desc_size_;
    indicator_desc_size_ += std::strlen(evaluator(i).description()) + 1;
  }
}

void NodeContext::write(FILE *out, const bgHeader_t *header) const {
  bgNodeContext_t bg_nc;
  bgNodeContextCreate(&bg_nc, header);

  bg_nc.dim = dim();

  bg_nc.indicator_name_size = indicator_name_size_;
  for(int i = 0; i < dim(); i++) {
    std::strcpy(
      bg_nc.indicator_names + indicator_name_offs_[i],
      evaluator(i).name()
    );
    bg_nc.indicator_name_offs[i] = indicator_name_offs_[i];
  }

  bg_nc.indicator_desc_size = indicator_desc_size_;
  for(int i = 0; i < dim(); i++) {
    std::strcpy(
      bg_nc.indicator_descs + indicator_desc_offs_[i],
      evaluator(i).description()
    );
    bg_nc.indicator_desc_offs[i] = indicator_desc_offs_[i];
  }

  bgNodeContextWrite(&bg_nc, out, header);
  bgNodeContextDestroy(&bg_nc);
}



Node::Node(NodeContext &nc, ID id__, double lon__, double lat__):
  id_(id__),
  indicators_(nc.dim(), 0.0f),
  lon_(lon__),
  lat_(lat__)
{}

void Node::add_way(Way &&way) {
  ways_.emplace_back(std::move(way));
}

void Node::write(FILE *out, const bgHeader_t *header, const NodeContext &nc) const {
  bgNode_t bg_node;
  bgNodeCreate(&bg_node, header);

  bg_node.idx = idx();
  bg_node.way_begin = ways().front().idx();
  bg_node.way_end   = ways().back().idx();
  bg_node.lon = lon();
  bg_node.lat = lat();
  for(int i = 0; i < nc.dim(); i++) {
    bg_node.indicators[i] = indicator(i);
  }

  bgNodeWrite(&bg_node, out, header);
  bgNodeDestroy(&bg_node);
}



double Node::distance(const Node &x, const Node &y) {
  double dlon = x.lon() - y.lon();
  double dlat = x.lat() - y.lat();
  return std::hypot(dlon, dlat);
}

};
