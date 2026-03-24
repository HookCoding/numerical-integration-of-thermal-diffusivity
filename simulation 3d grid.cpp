#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>

struct Material { double k, cp, rho, alpha; };

// 1st degree: any tissue cell that reaches 44C or above is a 1st degree burn
const double FIRST_DEG_TEMP = 44.0;

// Exponential threshold for 2nd Degree
double get_2nd_deg_limit(double T_c) {
    if (T_c < 44.0) return 1e15;
    return 2.0e14 * std::exp(-0.543 * T_c);
}

// Exponential threshold for 3rd Degree (Higher energy requirement)
double get_3rd_deg_limit(double T_c) {
    if (T_c < 48.0) return 1e15;
    return 2.0e16 * std::exp(-0.580 * T_c);
}

struct TissueCell {
    double damage_2nd  = 0.0;
    double damage_3rd  = 0.0;
    bool   burned_1st  = false;
};

struct BurnResult {
    double depth_1st_mm;
    double depth_2nd_mm;
    double depth_3rd_mm;
};

std::map<std::string, double> load_config(std::string filename) {
    std::map<std::string, double> config;
    std::ifstream file(filename);
    std::string line, key, val;
    std::getline(file, line); // skip header
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::getline(ss, key, ',');
        std::getline(ss, val, ',');
        config[key] = std::stod(val);
    }
    return config;
}

// Run a single simulation and return 1st, 2nd, and 3rd degree burn depths in mm
BurnResult run_simulation(const std::map<std::string, double>& cfg,
                          double cheese_temp, double contact_time) {
    double dx = 0.0001;
    Material cheese = {0.38, 2720.0, 1050.0, 0.38 / (2720.0 * 1050.0)};
    Material tissue = {0.50, 3500.0, 1000.0, 0.50 / (3500.0 * 1000.0)};

    int nx_cheese = static_cast<int>((cfg.at("cheese_thickness_mm") / 1000.0) / dx);
    int nx_tissue = static_cast<int>((cfg.at("tissue_depth_mm")     / 1000.0) / dx);
    int nx = nx_cheese + nx_tissue;

    double dt = 0.1 * (dx * dx) / std::max(cheese.alpha, tissue.alpha);
    int nt         = static_cast<int>(cfg.at("total_sim_time") / dt);
    int nt_contact = static_cast<int>(contact_time / dt);

    std::vector<double> T(nx, cfg.at("initial_mouth_temp"));
    for (int i = 0; i < nx_cheese; ++i) T[i] = cheese_temp;

    std::vector<TissueCell> palate(nx_tissue);

    for (int n = 0; n < nt; ++n) {
        std::vector<double> T_new = T;
        for (int i = 1; i < nx - 1; ++i) {
            double alpha = (i < nx_cheese) ? cheese.alpha : tissue.alpha;
            T_new[i] = T[i] + alpha * (dt / (dx * dx)) * (T[i + 1] - 2 * T[i] + T[i - 1]);
        }
        if (n > nt_contact) T_new[nx_cheese] = T_new[nx_cheese + 1];
        T = T_new;

        for (int i = nx_cheese; i < nx; ++i) {
            int tidx = i - nx_cheese;
            // 1st degree: latch true if tissue ever reaches 44C
            if (T[i] >= FIRST_DEG_TEMP)
                palate[tidx].burned_1st = true;
            palate[tidx].damage_2nd += (dt / get_2nd_deg_limit(T[i]));
            palate[tidx].damage_3rd += (dt / get_3rd_deg_limit(T[i]));
        }
    }

    BurnResult result = {0.0, 0.0, 0.0};
    for (int i = 0; i < nx_tissue; ++i) {
        double depth_mm = i * dx * 1000.0;
        if (palate[i].burned_1st)  result.depth_1st_mm = depth_mm;
        if (palate[i].damage_2nd >= 1.0) result.depth_2nd_mm = depth_mm;
        if (palate[i].damage_3rd >= 1.0) result.depth_3rd_mm = depth_mm;
    }
    return result;
}

int main() {
    auto cfg = load_config("config.csv");

    int n_temp = static_cast<int>(cfg["grid_steps_temp"]);
    int n_time = static_cast<int>(cfg["grid_steps_time"]);

    double temp_min = cfg["initial_mouth_temp"];
    double temp_max = cfg["cheese_temp"];
    double time_min = 0.0;
    double time_max = cfg["contact_time"];

    double temp_step = (temp_max - temp_min) / (n_temp - 1);
    double time_step = (time_max - time_min) / (n_time - 1);

    std::ofstream grid_1st("grid_1st.csv");
    std::ofstream grid_2nd("grid_2nd.csv");
    std::ofstream grid_3rd("grid_3rd.csv");

    // Header rows
    for (auto* out : {&grid_1st, &grid_2nd, &grid_3rd}) {
        *out << "Temp_C\\Time_s";
        for (int j = 0; j < n_time; ++j)
            *out << "," << (time_min + j * time_step);
        *out << "\n";
    }

    int total = n_temp * n_time;
    int done  = 0;

    for (int i = 0; i < n_temp; ++i) {
        double temp = temp_min + i * temp_step;
        grid_1st << temp;
        grid_2nd << temp;
        grid_3rd << temp;
        for (int j = 0; j < n_time; ++j) {
            double contact_time = time_min + j * time_step;
            BurnResult result = run_simulation(cfg, temp, contact_time);
            grid_1st << "," << result.depth_1st_mm;
            grid_2nd << "," << result.depth_2nd_mm;
            grid_3rd << "," << result.depth_3rd_mm;
            ++done;
            std::cout << "\rProgress: " << done << "/" << total
                      << "  (" << static_cast<int>(100.0 * done / total) << "%)"
                      << std::flush;
        }
        grid_1st << "\n";
        grid_2nd << "\n";
        grid_3rd << "\n";
    }

    grid_1st.close();
    grid_2nd.close();
    grid_3rd.close();
    std::cout << "\nGrid scan complete. Results saved to grid_1st.csv, grid_2nd.csv, grid_3rd.csv\n";
    return 0;
}