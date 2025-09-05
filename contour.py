import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import griddata
import os
import sys

# Always load CSV from the same folder as the script
if getattr(sys, 'frozen', False):  # 如果是 exe
    script_dir = os.path.dirname(sys.executable)
else:  # 如果是 python 腳本
    script_dir = os.path.dirname(os.path.abspath(__file__))

csv_path = os.path.join(script_dir, "result.csv")

df = pd.read_csv(csv_path)

# Load CSV with proper columns
df = pd.read_csv(csv_path, usecols=["x", "y", "z", "magnitude"])

# Make sure numeric
df[["x","y","z","magnitude"]] = df[["x","y","z","magnitude"]].apply(pd.to_numeric, errors="coerce")

# Loop over each z slice
for z_val, group in df.groupby("z"):
    x = group["x"].values
    y = group["y"].values
    mag = group["magnitude"].values

    # Create grid for contour
    xi = np.linspace(x.min(), x.max(), 200)
    yi = np.linspace(y.min(), y.max(), 200)
    Xi, Yi = np.meshgrid(xi, yi)

    try:
        Zi = griddata((x, y), mag, (Xi, Yi), method="cubic")
    except Exception as e:
        print(f"Could not interpolate for z={z_val}: {e}")
        continue

    plt.figure(figsize=(6,5))
    contour = plt.contourf(Xi, Yi, Zi, levels=20, cmap="Spectral_r")
    plt.colorbar(contour, label="Magnitude (μT)")
    plt.title(f"Contour at z = {z_val}")
    plt.xlabel("X")
    plt.ylabel("Y")
    plt.tight_layout()

# 統一在最後顯示所有圖
plt.show()
