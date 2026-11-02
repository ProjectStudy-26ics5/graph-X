#include <grapher.hpp>
#include <cmath>
#include <string>
#include <cstdio>

/* Node */
class Rank : public gp::NodeEvaluator {
  gp::IndicatorName name() const override {
    return "rank";
  }
  // 分岐の多さを、ユーザが重みを決める前に理解できるように説明します。
  gp::IndicatorDescription description() const override {
    return "Counts road connections beyond two. Higher values mean more branches.";
  }
  gp::Indicator evaluate_from_graph(const gp::Node &node) override {
    int rank = node.ways().size() - 2;
    return (rank > 0) ? rank : 0;
  }
};

class TrafficSignal : public gp::NodeEvaluator {
  gp::IndicatorName name() const override {
    return "traffic_signal";
  }
  // 信号機の有無がどの値に対応するかを説明します。
  gp::IndicatorDescription description() const override {
    return "Indicates a traffic signal: 1 when present, otherwise 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Node &osm_node) override {
    return osm_node.tags().has_tag("highway", "traffic_signals") ? 1 : 0;
  }
};



/* Way */
class Width : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "width";
  }
  // OSMに道幅がない場合の既定値も、実際の計算と一致するように説明します。
  gp::IndicatorDescription description() const override {
    return "Measures road width in meters. Missing OSM values default to 5 m.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (osm_way.tags().has_key("width")) {
      return std::atof(osm_way.tags().get_value_by_key("width"));
    }
    return 5;
  }
};

//detailと重複しているので,要検討
class Surface : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "surface";
  }
  // 基本的な舗装状態と、計算で使用する代表値を説明します。
  gp::IndicatorDescription description() const override {
    return "Rates basic surfaces: paved 1.0, unpaved 0.3, and other values 0.5.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (osm_way.tags().has_tag("surface", "paved")) return 1.0;
    if (osm_way.tags().has_tag("surface", "unpaved")) return 0.3;
    return 0.5;
  }
};

class Lanes : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "lanes";
  }
  // OSMに車線数がない場合の扱いも説明します。
  gp::IndicatorDescription description() const override {
    return "Measures the OSM lane count. Missing values default to 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (osm_way.tags().has_key("lanes")) {
      return std::atof(osm_way.tags().get_value_by_key("lanes"));
    }
    return 0;
  }
};

class CycleLane : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "cycle_lane";
  }
  // cyclewayタグの有無を0と1のどちらで表すか説明します。
  gp::IndicatorDescription description() const override {
    return "Indicates an OSM cycleway tag: 1 when present, otherwise 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    return osm_way.tags().has_key("cycleway") ? 1 : 0;
  }
};

class Oneway : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "oneway";
  }
  // 一方通行として扱う条件を説明します。
  gp::IndicatorDescription description() const override {
    return "Indicates one-way roads: 1 for oneway=yes, otherwise 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (osm_way.tags().has_tag("oneway", "yes")) return 1.0;
    if (osm_way.tags().has_tag("oneway:bicycle", "no")) return 0;
    return 0;
  }
};

class Maxspeed : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "maxspeed";
  }
  // OSMの最高速度と、値がない場合の既定値を説明します。
  gp::IndicatorDescription description() const override {
    return "Reads the numeric OSM maxspeed value. Missing values default to 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (osm_way.tags().has_key("maxspeed")) {
      return std::atof(osm_way.tags().get_value_by_key("maxspeed"));
    }
    return 0;
  }
};

class Height : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "height";
  }
  // OSMの高さと、値がない場合の既定値を説明します。
  gp::IndicatorDescription description() const override {
    return "Reads the numeric OSM height value. Missing values default to 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (osm_way.tags().has_key("height")) {
      return std::atof(osm_way.tags().get_value_by_key("height"));
    }
    return 0;
  }
};

class Surface_detail : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "surface_detail";
  }
  // 詳細な路面種別に割り当てる値の範囲を説明します。
  gp::IndicatorDescription description() const override {
    return "Rates detailed surfaces from 0.1 for dirt to 1.0 for asphalt.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
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

class Smoothness : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "smoothness";
  }
  // OSMのsmoothnessタグに対応する値の範囲を説明します。
  gp::IndicatorDescription description() const override {
    return "Rates OSM smoothness from 0.1 for horrible to 1.0 for excellent.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
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

class Cyclelane_detail : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "cyclelane_detail";
  }
  // 自転車道の種類ごとに割り当てる値を説明します。
  gp::IndicatorDescription description() const override {
    return "Rates cycleways: track 1.5, lane 1.0, shared lane 0.5, and opposite 0.3.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (!osm_way.tags().has_key("cycleway")) return 0;
    std::string v = osm_way.tags().get_value_by_key("cycleway");
    if (v == "lane")        return 1.0;
    if (v == "track")       return 1.5;
    if (v == "shared_lane") return 0.5;
    if (v == "opposite")    return 0.3;
    return 0;
  }
};

class Lit : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "lit";
  }
  // 道路照明の有無を0と1のどちらで表すか説明します。
  gp::IndicatorDescription description() const override {
    return "Indicates road lighting: 1 for lit=yes, otherwise 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (!osm_way.tags().has_key("lit")) return 0;
    std::string v = osm_way.tags().get_value_by_key("lit");
    if (v == "yes") return 1.0;
    if (v == "no")  return 0;
    return 0;
  }
};

class Barrier : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "barrier";
  }
  // 主な障害物の種類と、計算で使用する値を説明します。
  gp::IndicatorDescription description() const override {
    return "Rates barriers: gate 0.7, bollard 0.5, kerb 0.3, and other values 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (!osm_way.tags().has_key("barrier")) return 0;
    std::string v = osm_way.tags().get_value_by_key("barrier");
    if (v == "bollard") return 0.5;
    if (v == "gate")    return 0.7;
    if (v == "kerb")    return 0.3;
    return 0;
  }
};

class Tracktype : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "tracktype";
  }
  // OSMのtracktypeがgrade1からgrade5へ変わるときの値の範囲を説明します。
  gp::IndicatorDescription description() const override {
    return "Rates OSM track grade from 0.1 for grade5 to 1.0 for grade1.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
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

class Sac_scale : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "sac_scale";
  }
  // 主な登山道区分に割り当てる値を説明します。
  gp::IndicatorDescription description() const override {
    return "Rates hiking tags: hiking 1.0, mountain hiking 0.7, and alpine hiking 0.3.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (!osm_way.tags().has_key("sac_scale")) return 0;
    std::string v = osm_way.tags().get_value_by_key("sac_scale");
    if (v == "hiking")          return 1.0;
    if (v == "mountain_hiking") return 0.7;
    if (v == "alpine_hiking")   return 0.3;
    return 0;
  }
};

class Sidewalk : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "sidewalk";
  }
  // 歩道がある側の数に対応する値を説明します。
  gp::IndicatorDescription description() const override {
    return "Rates sidewalks: both sides 1.0, one side 0.5, and none 0.";
  }
  gp::Indicator evaluate_from_osm(const osmium::Way &osm_way) override {
    if (!osm_way.tags().has_key("sidewalk")) return 0;
    std::string v = osm_way.tags().get_value_by_key("sidewalk");
    if (v == "both")                return 1.0;
    if (v == "left" || v == "right") return 0.5;
    if (v == "none")                return 0;
    return 0;
  }
};

/* 未実装
class Slope : public gp::WayEvaluator {
  gp::IndicatorName name() const override {
    return "slope";
  }
  // 実装を有効にしたときにも説明が欠けないよう、予定する意味を書いておきます。
  gp::IndicatorDescription description() const override {
    return "Measures road slope. This indicator is not implemented yet.";
  }
  gp::Indicator evaluate_from_graph(const gp::Way &way, const gp::Node &from, const gp::Node &to) override {
    double horiz_dist = gp::Node::distance(from, to);
    if (horiz_dist < 0.00001) return 0;
    //return (to.altitude() - from.altitude()) / horiz_dist;
  }
};
*/



int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <region>\n", argv[0]);
    return 1;
  }

  std::string region = argv[1];
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

  std::string input = "data/" + region + "/source.osm.pbf";
  std::string output = "data/" + region + "/graph";

  gp::Graph graph(input.c_str(), nc, wc);
  graph.write(output.c_str());
  return 0;
}