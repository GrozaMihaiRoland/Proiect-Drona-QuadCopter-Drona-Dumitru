import socket
import struct
import threading
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

HOST = ''
PORT = 7500
MAX_POINTS = 200
SAMPLE_RATE = 200  # Hz — match your Arduino send rate

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((HOST, PORT))
sock.settimeout(1.0)

altitudes     = deque(maxlen=MAX_POINTS)
altitudes_kal = deque(maxlen=MAX_POINTS)
times         = deque(maxlen=MAX_POINTS)
t       = 0
running = True

def receive_loop():
    global t
    while running:
        try:
            data, addr = sock.recvfrom(32)
            if len(data) == 10:
                blev, alt, alt_kal = struct.unpack('<Hff', data)
                altitudes.append(alt)
                altitudes_kal.append(alt_kal)
                times.append(t)
                t += 1
                print(f"Sensor: {alt:.3f} m  Kalman: {alt_kal:.3f} m  Battery: {blev} mV")
        except socket.timeout:
            continue
        except struct.error as e:
            print(f"Unpack error: {e}")

rec_thread = threading.Thread(target=receive_loop, daemon=True)
rec_thread.start()

fig, axes = plt.subplots(2, 2, figsize=(14, 8))
ax_time = axes[0, 0]  # top left  — time domain
ax_fft  = axes[0, 1]  # top right — FFT
ax_kal  = axes[1, 0]  # bot left  — kalman time domain
ax_fftk = axes[1, 1]  # bot right — kalman FFT

stats_text = ax_time.text(0.02, 0.95, '', transform=ax_time.transAxes,
                           verticalalignment='top', fontsize=9,
                           bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))

# Time domain — raw sensor
line_raw,  = ax_time.plot([], [], 'b-', linewidth=1.2, label='Raw sensor')
ax_time.set_xlabel("Sample")
ax_time.set_ylabel("Altitude (m)")
ax_time.set_title("Raw Sensor Altitude")
ax_time.grid(True)
ax_time.legend()



# FFT — raw sensor
line_fft_raw, = ax_fft.plot([], [], 'b-', linewidth=1.2)
ax_fft.set_xlabel("Frequency (Hz)")
ax_fft.set_ylabel("Magnitude (m)")
ax_fft.set_title("FFT — Raw Sensor")
ax_fft.set_xlim(0, SAMPLE_RATE / 2)
ax_fft.grid(True)

# Time domain — Kalman
line_kal, = ax_kal.plot([], [], 'r-', linewidth=1.2, label='Kalman')
ax_kal.set_xlabel("Sample")
ax_kal.set_ylabel("Altitude (m)")
ax_kal.set_title("Kalman Filter Altitude")
ax_kal.grid(True)
ax_kal.legend()

# FFT — Kalman
line_fft_kal, = ax_fftk.plot([], [], 'r-', linewidth=1.2)
ax_fftk.set_xlabel("Frequency (Hz)")
ax_fftk.set_ylabel("Magnitude (m)")
ax_fftk.set_title("FFT — Kalman Output")
ax_fftk.set_xlim(0, SAMPLE_RATE / 2)
ax_fftk.grid(True)

def compute_fft(signal_deque):
    sig = np.array(signal_deque)
    sig = sig - np.mean(sig)  # remove DC
    n = len(sig)
    mag   = np.abs(np.fft.rfft(sig)) * 2 / n
    freqs = np.fft.rfftfreq(n, d=1.0 / SAMPLE_RATE)
    return freqs, mag

def update(frame):
    t_data = list(times)
    raw    = list(altitudes)
    kal    = list(altitudes_kal)

    if len(raw) == 0:
        return line_raw, line_kal, line_fft_raw, line_fft_kal, stats_text

    # time domain
    line_raw.set_data(t_data, raw)
    ax_time.relim(); ax_time.autoscale_view()

    line_kal.set_data(t_data, kal)
    ax_kal.relim(); ax_kal.autoscale_view()

    # stats on raw sensor
    if len(raw) >= 2:
        arr = np.array(raw)
        mean     = np.mean(arr)
        variance = np.var(arr)
        std      = np.std(arr)
        stats_text.set_text(f'Mean:  {mean:.4f} m\nVar:   {variance:.6f} m²\nStd:   {std:.4f} m')

    # FFT
    if len(raw) >= 16:
        freqs, mag_raw = compute_fft(altitudes)
        line_fft_raw.set_data(freqs, mag_raw)
        ax_fft.relim(); ax_fft.autoscale_view()
        ax_fft.set_xlim(0, SAMPLE_RATE / 2)

        freqs, mag_kal = compute_fft(altitudes_kal)
        line_fft_kal.set_data(freqs, mag_kal)
        ax_fftk.relim(); ax_fftk.autoscale_view()
        ax_fftk.set_xlim(0, SAMPLE_RATE / 2)

    return line_raw, line_kal, line_fft_raw, line_fft_kal, stats_text

ani = animation.FuncAnimation(fig, update, interval=100, blit=False)

plt.tight_layout()
plt.show()

running = False
sock.close()