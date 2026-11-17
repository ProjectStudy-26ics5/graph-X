
#include "../experiments/experiment.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

struct Experiment {
    const char *name;
    bool is_node;
    bgIdx_t index;
};

static bool find_indicator(
    bool is_node,
    const char *name,
    bgIdx_t &result
) {
    if (is_node) {
        for (bgIdx_t i = 0; i < e_nc.dim; ++i) {
            if (std::string(bgNodeContextIndicatorName(&e_nc, i)) == name) {
                result = i;
                return true;
            }
        }
    } else {
        for (bgIdx_t i = 0; i < e_wc.dim; ++i) {
            if (std::string(bgWayContextIndicatorName(&e_wc, i)) == name) {
                result = i;
                return true;
            }
        }
    }
    return false;
}

int main(int argc, char **argv) {
    if (argc != 6) {
        std::cerr
            << "Usage: " << argv[0]
            << " <bg_file> <start_lon> <start_lat>"
            << " <goal_lon> <goal_lat>\n";
        return 1;
    }

    e_init(argv[1], E_READ_RAM);

    const double start_lon = atof(argv[2]);
    const double start_lat = atof(argv[3]);
    const double goal_lon = atof(argv[4]);
    const double goal_lat = atof(argv[5]);

    // 使用する4指標を名前で特定する
    Experiment experiments[] = {
        {"width", false, 0},
        {"rank", true, 0},
        {"surface", false, 0},
        {"traffic_signal", true, 0}
    };

    const int experiment_count = 4;

    for (auto &exp : experiments) {
        if (!find_indicator(exp.is_node, exp.name, exp.index)) {
            std::cerr << "Indicator not found: " << exp.name << "\n";
            e_end();
            return 1;
        }
        std::cout << exp.name << " index=" << exp.index << "\n";
    }

    bgNodeFetchNearest(
        &e_node, &e_reader, &e_header, start_lon, start_lat
    );
    const bgIdx_t start_idx = e_node.idx;

    bgNodeFetchNearest(
        &e_node, &e_reader, &e_header, goal_lon, goal_lat
    );
    const bgIdx_t goal_idx = e_node.idx;

    std::cout << "Start node: " << start_idx
              << ", Goal node: " << goal_idx << "\n";

    // 道路ネットワークを書き出す
    {
        std::ofstream net("visualizer/network.json");
        if (!net) {
            std::cerr << "Cannot open network.json\n";
            e_end();
            return 1;
        }

        net << std::setprecision(10);
        net << "{\n\"edges\": [\n";

        bool first = true;

        for (bgIdx_t n = 0; n < e_header.node_count; ++n) {
            bgNodeFetch(&e_node, &e_reader, &e_header, n);

            const double from_lat = e_node.lat;
            const double from_lon = e_node.lon;

            for (bgIdx_t w = e_node.way_begin;
                 w < e_node.way_end; ++w) {

                bgWayFetch(&e_way, &e_reader, &e_header, w);

                bgNodeFetch(
                    &e_node, &e_reader, &e_header, e_way.to
                );

                if (!first) net << ",\n";
                first = false;

                net << "["
                    << from_lat << "," << from_lon << ","
                    << e_node.lat << "," << e_node.lon
                    << "]";
                
                // 次のループで参照するノードを再取得する
                bgNodeFetch(&e_node, &e_reader, &e_header, n);
            }
        }

        net << "\n]}\n";
        std::cout << "Saved: visualizer/network.json\n";
    }

    std::ofstream paths("visualizer/paths.json");
    std::ofstream metrics("visualizer/metrics.csv");

    if (!paths || !metrics) {
        std::cerr << "Cannot open output files\n";
        e_end();
        return 1;
    }

    paths << std::setprecision(10);
    paths << "{\n\"paths\": [\n";

    metrics
        << "indicator,weight,path_length_m,node_count,indicator_mean\n";

    bool first_path = true;

    for (int exp_no = 0; exp_no < experiment_count; ++exp_no) {
        const Experiment &exp = experiments[exp_no];

        for (int step = 0; step <= 10; ++step) {
            e_weight_zero();

            const bgWeight_t weight =
                static_cast<bgWeight_t>(step) / 10.0f;

            // 対象のNodeまたはWayだけに重みを設定
            if (exp.is_node) {
                bgNodeContextSetWeight(
                    &e_nc, exp.index, weight
                );
            } else {
                bgWayContextSetWeight(
                    &e_wc, exp.index, weight
                );
            }

            e_astar(start_idx, goal_idx, 1);

            if (e_path.size == 0) {
                std::cerr << "Empty path: "
                          << exp.name << " weight=" << weight << "\n";
                continue;
            }

            double path_length = 0.0;
            double indicator_sum = 0.0;
            unsigned long long indicator_count = 0;

            // Node指標の平均値
            if (exp.is_node) {
                for (bgIdx_t i = 0; i < e_path.size; ++i) {
                    bgNodeFetch(
                        &e_node, &e_reader, &e_header,
                        e_path.node_idx_order[i]
                    );

                    const double value =
                        e_node.indicators[exp.index];

                    if (std::isfinite(value)) {
                        indicator_sum += value;
                        ++indicator_count;
                    }
                }
            }

            // 経路上のWayを取得して距離・指標値を集計
            for (bgIdx_t i = 0; i + 1 < e_path.size; ++i) {
                const bgIdx_t from =
                    e_path.node_idx_order[i];
                const bgIdx_t to =
                    e_path.node_idx_order[i + 1];

                bgNodeFetch(
                    &e_node, &e_reader, &e_header, from
                );

                bool found = false;

                for (bgIdx_t w = e_node.way_begin;
                     w < e_node.way_end; ++w) {

                    bgWayFetch(&e_way, &e_reader, &e_header, w);

                    if (e_way.to == to) {
                        path_length += e_way.length;

                        if (!exp.is_node) {
                            const double value =
                                e_way.indicators[exp.index];

                            if (std::isfinite(value)) {
                                indicator_sum += value;
                                ++indicator_count;
                            }
                        }

                        found = true;
                        break;
                    }
                }

                if (!found) {
                    std::cerr << "Warning: path edge not found: "
                              << from << " -> " << to << "\n";
                }
            }

            const double indicator_mean =
                indicator_count > 0
                    ? indicator_sum / indicator_count
                    : 0.0;

            // 経路座標をJSONに保存
            if (!first_path) paths << ",\n";
            first_path = false;

            paths << "{\"indicator\":\"" << exp.name
                  << "\",\"weight\":" << weight
                  << ",\"coordinates\":[";

            for (bgIdx_t i = 0; i < e_path.size; ++i) {
                bgNodeFetch(
                    &e_node, &e_reader, &e_header,
                    e_path.node_idx_order[i]
                );

                if (i) paths << ",";
                paths << "[" << e_node.lat
                      << "," << e_node.lon << "]";
            }

            paths << "]}";

            metrics << exp.name << ","
                    << weight << ","
                    << path_length << ","
                    << e_path.size << ","
                    << indicator_mean << "\n";

            std::cout << exp.name
                      << " weight=" << weight
                      << " nodes=" << e_path.size
                      << " length=" << path_length
                      << " mean=" << indicator_mean
                      << "\n";
        }
    }

    paths << "\n]}\n";

    paths.close();
    metrics.close();

    e_end();

    std::cout << "Saved: visualizer/paths.json\n";
    std::cout << "Saved: visualizer/metrics.csv\n";

    return 0;
}
