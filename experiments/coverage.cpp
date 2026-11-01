#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdint>

#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

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

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <osm_data.osm.pbf> <tag1> [tag2] [tag3] ...\n";
        return 1;
    }

    std::string pbf_path = argv[1];

    std::vector<std::string> tags;
    for (int i = 2; i < argc; i++) {
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

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
