import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from matplotlib.colors import LinearSegmentedColormap

# ── Load single simulation ────────────────────────────────────────────────────
df = pd.read_csv("simulation_results.csv")
time_grid  = df.pivot(index="Time_s",   columns="Depth_mm", values="Time_s").values
depth_grid = df.pivot(index="Time_s",   columns="Depth_mm", values="Depth_mm").values
temp_grid  = df.pivot(index="Time_s",   columns="Depth_mm", values="Temperature_C").values
burn2_grid = df.pivot(index="Time_s",   columns="Depth_mm", values="Burn2").values
burn3_grid = df.pivot(index="Time_s",   columns="Depth_mm", values="Burn3").values

# ── Load contour grids ────────────────────────────────────────────────────────
df_1st = pd.read_csv("grid_1st.csv", index_col=0)
df_2nd = pd.read_csv("grid_2nd.csv", index_col=0)
df_3rd = pd.read_csv("grid_3rd.csv", index_col=0)

temperatures  = df_1st.index.astype(float).values
contact_times = df_1st.columns.astype(float).values
depths_1st    = df_1st.values
depths_2nd    = df_2nd.values
depths_3rd    = df_3rd.values

# ── Colourmaps ────────────────────────────────────────────────────────────────
cmap_1st = LinearSegmentedColormap.from_list(
    "burn1", ["#ffffff", "#fff9c4", "#ffe082", "#ffb300"], N=256)
cmap_2nd = LinearSegmentedColormap.from_list(
    "burn2", ["#ffffff", "#fff176", "#ff9800", "#e53935"], N=256)
cmap_3rd = LinearSegmentedColormap.from_list(
    "burn3", ["#ffffff", "#ff9800", "#e53935", "#7b1fa2"], N=256)

# ── Figure ────────────────────────────────────────────────────────────────────
fig, axes = plt.subplots(2, 2, figsize=(18, 14))
fig.patch.set_facecolor("#0f0f0f")

# ── Panel helper for contour grids ───────────────────────────────────────────
def plot_contour_panel(ax, depths, cmap, label, onset_color):
    ax.set_facecolor("#0f0f0f")
    max_depth = depths.max()
    if max_depth > 0:
        levels = np.linspace(0, max_depth, 200)
        cf = ax.contourf(contact_times, temperatures, depths,
                         levels=levels, cmap=cmap, extend="max")
        iso_levels = np.linspace(0, max_depth, 8)[1:]
        cs = ax.contour(contact_times, temperatures, depths,
                        levels=iso_levels, colors="white",
                        linewidths=0.6, alpha=0.5)
        ax.clabel(cs, fmt="%.2f mm", fontsize=8, inline=True,
                  inline_spacing=4, colors="white")
        ax.contour(contact_times, temperatures, depths,
                   levels=[1e-4], colors=[onset_color],
                   linewidths=2.0, linestyles="--")
        ax.plot([], [], color=onset_color, linestyle="--",
                linewidth=2.0, label="Burn onset")
        cbar = fig.colorbar(cf, ax=ax, pad=0.02, fraction=0.046)
        cbar.set_label("Max burn depth (mm)", fontsize=10, color="white")
        cbar.ax.yaxis.set_tick_params(color="white")
        cbar.ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%.2f"))
        plt.setp(cbar.ax.yaxis.get_ticklabels(), color="white")
        cbar.outline.set_edgecolor("white")
    else:
        ax.text(0.5, 0.5, f"No {label} burns detected",
                transform=ax.transAxes, ha="center", va="center",
                fontsize=13, color="white")
    ax.set_xlabel("Contact time (s)", fontsize=11, color="white")
    ax.set_ylabel("Food temperature (°C)", fontsize=11, color="white")
    ax.set_title(f"Max {label} Burn Depth", fontsize=13, color="white", pad=12)
    ax.tick_params(colors="white", labelsize=9)
    for spine in ax.spines.values():
        spine.set_edgecolor("#444444")
    ax.xaxis.set_major_locator(ticker.MaxNLocator(6))
    ax.yaxis.set_major_locator(ticker.MaxNLocator(7))
    ax.legend(fontsize=9, facecolor="#1a1a1a", edgecolor="#444444",
              labelcolor="white", loc="upper left")

# ── Top-left: single simulation ───────────────────────────────────────────────
ax0 = axes[0, 0]
ax0.set_facecolor("#0f0f0f")

cp = ax0.contourf(time_grid, depth_grid, temp_grid,
                  levels=50, cmap="YlOrRd", alpha=0.85)
cbar0 = fig.colorbar(cp, ax=ax0, pad=0.02, fraction=0.046)
cbar0.set_label("Temperature (°C)", fontsize=10, color="white")
cbar0.ax.yaxis.set_tick_params(color="white")
plt.setp(cbar0.ax.yaxis.get_ticklabels(), color="white")
cbar0.outline.set_edgecolor("white")

handles, labels = [], []

# 1st degree: 44°C isotherm
cs1 = ax0.contour(time_grid, depth_grid, temp_grid,
                  levels=[44.0], colors=["#ffb300"], linewidths=2)
h1, _ = cs1.legend_elements()
handles.append(h1[0]); labels.append("1st Degree")

# 2nd degree boundary
if burn2_grid.max() >= 1.0:
    cs2 = ax0.contour(time_grid, depth_grid, burn2_grid,
                      levels=[1.0], colors=["#00e5ff"], linewidths=2)
    h2, _ = cs2.legend_elements()
    handles.append(h2[0]); labels.append("2nd Degree")

# 3rd degree boundary
if burn3_grid.max() >= 1.0:
    cs3 = ax0.contour(time_grid, depth_grid, burn3_grid,
                      levels=[1.0], colors=["#e040fb"],
                      linestyles="--", linewidths=2.5)
    h3, _ = cs3.legend_elements()
    handles.append(h3[0]); labels.append("3rd Degree")

ax0.legend(handles, labels, loc="upper right",
           facecolor="#1a1a1a", edgecolor="#444444",
           labelcolor="white", fontsize=9, framealpha=1.0)
ax0.set_title("Temperature Profile with Burn Boundaries",
              fontsize=13, color="white", pad=12)
ax0.set_xlabel("Time (s)", fontsize=11, color="white")
ax0.set_ylabel("Depth (mm)", fontsize=11, color="white")
ax0.tick_params(colors="white", labelsize=9)
ax0.set_xlim(left=0)
ax0.set_ylim(bottom=0)
ax0.grid(True, linestyle=":", alpha=0.3, color="white")
for spine in ax0.spines.values():
    spine.set_edgecolor("#444444")

# ── Top-right: 1st degree contour ────────────────────────────────────────────
plot_contour_panel(axes[0, 1], depths_1st, cmap_1st, "1st-Degree", "#ffb300")

# ── Bottom-left: 2nd degree contour ──────────────────────────────────────────
plot_contour_panel(axes[1, 0], depths_2nd, cmap_2nd, "2nd-Degree", "#00e5ff")

# ── Bottom-right: 3rd degree contour ─────────────────────────────────────────
plot_contour_panel(axes[1, 1], depths_3rd, cmap_3rd, "3rd-Degree", "#00e5ff")

fig.suptitle("Thermal Burn Analysis: Single Simulation and Parameter Space",
             fontsize=16, color="white", y=1.01)

plt.tight_layout()
plt.savefig("burn_poster.png", dpi=300, bbox_inches="tight",
            facecolor=fig.get_facecolor())
print("Poster saved to burn_poster.png")
plt.show()