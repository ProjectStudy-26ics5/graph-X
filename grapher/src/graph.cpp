#include <grapher.hpp>
#include <stdexcept>

#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/handler.hpp>

namespace gp {

Graph::Graph(const char *file__, NodeContext &nc__, WayContext &wc__):
  file_(file__),
  nc_(nc__),
  wc_(wc__)
{
  construct();
  evaluate_from_osm();
  evaluate_from_graph();
  validate_indicators();
  find_max_indicators();
  normalize();
  resolve_idx();
  count_ways();
}

bool Graph::has(ID id) const {
  return nodes_.find(id) != nodes_.end();
}

const Node &Graph::get(ID id) const {
  return nodes_.find(id)->second;
}

Node &Graph::get(ID id) {
  return nodes_.find(id)->second;
}

void Graph::put(ID id, Node &&node) {
  nodes_.emplace(id, std::move(node));
}

const Node &Graph::nearest(double lon, double lat) const {
  double min_dist = INFINITY;
  const Node *n = nullptr;

  for(const auto &[id, node] : nodes()) {
    double dlon = node.lon() - lon;
    double dlat = node.lat() - lat;
    double dist = std::hypot(dlon, dlat);

    if(dist < min_dist) {
      min_dist = dist;
      n = &node;
    }
  }

  return *n;
}



void Graph::construct() {
  using construct_NodeMemo = std::unordered_set<ID>;

  struct construct_Handler1 : public osmium::handler::Handler {
    Graph &graph;
    construct_NodeMemo &memo;
    construct_Handler1(Graph &graph, construct_NodeMemo &memo): graph(graph), memo(memo) {}
  
    void way(const osmium::Way &osm_way) {
      if(invalid_osm_way(osm_way)) return;
  
      const osmium::WayNodeList &osm_nodes = osm_way.nodes();
      for(int i = 0; i < osm_nodes.size(); i++) {
        memo.insert(osm_nodes[i].ref());
      }
    }
  };
  
  struct construct_Handler2 : public osmium::handler::Handler {
    Graph &graph;
    construct_NodeMemo &memo;
    construct_Handler2(Graph &graph, construct_NodeMemo &memo): graph(graph), memo(memo) {}
  
    void node(const osmium::Node &osm_node) {
      if(memo.find(osm_node.id()) == memo.end()) return;
  
      const auto &loc = osm_node.location();
      graph.put(
        osm_node.id(),
        Node(graph.nc(), osm_node.id(), loc.lon(), loc.lat())
      );
    }
  };
  
  struct construct_Handler3 : public osmium::handler::Handler {
    Graph &graph;
    construct_NodeMemo &memo;
    construct_Handler3(Graph &graph, construct_NodeMemo &memo): graph(graph), memo(memo) {}
  
    void way(const osmium::Way &osm_way) {
      if(invalid_osm_way(osm_way)) return;
  
      const osmium::WayNodeList &osm_nodes = osm_way.nodes();
      for(int i = 1; i < osm_nodes.size(); i++) {
        Node &from = graph.get(osm_nodes[i-1].ref());
        Node &to   = graph.get(osm_nodes[i].ref());
  
        const double length = Node::distance(from, to);
        from.add_way(Way(graph.wc(), osm_way.id(), to.id(), length));
        to.add_way(Way(graph.wc(), osm_way.id(), from.id(), length));
      }
    }
  };

  construct_NodeMemo memo;

  // 1. ウェイのみを読んで、ウェイに存在するノードをmemoに追加する
  osmium::io::Reader r1(file_);
  construct_Handler1 h1(*this, memo);
  osmium::apply(r1, h1);
  r1.close();

  // 2. memoにあるノードの情報を読んで、グラフに追加
  osmium::io::Reader r2(file_);
  construct_Handler2 h2(*this, memo);
  osmium::apply(r2, h2);
  r2.close();

  // 3. もう一度ウェイを読んで、ノードにウェイを追加する
  osmium::io::Reader r3(file_);
  construct_Handler3 h3(*this, memo);
  osmium::apply(r3, h3);
  r3.close();
}

void Graph::evaluate_from_osm() {
  using evaluate_NodeMemo = std::unordered_map<ID, Node *>;
  using evaluate_WayMemo = std::unordered_map<ID, std::vector<Way *>>;

  struct evaluate_Handler1 : public osmium::handler::Handler {
    Graph &graph;
    evaluate_NodeMemo &nm;
    evaluate_WayMemo &wm;
    evaluate_Handler1(Graph &graph, evaluate_NodeMemo &nm, evaluate_WayMemo &wm):
      graph(graph),
      nm(nm),
      wm(wm)
    {}
  
    void node(const osmium::Node &osm_node) {
      const auto &it = nm.find(osm_node.id());
      if(it == nm.end()) return;
  
      Node *node = it->second;
      for(int i = 0; i < graph.nc().dim(); i++) {
        node->indicators_[i] += graph.nc().evaluator(i).evaluate_from_osm(osm_node);
      }
    }
  
    void way(const osmium::Way &osm_way) {
      const auto &it = wm.find(osm_way.id());
      if(it == wm.end()) return;
  
      IndicatorList way_indicators(graph.wc().dim());
      for(int i = 0; i < graph.wc().dim(); i++) {
        way_indicators[i] = graph.wc().evaluator(i).evaluate_from_osm(osm_way);
      }
  
      for(Way *way : it->second) {
        for(int i = 0; i < graph.wc().dim(); i++) {
          way->indicators_[i] += way_indicators[i];
        }
      }
    }
  };

  evaluate_NodeMemo nm;
  evaluate_WayMemo wm;

  // 1. グラフにあるノード、ウェイをメモに追加
  for(auto &[id, node] : nodes()) {
    nm[node.id()] = &node;
    for(auto &way : node.ways()) {
      wm[way.id()].push_back(&way);
    }
  }

  // 2. メモにあるものに対応するデータを読む
  osmium::io::Reader r1(file_);
  evaluate_Handler1 h1(*this, nm, wm);
  osmium::apply(r1, h1);
  r1.close();
}

void Graph::evaluate_from_graph() {
  for(auto &[id, node] : nodes()) {
    for(int i = 0; i < nc().dim(); i++) {
      node.indicators_[i] += nc().evaluator(i).evaluate_from_graph(node);
    }

    for(auto &way : node.ways()) {
      for(int i = 0; i < wc().dim(); i++) {
        way.indicators_[i] += wc().evaluator(i).evaluate_from_graph(way, node, get(way.to()));
      }
    }
  }
}

void Graph::validate_indicators() {
  for(auto &[id, node] : nodes()) {
    for(int i = 0; i < nc().dim(); i++) {
      if(invalid_indicator(node.indicator(i))) {
        std::stringstream msg;
        msg << "無効な指標が発生: "
            << node.indicator(i)
            << "(" << nc().evaluator(i).name() << ")";

        node.indicators_[i] = 0;
        // throw std::domain_error(msg.str());
      }
    }

    for(auto &way : node.ways()) {
      for(int i = 0; i < wc().dim(); i++) {
        if(invalid_indicator(way.indicator(i))) {
          std::stringstream msg;
          // Way指標のエラーなので、検査中のWayから無効な値を取得します。
          msg << "無効な指標が発生: "
              << way.indicator(i)
              << "(" << wc().evaluator(i).name() << ")";

          way.indicators_[i] = 0;
          // throw std::domain_error(msg.str());
        }
      }
    }
  }
}



static inline void max_update(Indicator &x, Indicator y) {
  x = (x > y) ? x : y;
}
static inline void min_update(Indicator &x, Indicator y) {
  x = (x < y) ? x : y;
}

void Graph::find_max_indicators() {
  for(auto &[id, node] : nodes()) {
    for(int i = 0; i < nc().dim(); i++) {
      max_update(nc().max_indicators_[i], node.indicator(i));
      min_update(nc().min_indicators_[i], node.indicator(i));
    }

    for(auto &way : node.ways()) {
      for(int i = 0; i < wc().dim(); i++) {
        max_update(wc().max_indicators_[i], way.indicator(i));
        min_update(wc().min_indicators_[i], way.indicator(i));
      }
    }
  }
}

void Graph::normalize() {
  for(auto &[id, node] : nodes()) {
    for(int i = 0; i < nc().dim(); i++) {
      Indicator a = node.indicator(i) - nc().min_indicator(i);
      Indicator b = nc().max_indicator(i) - nc().min_indicator(i);
      
      if(b == 0) {
        node.indicators_[i] = 1;
      }else {
        node.indicators_[i] = a / b;
      }
    }

    for(auto &way : node.ways()) {
      for(int i = 0; i < wc().dim(); i++) {
        Indicator a = way.indicator(i) - wc().min_indicator(i);
        Indicator b = wc().max_indicator(i) - wc().min_indicator(i);
        
        if(b == 0) {
          way.indicators_[i] = 1;
        }else {
          way.indicators_[i] = a / b;
        }
      }
    }
  }
}



void Graph::resolve_idx() {
  IDX ni = 0;
  IDX wi = 0;

  for(auto &[id, node] : nodes()) {
    node.idx_ = ni++;

    for(auto &way : node.ways()) {
      way.idx_ = wi++;
    }
  }
}

void Graph::count_ways() {
  way_count_ = 0;
  for(const auto &[id, node] : nodes()) {
    for(const auto &way : node.ways()) {
      way_count_++;
    }
  }
}

void Graph::write(std::filesystem::path path) const {
  path += ".bg";

  FILE *out = fopen(path.string().c_str(), "wb");
  if(out == NULL) {
    throw std::runtime_error("[gp::Graph::write] fopen failed.");
  }

  bgOffset_t pos = BG_HEADER_POS;

  bgHeader_t header;
  bgError_t err = bgHeaderCreate(&header);
  if(err != BG_ERROR_OK) {
    throw std::runtime_error("[gp::Graph::write]");
  }
  
  pos += bgHeaderByteCount();

  header.nc_pos = pos;
  header.nc_dim = nc().dim();
  header.nc_ind_name_size = nc().indicator_name_size_;
  header.nc_ind_desc_size = nc().indicator_desc_size_;
  pos += bgNodeContextByteCount(header.nc_dim, header.nc_ind_name_size, header.nc_ind_desc_size);

  header.wc_pos = pos;
  header.wc_dim = wc().dim();
  header.wc_ind_name_size = wc().indicator_name_size_;
  header.wc_ind_desc_size = wc().indicator_desc_size_;
  pos += bgWayContextByteCount(header.wc_dim, header.wc_ind_name_size, header.wc_ind_desc_size);

  header.node_pos = pos;
  header.node_count = node_count();
  header.node_stride = bgNodeByteCount(header.nc_dim);
  pos += header.node_count * header.node_stride;

  header.way_pos = pos;
  header.way_count = way_count();
  header.way_stride = bgWayByteCount(header.wc_dim);

  bgHeaderWrite(&header, out);
  nc().write(out, &header);
  wc().write(out, &header);
  for(const auto &[id, node] : nodes()) {
    node.write(out, &header, nc());
    for(const auto &way : node.ways()) {
      way.write(out, &header, wc(), *this);
    }
  }

  bgHeaderDestroy(&header);
  fclose(out);
}

};
