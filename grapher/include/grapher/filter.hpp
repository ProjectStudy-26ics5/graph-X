#ifndef GRAPHER_FILTER_HPP
#define GRAPHER_FILTER_HPP

#include <grapher/types.hpp>

namespace gp {

bool invalid_osm_way(const osmium::Way &osm_way);
bool invalid_indicator(Indicator indicator);

};

#endif
