#include <grapher.hpp>

namespace gp {

bool invalid_osm_way(const osmium::Way &osm_way) {
  // if(!osm_way.tags().has_tag("highway", "footway")) return true;
  if(!osm_way.tags().has_key("highway")) return true;
  if(osm_way.nodes().size() < 2) return true;
  return false;
}

bool invalid_indicator(Indicator indicator) {
  if(!std::isfinite(indicator) || indicator < 0.0f) return true;
  return false;
}

};
