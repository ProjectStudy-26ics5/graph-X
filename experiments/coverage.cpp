#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cmath>

#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

#include <beltgraph.h>

// ============================================================
// Way coverage
// ============================================================

struct TagResult {
    std::string key;
    int64_t total = 0;
    int64_t tagged = 0;
};

struct CompletenessHandler : public osmium::handler::Handler {
    int64_t total_roads = 0;
    std::vector<TagResult> results;

    CompletenessHandler(const std::vector<std::string>& tags) {
        for (const auto& tag : tags) {
            results.push_back({tag, 0, 0});
        }
    }

    void way(const osmium::Way& way) {
        if (!way.tags().has_key("highway")) {
            return;
        }

        total_roads++;

        for (auto& result : results) {
            result.total++;

            if (way.tags().has_key(result.key.c_str())) {
                result.tagged++;
            }
        }
    }
};

// ============================================================
// Node coverage
// ============================================================

static FILE* bg_file = nullptr;

static void bg_read_callback(
    bgOffset_t pos,
    void* buffer,
    bgSize_t size
) {
    fseek(bg_file, pos, SEEK_SET);
    fread(buffer, size, 1, bg_file);
}

static bgIdx_t find_node_indicator(
    const bgNodeContext_t& nc,
    const char* name
) {
    for (bgIdx_t i = 0; i < nc.dim; i++) {
        if (std::string(bgNodeContextIndicatorName(&nc, i)) == name) {
            return i;
        }
    }

    return static_cast<bgIdx_t>(-1);
}

static void analyze_nodes(const std::string& path) {
    bg_file = fopen(path.c_str(), "rb");

    if (bg_file == nullptr) {
        throw std::runtime_error("Cannot open graph.bg: " + path);
    }

    bgReader_t reader;
    bgHeader_t header;
    bgNodeContext_t nc;
    bgNode_t node;

    bgReaderCreate(&reader, bg_read_callback);

    bgHeaderCreate(&header);
    bgHeaderFetch(&header, &reader);

    bgNodeContextCreate(&nc, &header);
    bgNodeContextFetch(&nc, &reader, &header);

    bgNodeCreate(&node, &header);

    const bgIdx_t rank_idx =
        find_node_indicator(nc, "rank");

    const bgIdx_t signal_idx =
        find_node_indicator(nc, "traffic_signal");

    if (rank_idx == static_cast<bgIdx_t>(-1)) {
        throw std::runtime_error("Node indicator not found: rank");
    }

    if (signal_idx == static_cast<bgIdx_t>(-1)) {
        throw std::runtime_error(
            "Node indicator not found: traffic_signal"
        );
    }

    int64_t rank_nonzero = 0;
    int64_t signal_present = 0;

    double rank_min = INFINITY;
    double rank_max = -INFINITY;
    double rank_sum = 0.0;

    for (bgIdx_t i = 0; i < header.node_count; i++) {
        bgNodeFetch(&node, &reader, &header, i);

        const double rank = node.indicators[rank_idx];
        const double signal = node.indicators[signal_idx];

        if (rank > 0.0) {
            rank_nonzero++;
        }

        if (signal > 0.0) {
            signal_present++;
        }

        if (rank < rank_min) rank_min = rank;
        if (rank > rank_max) rank_max = rank;

        rank_sum += rank;
    }

    const double total = static_cast<double>(header.node_count);

    std::cout << "\n========== Node indicators ==========\n";
    std::cout << "Graph: " << path << "\n";
    std::cout << "Total Nodes: " << header.node_count << "\n\n";

    std::cout << std::left
              << std::setw(20) << "Indicator"
              << std::right
              << std::setw(15) << "Non-zero"
              << std::setw(15) << "Total"
              << std::setw(12) << "Rate"
              << "\n";

    std::cout << std::left
              << std::setw(20) << "rank"
              << std::right
              << std::setw(15) << rank_nonzero
              << std::setw(15) << header.node_count
              << std::setw(11)
              << std::fixed << std::setprecision(2)
              << (total > 0 ? rank_nonzero / total * 100.0 : 0.0)
              << "%\n";

    std::cout << std::left
              << std::setw(20) << "traffic_signal"
              << std::right
              << std::setw(15) << signal_present
              << std::setw(15) << header.node_count
              << std::setw(11)
              << std::fixed << std::setprecision(2)
              << (total > 0 ? signal_present / total * 100.0 : 0.0)
              << "%\n";

    std::cout << "\nrank min: " << rank_min
              << "\nrank max: " << rank_max
              << "\nrank mean: "
              << (total > 0 ? rank_sum / total : 0.0)
              << "\n";

    bgNodeDestroy(&node);
    bgNodeContextDestroy(&nc);
    fclose(bg_file);
    bg_file = nullptr;
}

// ============================================================
// Main
// ============================================================

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr
            << "Usage: " << argv[0]
            << " <osm_data.osm.pbf> <graph.bg>"
            << " [way_tag1] [way_tag2] ...\n";
        return 1;
    }

    const std::string pbf_path = argv[1];
    const std::string graph_path = argv[2];

    std::vector<std::string> tags;

    for (int i = 3; i < argc; i++) {
        tags.push_back(argv[i]);
    }

    try {
        osmium::io::Reader reader{
            pbf_path,
            osmium::io::read_meta::no
        };

        CompletenessHandler handler{tags};

        osmium::apply(reader, handler);
        reader.close();

        std::cout << "========== Way indicators ==========\n";
        std::cout << "PBF: " << pbf_path << "\n";
        std::cout << "Total highway Ways: "
                  << handler.total_roads << "\n\n";

        std::cout << std::left
                  << std::setw(20) << "Tag"
                  << std::right
                  << std::setw(15) << "Registered"
                  << std::setw(15) << "Total"
                  << std::setw(12) << "Rate"
                  << "\n";

        for (const auto& result : handler.results) {
            double rate = 0.0;

            if (result.total > 0) {
                rate = static_cast<double>(result.tagged)
                     / result.total * 100.0;
            }

            std::cout << std::left
                      << std::setw(20) << result.key
                      << std::right
                      << std::setw(15) << result.tagged
                      << std::setw(15) << result.total
                      << std::setw(11)
                      << std::fixed << std::setprecision(2)
                      << rate << "%\n";
        }

        analyze_nodes(graph_path);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}