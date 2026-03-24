# Thermal Burn Simulation

A physics-based simulation of heat transfer from hot food (e.g. melted cheese) into oral tissue, modelling 1st, 2nd, and 3rd degree burn injury across a parameter space of food temperatures and contact times.

---

## Overview

This project simulates the transient conduction of heat from a hot food bolus into the palate, then applies empirical burn damage models to predict the depth and severity of thermal injury. Two modes are supported:

- **Single simulation** — detailed time-resolved output for one set of conditions
- **Grid scan** — sweeps across a range of food temperatures and contact times to build parameter-space maps

Results are visualised as contour plots showing burn depth as a function of temperature and exposure duration.

---

## Repository Structure

```
.
├── simulation_3d_single.cpp   # Single-run simulation (C++)
├── simulation_3d_grid.cpp     # Parameter grid scan (C++)
├── simulation_grid.py         # Visualisation of results (Python)
├── config.csv                 # Simulation parameters
└── README.md
```

---

## Physics

Heat conduction is solved with an explicit finite-difference scheme (1D):

$$T_i^{n+1} = T_i^n + \alpha \frac{\Delta t}{\Delta x^2} \left( T_{i+1}^n - 2T_i^n + T_{i-1}^n \right)$$

Stability is enforced by setting $\Delta t = 0.1 \cdot \Delta x^2 / \alpha_{\max}$.

### Material Properties

| Layer  | $k$ (W/m·K) | $c_p$ (J/kg·K) | $\rho$ (kg/m³) |
|--------|:-----------:|:--------------:|:--------------:|
| Cheese | 0.38        | 2720           | 1050           |
| Tissue | 0.50        | 3500           | 1000           |

Spatial resolution: $\Delta x = 0.1\,\text{mm}$

---

## Burn Damage Models

### 1st Degree
Any tissue node that reaches **≥ 44 °C** is flagged as a 1st-degree burn (latched — once burned, always burned).

### 2nd and 3rd Degree
Cumulative damage is integrated using temperature-dependent Arrhenius-style rate functions:

$$\Omega(t) = \int_0^t \frac{dt'}{\tau(T)}$$

Burn is predicted when $\Omega \geq 1$.

| Degree | Threshold temp | Rate function $\tau(T)$ |
|--------|:--------------:|:-----------------------:|
| 2nd    | 44 °C          | $2 \times 10^{14} \cdot e^{-0.543\,T}$ |
| 3rd    | 48 °C          | $2 \times 10^{16} \cdot e^{-0.580\,T}$ |

---

## Configuration (`config.csv`)

| Parameter | Description |
|-----------|-------------|
| `cheese_temp` | Initial food temperature (°C) |
| `initial_mouth_temp` | Baseline oral tissue temperature (°C) |
| `contact_time` | Duration of food contact (s) |
| `total_sim_time` | Total simulation duration (s) |
| `cheese_thickness_mm` | Thickness of food layer (mm) |
| `tissue_depth_mm` | Depth of tissue domain (mm) |
| `grid_steps_temp` | Number of temperature steps in grid scan |
| `grid_steps_time` | Number of time steps in grid scan |

---

## Building and Running (C++)

Requires a C++11-compatible compiler.

```bash
# Single simulation
g++ -O2 -o sim_single simulation_3d_single.cpp
./sim_single
# Outputs: simulation_results.csv

# Grid scan
g++ -O2 -o sim_grid simulation_3d_grid.cpp
./sim_grid
# Outputs: grid_1st.csv, grid_2nd.csv, grid_3rd.csv
```

---

## Visualisation (Python)

Requires Python 3 with `pandas`, `numpy`, and `matplotlib`.

```bash
pip install pandas numpy matplotlib
python simulation_grid.py
# Outputs: burn_poster.png
```

The script generates a 2×2 figure:

| Panel | Contents |
|-------|----------|
| Top-left | Temperature profile with burn-degree boundaries (single sim) |
| Top-right | Max 1st-degree burn depth across parameter space |
| Bottom-left | Max 2nd-degree burn depth across parameter space |
| Bottom-right | Max 3rd-degree burn depth across parameter space |

---

## Output Files

| File | Description |
|------|-------------|
| `simulation_results.csv` | Time × depth grid of temperature and damage accumulation |
| `grid_1st.csv` | Max 1st-degree burn depth (mm) for each (temp, time) pair |
| `grid_2nd.csv` | Max 2nd-degree burn depth (mm) for each (temp, time) pair |
| `grid_3rd.csv` | Max 3rd-degree burn depth (mm) for each (temp, time) pair |
| `burn_poster.png` | Composite visualisation |

---

## Limitations

- 1D heat transfer only — no lateral conduction or saliva cooling effects
- Tissue is modelled as homogeneous; no layered skin/mucosal structure
- Burn thresholds are empirically fitted; values should be validated against clinical data before any medical application
- Cheese is treated as a passive thermal reservoir with fixed properties

---

## License

MIT
