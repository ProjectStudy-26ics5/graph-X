#include <grapher.hpp>
#include <cmath>
#include <string>

/* Node */
struct Rank : public gp::NodeEvaluator {
  gp::IndicatorName name() const {
    return "rank";
  }
  // 分岐の多さを、ユーザが重みを決める前に理解できるように説明します。
  gp::IndicatorDescription description() const {
    return "Counts road connections beyond two. Higher values mean more branches.";
  }
  gp::Indicator evaluate_from_graph(const gp::Node &node) {
    int rank = node.ways().size() - 2;
    return (rank > 0) ? rank : 0;
  }
};

struct TrafficSignal : public gp::NodeEvaluator {
  gp::IndicatorName name() const {
    return "traffic_signal";
  }
  // 信号機の有無がどの値に対応するかを説明します。
  gp::IndicatorDescription description() const {
    return "Indicates a traffic signal: 1 when present, otherwise 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Node &osm_node) {
    return osm_node.tags().has_tag("highway", "traffic_signals") ? 1 : 0;
  }
};



/* Way */
struct Width : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "width";
  }
  // OSMに道幅がない場合の既定値も、実際の計算と一致するように説明します。
  gp::IndicatorDescription description() const {
    return "Measures road width in meters. Missing OSM values default to 5 m.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (osm_way.tags().has_key("width")) {
      return std::atof(osm_way.tags().get_value_by_key("width"));
    }
    return 5;
  }
};

//detailと重複しているので,要検討
struct Surface : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "surface";
  }
  // 基本的な舗装状態と、計算で使用する代表値を説明します。
  gp::IndicatorDescription description() const {
    return "Rates basic surfaces: paved 1.0, unpaved 0.3, and other values 0.5.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (osm_way.tags().has_tag("surface", "paved")) return 1.0;
    if (osm_way.tags().has_tag("surface", "unpaved")) return 0.3;
    return 0.5;
  }
};

struct Lanes : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "lanes";
  }
  // OSMに車線数がない場合の扱いも説明します。
  gp::IndicatorDescription description() const {
    return "Measures the OSM lane count. Missing values default to 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (osm_way.tags().has_key("lanes")) {
      return std::atof(osm_way.tags().get_value_by_key("lanes"));
    }
    return 0;
  }
};

struct CycleLane : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "cycle_lane";
  }
  // cyclewayタグの有無を0と1のどちらで表すか説明します。
  gp::IndicatorDescription description() const {
    return "Indicates an OSM cycleway tag: 1 when present, otherwise 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    return osm_way.tags().has_key("cycleway") ? 1 : 0;
  }
};

struct Oneway : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "oneway";
  }
  // 一方通行として扱う条件を説明します。
  gp::IndicatorDescription description() const {
    return "Indicates one-way roads: 1 for oneway=yes, otherwise 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (osm_way.tags().has_tag("oneway", "yes")) return 1.0;
    if (osm_way.tags().has_tag("oneway:bicycle", "no")) return 0;
    return 0;
  }
};

struct Maxspeed : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "maxspeed";
  }
  // OSMの最高速度と、値がない場合の既定値を説明します。
  gp::IndicatorDescription description() const {
    return "Reads the numeric OSM maxspeed value. Missing values default to 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (osm_way.tags().has_key("maxspeed")) {
      return std::atof(osm_way.tags().get_value_by_key("maxspeed"));
    }
    return 0;
  }
};

struct Height : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "height";
  }
  // OSMの高さと、値がない場合の既定値を説明します。
  gp::IndicatorDescription description() const {
    return "Reads the numeric OSM height value. Missing values default to 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (osm_way.tags().has_key("height")) {
      return std::atof(osm_way.tags().get_value_by_key("height"));
    }
    return 0;
  }
};

struct Surface_detail : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "surface_detail";
  }
  // 詳細な路面種別に割り当てる値の範囲を説明します。
  gp::IndicatorDescription description() const {
    return "Rates detailed surfaces from 0.1 for dirt to 1.0 for asphalt.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (!osm_way.tags().has_key("surface")) return 0.5;
    std::string v = osm_way.tags().get_value_by_key("surface");
    if (v == "asphalt") return 1.0;
    if (v == "paved")   return 0.9;
    if (v == "gravel")  return 0.3;
    if (v == "dirt")    return 0.1;
    if (v == "ground")  return 0.2;
    return 0.5;
  }
};

struct Smoothness : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "smoothness";
  }
  // OSMのsmoothnessタグに対応する値の範囲を説明します。
  gp::IndicatorDescription description() const {
    return "Rates OSM smoothness from 0.1 for horrible to 1.0 for excellent.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (!osm_way.tags().has_key("smoothness")) return 0.5;
    std::string v = osm_way.tags().get_value_by_key("smoothness");
    if (v == "excellent")    return 1.0;
    if (v == "good")        return 0.8;
    if (v == "intermediate") return 0.6;
    if (v == "bad")          return 0.3;
    if (v == "very_bad")     return 0.2;
    if (v == "horrible")     return 0.1;
    return 0.5;
  }
};

struct Cyclelane_detail : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "cyclelane_detail";
  }
  // 自転車道の種類ごとに割り当てる値を説明します。
  gp::IndicatorDescription description() const {
    return "Rates cycleways: track 1.5, lane 1.0, shared lane 0.5, and opposite 0.3.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (!osm_way.tags().has_key("cycleway")) return 0;
    std::string v = osm_way.tags().get_value_by_key("cycleway");
    if (v == "lane")        return 1.0;
    if (v == "track")       return 1.5;
    if (v == "shared_lane") return 0.5;
    if (v == "opposite")    return 0.3;
    return 0;
  }
};

struct Lit : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "lit";
  }
  // 道路照明の有無を0と1のどちらで表すか説明します。
  gp::IndicatorDescription description() const {
    return "Indicates road lighting: 1 for lit=yes, otherwise 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (!osm_way.tags().has_key("lit")) return 0;
    std::string v = osm_way.tags().get_value_by_key("lit");
    if (v == "yes") return 1.0;
    if (v == "no")  return 0;
    return 0;
  }
};

struct Barrier : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "barrier";
  }
  // 主な障害物の種類と、計算で使用する値を説明します。
  gp::IndicatorDescription description() const {
    return "Rates barriers: gate 0.7, bollard 0.5, kerb 0.3, and other values 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (!osm_way.tags().has_key("barrier")) return 0;
    std::string v = osm_way.tags().get_value_by_key("barrier");
    if (v == "bollard") return 0.5;
    if (v == "gate")    return 0.7;
    if (v == "kerb")    return 0.3;
    return 0;
  }
};

struct Tracktype : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "tracktype";
  }
  // OSMのtracktypeがgrade1からgrade5へ変わるときの値の範囲を説明します。
  gp::IndicatorDescription description() const {
    return "Rates OSM track grade from 0.1 for grade5 to 1.0 for grade1.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (!osm_way.tags().has_key("tracktype")) return 0;
    std::string v = osm_way.tags().get_value_by_key("tracktype");
    if (v == "grade1") return 1.0;
    if (v == "grade2") return 0.8;
    if (v == "grade3") return 0.5;
    if (v == "grade4") return 0.2;
    if (v == "grade5") return 0.1;
    return 0;
  }
};

struct Sac_scale : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "sac_scale";
  }
  // 主な登山道区分に割り当てる値を説明します。
  gp::IndicatorDescription description() const {
    return "Rates hiking tags: hiking 1.0, mountain hiking 0.7, and alpine hiking 0.3.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (!osm_way.tags().has_key("sac_scale")) return 0;
    std::string v = osm_way.tags().get_value_by_key("sac_scale");
    if (v == "hiking")          return 1.0;
    if (v == "mountain_hiking") return 0.7;
    if (v == "alpine_hiking")   return 0.3;
    return 0;
  }
};

struct Sidewalk : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "sidewalk";
  }
  // 歩道がある側の数に対応する値を説明します。
  gp::IndicatorDescription description() const {
    return "Rates sidewalks: both sides 1.0, one side 0.5, and none 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) {
    if (!osm_way.tags().has_key("sidewalk")) return 0;
    std::string v = osm_way.tags().get_value_by_key("sidewalk");
    if (v == "both")                return 1.0;
    if (v == "left" || v == "right") return 0.5;
    if (v == "none")                return 0;
    return 0;
  }
};

/* 未実装
struct Slope : public gp::WayEvaluator {
  gp::IndicatorName name() const {
    return "slope";
  }
  // 実装を有効にしたときにも説明が欠けないよう、予定する意味を書いておきます。
  gp::IndicatorDescription description() const {
    return "Measures road slope. This indicator is not implemented yet.";
  }
  gp::Indicator evaluate_from_graph(const gp::Way &way, const gp::Node &from, const gp::Node &to) {
    double horiz_dist = gp::Node::distance(from, to);
    if (horiz_dist < 0.00001) return 0;
    //return (to.altitude() - from.altitude()) / horiz_dist;
  }
};
*/



int main(void) {
  Rank rank;
  TrafficSignal trafficSignal;

  Width width;
  Surface surface;
  Lanes lanes;
  CycleLane cyclelane;
  Oneway oneway;
  Maxspeed maxspeed;
  Height height;
  Surface_detail surface_detail;
  Lit lit;
  Barrier barrier;
  Tracktype tracktype;
  Sac_scale sac_scale;
  Sidewalk sidewalk;

  gp::NodeContext nc({
    &rank,
    &trafficSignal
  });
  gp::WayContext wc({
    &width,
    &surface,
    &lanes,
    &cyclelane,
    &oneway,
    &maxspeed,
    &height,
    &surface_detail,
    &lit,
    &barrier,
    &tracktype,
    &sac_scale,
    &sidewalk
  });

  gp::Graph graph("osm/minato.osm.pbf", nc, wc);
  graph.write("out/test");
  return 0;
}
