import sys
import time
import cv2
import matplotlib.patheffects as path_effects
import matplotlib.pyplot as plt
from matplotlib.widgets import Button
import numpy as np
import serial
import serial.tools.list_ports

# --- Configuration ---
BAUD_RATE = 115200
ROWS, COLS = 8, 8
NUM_PIXELS = ROWS * COLS

BEGIN_BYTE = b"\xfe"
END_BYTE = b"\xff"

# Threshold choices for gesture detection sensitivity
THRESHOLD_OPTIONS = ("Low (0.5)", "Medium (1.0)", "High (1.5)", "Ultra (2.0)")
THRESHOLD_VALUES = {
    "Low (0.5)": 0.5,
    "Medium (1.0)": 1.0,
    "High (1.5)": 1.5,
    "Ultra (2.0)": 2.0,
}


def auto_find_port():
    """Automatically finds the Arduino / Serial port across Mac, Linux, and Windows."""
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        return None

    target_keywords = [
        "arduino",
        "usb",
        "ch340",
        "cp210",
        "ftdi",
        "usbmodem",
        "usbserial",
        "acm",
    ]

    for port in ports:
        port_info = f"{port.device} {port.description} {port.hwid}".lower()
        if any(keyword in port_info for keyword in target_keywords):
            print(f"Detected device: {port.device} ({port.description})")
            return port.device

    print(f"No specific matching device found. Trying first available port: {ports[0].device}")
    return ports[0].device


def detect_gesture(prev_grid, curr_grid, threshold):
    """
    Detects horizontal swipe gestures (Left / Right) based on thermal motion optical flow.
    Returns 'RIGHT', 'LEFT', or None.
    """
    # Upscale 8x8 matrix to 32x32 for smooth optical flow analysis
    prev_resized = cv2.resize(prev_grid, (32, 32), interpolation=cv2.INTER_CUBIC)
    curr_resized = cv2.resize(curr_grid, (32, 32), interpolation=cv2.INTER_CUBIC)

    # Calculate absolute temperature difference
    diff = np.abs(curr_resized.astype(np.float32) - prev_resized.astype(np.float32))
    mask = diff > 0.8  # Mask out ambient background noise

    if not np.any(mask):
        return None

    p_norm = cv2.normalize(prev_resized, None, 0, 255, cv2.NORM_MINMAX).astype(np.uint8)
    c_norm = cv2.normalize(curr_resized, None, 0, 255, cv2.NORM_MINMAX).astype(np.uint8)

    # Farneback Optical Flow
    flow = cv2.calcOpticalFlowFarneback(
        p_norm, c_norm, None, 0.5, 3, 15, 3, 5, 1.2, 0
    )

    # Average horizontal motion displacement (dx)
    dx = np.mean(flow[..., 0][mask])

    # Classify swipe gesture based on user-selected threshold
    if dx > threshold:
        return "RIGHT"
    elif dx < -threshold:
        return "LEFT"

    return None


def main():
    # 1. Dynamically find serial port
    serial_port = auto_find_port()
    if not serial_port:
        print("Error: No serial ports detected. Please plug in your Arduino.")
        sys.exit(1)

    # 2. Initialize Serial Connection
    try:
        ser = serial.Serial(serial_port, BAUD_RATE, timeout=1.0)
        time.sleep(2)  # Wait for Arduino reboot
        ser.reset_input_buffer()
        print(f"Successfully connected to {serial_port} at {BAUD_RATE} baud.")
    except Exception as e:
        print(f"Error opening serial port {serial_port}: {e}")
        sys.exit(1)

    # 3. Setup Compact Plotting Environment
    plt.ion()
    fig = plt.figure(figsize=(4.8, 5.7))
    fig.canvas.manager.set_window_title("AMG8833 Thermal Diff & Gesture Viewer")

    # Main Display Axes (Shifted slightly upward to provide breathing room)
    ax = fig.add_axes([0.08, 0.28, 0.80, 0.65])

    grid_data = np.zeros((ROWS, COLS))
    im = ax.imshow(
        grid_data, cmap="coolwarm", interpolation="nearest", vmin=-10.0, vmax=10.0
    )

    cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
    cbar.set_label("Temp Delta (°C)", fontsize=8)
    ax.axis("off")
    ax.set_title("AMG8833 Thermal Diff & Gesture", fontsize=11, fontweight="bold", pad=8)

    # Gesture Notification Text Overlay (Moved down to 8.4 for comfortable padding from window)
    gesture_text = ax.text(
        3.5, 8.4, "", ha="center", va="center", fontsize=12, fontweight="bold", color="yellow"
    )
    gesture_text.set_path_effects([path_effects.withStroke(linewidth=3, foreground="black")])

    # Pre-create 8x8 numerical overlay
    text_grid = []
    for r in range(ROWS):
        row_texts = []
        for c in range(COLS):
            txt = ax.text(
                c, r, "", ha="center", va="center", color="white", fontsize=8, fontweight="bold"
            )
            txt.set_path_effects([path_effects.withStroke(linewidth=2, foreground="black")])
            txt.set_visible(False)
            row_texts.append(txt)
        text_grid.append(row_texts)

    # --- GUI Control Buttons (Lowered slightly to fit the downward shifted SWIPE text) ---
    btn_w = 0.26
    btn_h = 0.045
    y_row1 = 0.09
    y_row2 = 0.035
    x_col1, x_col2, x_col3 = 0.08, 0.37, 0.66

    ax_diff_mag = fig.add_axes([x_col1, y_row1, btn_w, btn_h])
    ax_temp = fig.add_axes([x_col2, y_row1, btn_w, btn_h])
    ax_thresh = fig.add_axes([x_col3, y_row1, btn_w, btn_h])

    ax_quit = fig.add_axes([x_col3, y_row2, btn_w, btn_h])

    # Diff Magnification Toggle (10x vs 1x)
    diff_mag_10x = [True]
    btn_diff_mag = Button(
        ax_diff_mag, "Mag: 10x", color="#77dd77", hovercolor="#55bb55"
    )

    def toggle_diff_mag(event):
        diff_mag_10x[0] = not diff_mag_10x[0]
        if diff_mag_10x[0]:
            btn_diff_mag.label.set_text("Mag: 10x")
            btn_diff_mag.color = "#77dd77"
            btn_diff_mag.hovercolor = "#55bb55"
        else:
            btn_diff_mag.label.set_text("Mag: 1x")
            btn_diff_mag.color = "0.85"
            btn_diff_mag.hovercolor = "0.75"
        fig.canvas.draw_idle()

    btn_diff_mag.on_clicked(toggle_diff_mag)

    # Temperature Overlay Toggle Button
    show_temp = [False]
    btn_temp = Button(ax_temp, "Text: OFF", color="0.85", hovercolor="0.75")

    def toggle_temp(event):
        show_temp[0] = not show_temp[0]
        if show_temp[0]:
            btn_temp.label.set_text("Text: ON")
            btn_temp.color = "#77dd77"
            btn_temp.hovercolor = "#55bb55"
        else:
            btn_temp.label.set_text("Text: OFF")
            btn_temp.color = "0.85"
            btn_temp.hovercolor = "0.75"
            for r in range(ROWS):
                for c in range(COLS):
                    text_grid[r][c].set_visible(False)
        fig.canvas.draw_idle()

    btn_temp.on_clicked(toggle_temp)

    # Threshold Dropdown Menu setup
    current_threshold = [1.0]  # Default threshold
    btn_thresh = Button(ax_thresh, "Thresh: 1.0 ▾", color="0.85", hovercolor="0.75")
    dropdown_axes = []
    dropdown_buttons = []
    dropdown_visible = [False]

    def toggle_dropdown(event):
        dropdown_visible[0] = not dropdown_visible[0]
        for ax_item in dropdown_axes:
            ax_item.set_visible(dropdown_visible[0])
        fig.canvas.draw_idle()

    btn_thresh.on_clicked(toggle_dropdown)

    for i, t_name in enumerate(THRESHOLD_OPTIONS):
        ax_item = fig.add_axes([x_col3, y_row1 + (i + 1) * 0.046, btn_w, 0.044])
        btn_item = Button(ax_item, t_name, color="0.95", hovercolor="#77dd77")
        ax_item.set_visible(False)
        dropdown_axes.append(ax_item)
        dropdown_buttons.append(btn_item)

        def make_select_callback(name):
            def on_select(event):
                val = THRESHOLD_VALUES[name]
                current_threshold[0] = val
                btn_thresh.label.set_text(f"Thresh: {val} ▾")
                toggle_dropdown(None)
            return on_select

        btn_item.on_clicked(make_select_callback(t_name))

    # Quit Button
    btn_quit = Button(ax_quit, "Quit", color="#ff8888", hovercolor="#ff4444")
    is_running = [True]

    def on_quit(event):
        is_running[0] = False
        plt.close(fig)

    btn_quit.on_clicked(on_quit)

    prev_temp_grid = None
    last_gesture_time = 0.0

    print("Starting Thermal Diff & Gesture Viewer... Press Ctrl+C or click Quit to exit.")

    try:
        while is_running[0] and plt.fignum_exists(fig.number):
            # Flush buffer for zero latency
            if ser.in_waiting > (NUM_PIXELS + 2) * 2:
                ser.reset_input_buffer()

            # Sync stream with BEGIN byte [0xFE]
            if ser.read(1) != BEGIN_BYTE:
                continue

            # Read full 64-pixel payload
            pixel_bytes = ser.read(NUM_PIXELS)
            if len(pixel_bytes) != NUM_PIXELS:
                continue

            # Verify END byte [0xFF]
            if ser.read(1) != END_BYTE:
                continue

            # Convert binary data to 8x8 matrix (1 LSB = 0.25°C)
            raw_grid = np.frombuffer(pixel_bytes, dtype=np.uint8).reshape((ROWS, COLS))
            temp_grid = raw_grid.astype(np.float32) * 0.25
            temp_grid = np.flipud(temp_grid)  # Flip vertically

            # Compute Differential Thermal Grid
            mag_factor = 10.0 if diff_mag_10x[0] else 1.0
            if prev_temp_grid is not None:
                display_grid = (temp_grid - prev_temp_grid) * mag_factor
                
                # --- Gesture Detection Logic ---
                gesture = detect_gesture(prev_temp_grid, temp_grid, current_threshold[0])
                curr_time = time.time()
                
                if gesture and (curr_time - last_gesture_time > 0.6):  # Debounce gesture detection
                    last_gesture_time = curr_time
                    if gesture == "RIGHT":
                        gesture_text.set_text(">>> SWIPE RIGHT >>>")
                        gesture_text.set_color("#00FFCC")
                    elif gesture == "LEFT":
                        gesture_text.set_text("<<< SWIPE LEFT <<<")
                        gesture_text.set_color("#FF3366")
            else:
                display_grid = np.zeros((ROWS, COLS), dtype=np.float32)

            prev_temp_grid = temp_grid.copy()

            # Clear gesture message after 0.8 seconds
            if time.time() - last_gesture_time > 0.8:
                gesture_text.set_text("")

            # Update heatmap display
            im.set_data(display_grid)

            # Update text overlay
            if show_temp[0]:
                for r in range(ROWS):
                    for c in range(COLS):
                        val = display_grid[r, c]
                        if diff_mag_10x[0]:
                            text_grid[r][c].set_text(f"{int(round(val)):+d}")
                        else:
                            text_grid[r][c].set_text(f"{val:+.1f}")
                        text_grid[r][c].set_visible(True)

            fig.canvas.draw_idle()
            fig.canvas.flush_events()

    except KeyboardInterrupt:
        print("\nStopping viewer...")
    finally:
        ser.close()
        plt.ioff()
        plt.close("all")
        print("Application closed successfully.")


if __name__ == "__main__":
    main()