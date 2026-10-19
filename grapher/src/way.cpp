#include <grapher.hpp>
#include <cstring>

namespace gp {

IndicatorDescription WayEvaluator::description() const {
  return "<no description>";
}
Indicator WayEvaluator::evaluate_from_osm(const osmium::Way &osm_way) {
  return 0;
}
Indicator WayEvaluator::evaluate_from_graph(const Way &way, const Node &from, const Node &to) {
  return 0;
}



WayContext::WayContext(WayEvaluatorList &&evaluators__):
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

void WayContext::write(FILE *out, const bgHeader_t *header) const {
  bgWayContext_t bg_wc;
  bgError_t err = bgWayContextCreate(&bg_wc, header);
  if(err != BG_ERROR_OK) {
    bgWayContextDestroy(&bg_wc);
    throw std::runtime_error("[gp::WayContext::write]");
  }

  bg_wc.dim = dim();

  bg_wc.indicator_name_size = indicator_name_size_;
  for(int i = 0; i < dim(); i++) {
    std::strcpy(
      bg_wc.indicator_names + indicator_name_offs_[i],
      evaluator(i).name()
    );
    bg_wc.indicator_name_offs[i] = indicator_name_offs_[i];
  }

  bg_wc.indicator_desc_size = indicator_desc_size_;
  for(int i = 0; i < dim(); i++) {
    std::strcpy(
      bg_wc.indicator_descs + indicator_desc_offs_[i],
      evaluator(i).description()
    );
    bg_wc.indicator_desc_offs[i] = indicator_desc_offs_[i];
  }

  bgWayContextWrite(&bg_wc, out, header);
  bgWayContextDestroy(&bg_wc);
}



Way::Way(WayContext &wc, ID id__, ID to__, double length__):
  id_(id__),
  to_(to__),
  indicators_(wc.dim(), 0.0f),
  length_(length__)
{}

void Way::write(FILE *out, const bgHeader_t *header, const WayContext &wc, const Graph &graph) const {
  bgWay_t bg_way;
  bgError_t err = bgWayCreate(&bg_way, header);
  if(err != BG_ERROR_OK) {
    bgWayDestroy(&bg_way);
    throw std::runtime_error("[gp::Way::write]");
  }

  bg_way.idx = idx();
  bg_way.reverse_idx = reverse_idx();
  bg_way.to = graph.get(to()).idx();
  bg_way.length = length();
  for(int i = 0; i < wc.dim(); i++) {
    bg_way.indicators[i] = indicator(i);
  }

  bgWayWrite(&bg_way, out, header);
  bgWayDestroy(&bg_way);
}

};
