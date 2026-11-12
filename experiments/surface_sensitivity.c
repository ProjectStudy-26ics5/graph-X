#include "experiment.h"
#include <math.h>
#include <stdint.h>

#define STEP_COUNT 40
#define MAP_COUNT 3

typedef struct {
    double lon;
    double lat;
} EPoint;

typedef struct {
    double weight;
    double length;
    double surface;
} EResult;

static EResult results[STEP_COUNT + 1];
static EPoint *map_points[MAP_COUNT];
static size_t map_sizes[MAP_COUNT];

static const double map_weights[MAP_COUNT] = {
    -1.0, 0.0, 1.0
};

static const char *map_colors[MAP_COUNT] = {
    "#2563eb", "#16a34a", "#dc2626"
};

static void collect_route(
    bgIdx_t surface_idx,
    int map_idx,
    EResult *result
) {
    double total_length = 0.0;
    double surface_sum = 0.0;

    size_t n = e_path.size;

    if (n == 0) {
        fprintf(stderr, "empty path\n");
        exit(1);
    }

    if (map_idx >= 0) {
        map_points[map_idx] = malloc(n * sizeof(EPoint));
        if (!map_points[map_idx]) {
            perror("malloc");
            exit(1);
        }
        map_sizes[map_idx] = n;
    }

    for (size_t i = 0; i < n; i++) {
        bgIdx_t current = e_path.node_idx_order[i];

        bgNodeFetch(
            &e_node, &e_reader, &e_header, current
        );

        if (map_idx >= 0) {
            map_points[map_idx][i].lon = e_node.lon;
            map_points[map_idx][i].lat = e_node.lat;
        }

        if (i + 1 >= n) {
            continue;
        }

        bgIdx_t next = e_path.node_idx_order[i + 1];
        int found = 0;

        for (bgIdx_t j = e_node.way_begin;
             j < e_node.way_end; j++) {

            bgWayFetch(&e_way, &e_reader, &e_header, j);

            if (e_way.to == next) {
                double length = e_way.length;
                double surface =
                    (double)e_way.indicators[surface_idx];

                total_length += length;
                surface_sum += length * surface;

                found = 1;
                break;
            }
        }

        if (!found) {
            fprintf(stderr,
                "way not found: %u -> %u\n",
                current, next);
            exit(1);
        }
    }

    result->length = total_length;
    result->surface =
        total_length > 0.0
        ? surface_sum / total_length
        : 0.0;
}

static double map_x(double lon,
                    double min_lon,
                    double max_lon) {
    return 100.0 + 800.0 *
        (lon - min_lon) / (max_lon - min_lon);
}

static double map_y(double lat,
                    double min_lat,
                    double max_lat) {
    return 800.0 - 280.0 *
        (lat - min_lat) / (max_lat - min_lat);
}

static void write_svg(const char *filename) {
    FILE *fp = fopen(filename, "w");

    if (!fp) {
        perror(filename);
        exit(1);
    }

    double min_lon = 1e100;
    double max_lon = -1e100;
    double min_lat = 1e100;
    double max_lat = -1e100;

    double min_surface = 1e100;
    double max_surface = -1e100;

    for (int m = 0; m < MAP_COUNT; m++) {
        for (size_t i = 0; i < map_sizes[m]; i++) {
            EPoint p = map_points[m][i];

            if (p.lon < min_lon) min_lon = p.lon;
            if (p.lon > max_lon) max_lon = p.lon;
            if (p.lat < min_lat) min_lat = p.lat;
            if (p.lat > max_lat) max_lat = p.lat;
        }
    }

    for (int i = 0; i <= STEP_COUNT; i++) {
        double v = results[i].surface;
        if (v < min_surface) min_surface = v;
        if (v > max_surface) max_surface = v;
    }

    if (max_lon == min_lon) max_lon += 0.000001;
    if (max_lat == min_lat) max_lat += 0.000001;
    if (max_surface == min_surface) {
        max_surface += 0.001;
        min_surface -= 0.001;
    }

    fprintf(fp,
        "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        "width=\"1000\" height=\"1000\" "
        "viewBox=\"0 0 1000 1000\">\n");

    fprintf(fp,
        "<rect width=\"1000\" height=\"1000\" fill=\"white\"/>\n");

    fprintf(fp,
        "<text x=\"500\" y=\"35\" text-anchor=\"middle\" "
        "font-size=\"23\" font-weight=\"bold\">"
        "Surface weight sensitivity</text>\n");

    /* 感度分析グラフ */

    fprintf(fp,
        "<text x=\"500\" y=\"75\" text-anchor=\"middle\" "
        "font-size=\"17\">"
        "Weight vs. length-weighted surface indicator</text>\n");

    fprintf(fp,
        "<line x1=\"100\" y1=\"100\" x2=\"100\" y2=\"400\" "
        "stroke=\"black\"/>"
        "<line x1=\"100\" y1=\"400\" x2=\"900\" y2=\"400\" "
        "stroke=\"black\"/>\n");

    fprintf(fp,
        "<text x=\"500\" y=\"440\" text-anchor=\"middle\" "
        "font-size=\"15\">Surface weight</text>\n");

    fprintf(fp,
        "<text x=\"25\" y=\"250\" text-anchor=\"middle\" "
        "font-size=\"15\" "
        "transform=\"rotate(-90 25 250)\">"
        "Mean surface indicator</text>\n");

    for (int i = 0; i <= STEP_COUNT; i++) {
        double x = 100.0 + 800.0 *
            results[i].weight / 2.0 + 400.0;

        double y = 400.0 - 300.0 *
            (results[i].surface - min_surface) /
            (max_surface - min_surface);

        fprintf(fp, "<circle cx=\"%.2f\" cy=\"%.2f\" "
                    "r=\"3\" fill=\"#2563eb\"/>\n", x, y);
    }

    for (int i = 0; i < STEP_COUNT; i++) {
        double x1 = 100.0 + 800.0 *
            (results[i].weight + 1.0) / 2.0;

        double x2 = 100.0 + 800.0 *
            (results[i + 1].weight + 1.0) / 2.0;

        double y1 = 400.0 - 300.0 *
            (results[i].surface - min_surface) /
            (max_surface - min_surface);

        double y2 = 400.0 - 300.0 *
            (results[i + 1].surface - min_surface) /
            (max_surface - min_surface);

        fprintf(fp,
            "<line x1=\"%.2f\" y1=\"%.2f\" "
            "x2=\"%.2f\" y2=\"%.2f\" "
            "stroke=\"#2563eb\" stroke-width=\"2\"/>\n",
            x1, y1, x2, y2);
    }

    /* 経路図 */

    fprintf(fp,
        "<text x=\"500\" y=\"490\" text-anchor=\"middle\" "
        "font-size=\"18\">"
        "Selected routes</text>\n");

    fprintf(fp,
        "<rect x=\"100\" y=\"520\" width=\"800\" height=\"280\" "
        "fill=\"none\" stroke=\"#999\"/>\n");

    for (int m = 0; m < MAP_COUNT; m++) {
        fprintf(fp, "<polyline fill=\"none\" "
                    "stroke=\"%s\" stroke-width=\"3\" "
                    "stroke-linejoin=\"round\" "
                    "stroke-linecap=\"round\" points=\"",
                map_colors[m]);

        for (size_t i = 0; i < map_sizes[m]; i++) {
            double x = map_x(
                map_points[m][i].lon,
                min_lon, max_lon
            );

            double y = map_y(
                map_points[m][i].lat,
                min_lat, max_lat
            );

            fprintf(fp, "%.2f,%.2f ", x, y);
        }

        fprintf(fp, "\"/>\n");
    }

    for (int m = 0; m < MAP_COUNT; m++) {
        fprintf(fp,
            "<line x1=\"%d\" y1=\"850\" x2=\"%d\" y2=\"850\" "
            "stroke=\"%s\" stroke-width=\"4\"/>"
            "<text x=\"%d\" y=\"855\" font-size=\"15\">"
            "Weight %.1f</text>\n",
            150 + m * 250, 185 + m * 250,
            map_colors[m], 195 + m * 250,
            map_weights[m]);
    }

    fprintf(fp, "</svg>\n");
    fclose(fp);
}


typedef struct {
    const char *name;
    double start_lon;
    double start_lat;
    double goal_lon;
    double goal_lat;
} ERegion;

static const ERegion regions[] = {
    {
        "Minato",
        139.7476, 35.6457,
        139.7765, 35.6305
    },
    {
        "Takao",
        139.2817457, 35.6416466,
        139.2698231, 35.6323974
    },
    {
        "Edogawa",
        139.8616612, 35.6443558,
        139.8650102, 35.6842343
    },
    {
        "Hachijo",
        139.81710, 33.12291,
        139.78270, 33.11664
    }
};


static void write_route_data(const char *region_name) {
    const char *suffix[MAP_COUNT] = {
        "m1", "zero", "p1"
    };

    for (int m = 0; m < MAP_COUNT; m++) {
        char filename[256];

        snprintf(filename, sizeof(filename),
            "experiments/%s_route_%s.dat",
            region_name, suffix[m]);

        FILE *fp = fopen(filename, "w");
        if (!fp) {
            perror(filename);
            exit(1);
        }

        for (size_t i = 0; i < map_sizes[m]; i++) {
            fprintf(fp, "%.8f %.8f\n",
                map_points[m][i].lon,
                map_points[m][i].lat);
        }

        fclose(fp);
        printf("Route data: %s\n", filename);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr,
            "Usage: %s <bg_file> <region>\n"
            "Regions: Minato Takao Edogawa Hachijo\n",
            argv[0]);
        return 1;
    }

    const ERegion *region = NULL;

    for (size_t i = 0;
         i < sizeof(regions) / sizeof(regions[0]); i++) {
        if (strcmp(argv[2], regions[i].name) == 0) {
            region = &regions[i];
            break;
        }
    }

    if (!region) {
        fprintf(stderr, "Unknown region: %s\n", argv[2]);
        return 1;
    }

    e_init(argv[1], E_READ_RAM);

    bgNodeFetchNearest(
        &e_node, &e_reader, &e_header,
        region->start_lon, region->start_lat
    );
    bgIdx_t start = e_node.idx;

    bgNodeFetchNearest(
        &e_node, &e_reader, &e_header,
        region->goal_lon, region->goal_lat
    );
    bgIdx_t goal = e_node.idx;

    printf("Region: %s\n", region->name);
    printf("Start node: %u\n", start);
    printf("Goal node: %u\n", goal);

    bgIdx_t surface_idx = e_way_ind_find("surface");

    printf("Surface indicator index: %u\n", surface_idx);

    for (int k = 0; k <= STEP_COUNT; k++) {
        double weight = -1.0 + 2.0 * k / STEP_COUNT;

        e_weight_zero();

        bgWayContextSetWeight(
            &e_wc, surface_idx, (bgWeight_t)weight
        );

        e_astar(start, goal, 1.0);

        int map_idx = -1;
        if (k == 0) map_idx = 0;
        if (k == STEP_COUNT / 2) map_idx = 1;
        if (k == STEP_COUNT) map_idx = 2;

        results[k].weight = weight;

        collect_route(
            surface_idx, map_idx, &results[k]
        );

        printf(
            "weight=%+.2f length=%.3f surface=%.5f nodes=%u\n",
            weight,
            results[k].length,
            results[k].surface,
            e_path.size
        );
    }

    write_route_data(region->name);

    char csv_name[256];
    char svg_name[256];

    snprintf(csv_name, sizeof(csv_name),
        "experiments/%s_surface_sensitivity.csv",
        region->name);

    snprintf(svg_name, sizeof(svg_name),
        "experiments/%s_surface_sensitivity.svg",
        region->name);

    FILE *csv = fopen(csv_name, "w");
    if (!csv) {
        perror(csv_name);
        e_end();
        return 1;
    }

    fprintf(csv, "weight,length,surface\n");

    for (int i = 0; i <= STEP_COUNT; i++) {
        fprintf(csv, "%.2f,%.6f,%.8f\n",
            results[i].weight,
            results[i].length,
            results[i].surface);
    }

    fclose(csv);

    write_svg(svg_name);

    for (int i = 0; i < MAP_COUNT; i++) {
        free(map_points[i]);
    }

    e_end();

    printf("\nOutput:\n%s\n%s\n", csv_name, svg_name);

    return 0;
}
