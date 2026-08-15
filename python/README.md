# Python Thermal Camera Viewer

Cross-platform real-time GUI thermal camera viewer for AMG8833 powered by Matplotlib, NumPy, and PySerial.

## Features

- **Automatic Serial Port Detection**: Automatically detects Arduino / USB-Serial devices across macOS, Linux, and Windows.
- **Interactive UI Controls**:
  - **Auto Scale**: Toggle automatic dynamic color-scale min/max scaling.
  - **Smooth Interpolation**: Switch between smooth Gaussian interpolation and sharp 8x8 matrix display.
  - **Temperature Overlay**: Display real-time numeric temperatures (°C) directly over pixels.
  - **Colormap Selection**: Dropdown menu offering `JET`, `HOT`, `INFERNO`, `PLASMA`, `COOLWARM`, and `VIRIDIS`.
  - **Binarization & Connected-Component Labeling**: Thresholding with real-time 8-connectivity island labeling.
  - **Frame Difference (Motion Detection)**: Real-time gradient diffing between consecutive frames to highlight motion.
  - **Dynamic Range Sliders**: Real-time `vmin` and `vmax` temperature range sliders.

## Prerequisites & Installation

```bash
pip install -r requirements.txt
```

## Running the Viewer

Ensure your Arduino (with `sketch_amg8833.ino`) is plugged into your computer via USB, then run:

```bash
python viewer.py
```
