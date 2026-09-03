import socket
import struct
import tkinter as tk
from tkinter import ttk


# ============================================================
# Configuration
# ============================================================

HOST = ""
PORT = 7500

ESP_IP = "192.168.100.200"
ESP_PORT = 8000

UPDATE_INTERVAL_MS = 50       # 20 Hz
MAX_DUTY = 1023


# ============================================================
# UDP
# ============================================================

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Listen for telemetry
sock.bind((HOST, PORT))

# Never allow recvfrom() to block the GUI
sock.setblocking(False)


# ============================================================
# State
# ============================================================

desired_duty = 0
last_sent_duty = None


# ============================================================
# Send duty to ESP
# ============================================================

def send_duty(duty: int):
    global last_sent_duty

    duty = max(0, min(MAX_DUTY, int(duty)))

    # uint16_t, little-endian
    data = struct.pack("<H", duty)

    try:
        sock.sendto(data, (ESP_IP, ESP_PORT))

        last_sent_duty = duty
        label_current.config(text=f"Duty: {duty}")

    except OSError as e:
        label_current.config(text=f"Send error: {e}")


# ============================================================
# Periodic duty transmission
# ============================================================

def send_loop():
    """
    Send the latest desired duty at a fixed rate.

    The slider does NOT directly send UDP packets.
    """

    send_duty(desired_duty)

    root.after(UPDATE_INTERVAL_MS, send_loop)


# ============================================================
# Receive telemetry
# ============================================================

def get_data():
    """Receive and display ESP telemetry."""

    try:
        while True:
            data, addr = sock.recvfrom(32)

            print(f"Received {len(data)} bytes from {addr}: {data.hex()}")

            if len(data) == 6:
                motor_voltage, motor_current, battery_voltage = \
                    struct.unpack("<HHH", data)

                print(
                    f"Voltage={motor_voltage} mV, "
                    f"Current={motor_current} mA, "
                    f"Battery={battery_voltage} mV"
                )

                label_motor_voltage.config(
                    text=f"Motor voltage: {motor_voltage} mV"
                )

                label_motor_current.config(
                    text=f"Motor current: {motor_current} mA"
                )

                label_battery.config(
                    text=f"Battery: {battery_voltage} mV"
                )

            else:
                print(
                    f"Unexpected packet from {addr}: "
                    f"{len(data)} bytes"
                )

    except BlockingIOError:
        pass

    except OSError as e:
        print(f"Receive error: {e}")

    root.after(UPDATE_INTERVAL_MS, get_data)

# ============================================================
# Slider
# ============================================================

def on_slider(value):
    global desired_duty

    desired_duty = int(float(value))

    label_current.config(
        text=f"Duty: {desired_duty}"
    )


# ============================================================
# Entry
# ============================================================

def on_entry():
    global desired_duty

    try:
        duty = int(entry_duty.get())

        if not 0 <= duty <= MAX_DUTY:
            raise ValueError

        desired_duty = duty

        slider.set(duty)

        # Send immediately when pressing Set
        send_duty(duty)

    except ValueError:
        label_current.config(
            text="Invalid input: use 0–1023"
        )


# ============================================================
# Stop
# ============================================================

def on_stop():
    global desired_duty

    desired_duty = 0

    slider.set(0)

    entry_duty.delete(0, tk.END)
    entry_duty.insert(0, "0")

    # Send stop immediately
    send_duty(0)


# ============================================================
# Close
# ============================================================

def on_close():
    global desired_duty

    desired_duty = 0

    try:
        # Make sure motor gets a stop command
        send_duty(0)
    except Exception:
        pass

    sock.close()
    root.destroy()


# ============================================================
# GUI
# ============================================================

root = tk.Tk()

root.title("Motor Control")
root.resizable(False, False)


frame = ttk.Frame(
    root,
    padding=20
)

frame.grid()


# ============================================================
# Duty cycle
# ============================================================

ttk.Label(
    frame,
    text="Duty cycle (0 – 1023)"
).grid(
    row=0,
    column=0,
    columnspan=2,
    pady=(0, 6)
)


slider = ttk.Scale(
    frame,
    from_=0,
    to=MAX_DUTY,
    orient="horizontal",
    length=400,
    command=on_slider
)

slider.grid(
    row=1,
    column=0,
    columnspan=2,
    pady=(0, 12)
)


# ============================================================
# Entry
# ============================================================

ttk.Label(
    frame,
    text="Enter value:"
).grid(
    row=2,
    column=0,
    sticky="e",
    padx=(0, 6)
)


entry_duty = ttk.Entry(
    frame,
    width=8
)

entry_duty.grid(
    row=2,
    column=1,
    sticky="w"
)

entry_duty.insert(0, "0")

entry_duty.bind(
    "<Return>",
    lambda event: on_entry()
)


# ============================================================
# Buttons
# ============================================================

button_frame = ttk.Frame(frame)

button_frame.grid(
    row=3,
    column=0,
    columnspan=2,
    pady=12
)


ttk.Button(
    button_frame,
    text="Set",
    command=on_entry
).grid(
    row=0,
    column=0,
    padx=6
)


ttk.Button(
    button_frame,
    text="STOP",
    command=on_stop
).grid(
    row=0,
    column=1,
    padx=6
)


# ============================================================
# Duty status
# ============================================================

label_current = ttk.Label(
    frame,
    text="Duty: 0"
)

label_current.grid(
    row=4,
    column=0,
    columnspan=2,
    pady=(0, 15)
)

slider.set(0)

# ============================================================
# Telemetry
# ============================================================

telemetry_frame = ttk.LabelFrame(
    frame,
    text="Motor telemetry",
    padding=10
)

telemetry_frame.grid(
    row=5,
    column=0,
    columnspan=2,
    sticky="ew"
)


label_motor_voltage = ttk.Label(
    telemetry_frame,
    text="Motor voltage: — mV"
)

label_motor_voltage.grid(
    row=0,
    column=0,
    sticky="w",
    pady=2
)


label_motor_current = ttk.Label(
    telemetry_frame,
    text="Motor current: — mA"
)

label_motor_current.grid(
    row=1,
    column=0,
    sticky="w",
    pady=2
)


label_battery = ttk.Label(
    telemetry_frame,
    text="Battery: — mV"
)

label_battery.grid(
    row=2,
    column=0,
    sticky="w",
    pady=2
)


# ============================================================
# Start periodic tasks
# ============================================================

root.after(
    UPDATE_INTERVAL_MS,
    get_data
)

root.after(
    UPDATE_INTERVAL_MS,
    send_loop
)


# ============================================================
# Safe shutdown
# ============================================================

root.protocol(
    "WM_DELETE_WINDOW",
    on_close
)


# ============================================================
# Main loop
# ============================================================

root.mainloop()