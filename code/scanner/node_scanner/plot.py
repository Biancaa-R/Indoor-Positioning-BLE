import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import json
import random

# Initialize figure
fig, ax = plt.subplots()
vib_data, dist_data, motion_data = [], [], []
time_data = []

line_vib, = ax.plot([], [], label='Vibration')
line_dist, = ax.plot([], [], label='Distance')
line_motion, = ax.plot([], [], label='Motion')

ax.set_xlim(0, 20)
ax.set_ylim(0, 2000)
ax.legend()
ax.set_xlabel("Time (s)")
ax.set_ylabel("Value")

counter = 0

def update(frame):
    global counter

    # Simulate JSON string as if coming from ESP32
    simulated_json = json.dumps({
        "vib": random.randint(900, 1200),
        "dist": round(random.uniform(20, 35), 2),
        "motion": random.randint(0, 1)
    })

    # Parse JSON
    data = json.loads(simulated_json)
    vib_data.append(data["vib"])
    dist_data.append(data["dist"])
    motion_data.append(data["motion"])
    time_data.append(counter)
    counter += 1

    # Keep last 20 points
    vib_data_trim = vib_data[-20:]
    dist_data_trim = dist_data[-20:]
    motion_data_trim = motion_data[-20:]
    time_trim = time_data[-20:]

    line_vib.set_data(time_trim, vib_data_trim)
    line_dist.set_data(time_trim, dist_data_trim)
    line_motion.set_data(time_trim, motion_data_trim)

    ax.set_xlim(max(0, counter-20), counter)
    return line_vib, line_dist, line_motion

ani = FuncAnimation(fig, update, interval=500)
plt.show()
