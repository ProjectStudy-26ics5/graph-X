#ifndef GRAPHER_TYPES_HPP
#define GRAPHER_TYPES_HPP

#include <cmath>
#include <initializer_list>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <filesystem>

#include <osmium/osm.hpp>
#include <beltgraph.h>

namespace gp {

using IndicatorName = const char *;
using IndicatorDescription = const char *;
using Indicator = bgIndicator_t;
using IndicatorList = std::vector<Indicator>;

using Weight = bgWeight_t;
using WeightList = std::vector<Weight>;

using ID = osmium::object_id_type;
using IDX = bgIdx_t;

class Way;
class WayContext;
class WayEvaluator;
using WayEvaluatorList = const std::vector<WayEvaluator *>;

class Node;
class NodeContext;
class NodeEvaluator;
using NodeEvaluatorList = const std::vector<NodeEvaluator *>;

class Graph;

};

#endif
