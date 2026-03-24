#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>

struct Material { double k, cp, rho, alpha; };

// 1st degree: any tissue cell that reaches 44C or above
const double FIRST_DEG_TEMP = 44.0;

// Exponential threshold for 2nd Degree
double get_2nd_deg_limit(double T_c) {
    if (T_c < 44.0) return 1e15; 
    return 2.0e14 * std::exp(-0.543 * T_c);
}

// Exponential threshold for 3rd Degree (Higher energy requirement)
double get_3rd_deg_limit(double T_c) {
    if (T_c < 48.0) return 1e15; // 3rd degree rarely occurs below 48C regardless of time
    return 2.0e16 * std::exp(-0.580 * T_c);
}

struct TissueCell {
    double damage_2nd = 0.0;
    double damage_3rd = 0.0;
    bool   burned_1st = false;
};

std::map<std::string, double> load_config(std::string filename) {
    std::map<std::string, double> config;
    std::ifstream file(filename);
    std::string line, key, val;
    std::getline(file, line); 
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::getline(ss, key, ',');
        std::getline(ss, val, ',');
        config[key] = std::stod(val);
    }
    return config;
}

int main() {
    auto cfg = load_config("config.csv");
    double dx = 0.0001; 
    Material cheese = {0.38, 2720.0, 1050.0, 0.38 / (2720.0 * 1050.0)};
    Material tissue = {0.50, 3500.0, 1000.0, 0.50 / (3500.0 * 1000.0)};

    int nx_cheese = static_cast<int>((cfg["cheese_thickness_mm"]/1000.0) / dx);
    int nx_tissue = static_cast<int>((cfg["tissue_depth_mm"]/1000.0) / dx);
    int nx = nx_cheese + nx_tissue;

    double dt = 0.1 * (dx * dx) / std::max(cheese.alpha, tissue.alpha);
    int nt = static_cast<int>(cfg["total_sim_time"] / dt);
    int nt_contact = static_cast<int>(cfg["contact_time"] / dt);

    std::vector<double> T(nx, cfg["initial_mouth_temp"]);
    for (int i = 0; i < nx_cheese; ++i) T[i] = cfg["cheese_temp"];

    std::vector<TissueCell> palate(nx_tissue);

    std::ofstream sim_out("simulation_results.csv");
    sim_out << "Time_s,Depth_mm,Temperature_C,Burn1,Burn2,Burn3\n";

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

        if (n % static_cast<int>(0.1 / dt) == 0) {
            for (int i = nx_cheese; i < nx; ++i) {
                int tidx = i - nx_cheese;
                sim_out << n * dt << "," << tidx * dx * 1000.0 << "," 
                        << T[i] << "," << palate[tidx].burned_1st << ","
                        << palate[tidx].damage_2nd << "," 
                        << palate[tidx].damage_3rd << "\n";
            }
        }
    }
    sim_out.close();
    return 0;
}