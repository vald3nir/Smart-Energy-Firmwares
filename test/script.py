# coding=utf-8
# !/usr/bin/python3

import matplotlib.pyplot as plt

import time

from serial import Serial

COM = 'COM4'

NUMBER_OF_SAMPLES = 5 * 64  # 5 periods of 64 samples

connection_serial = Serial(COM, baudrate=115200, timeout=1)
time.sleep(1.8)

current_signal_temp = []

# start the acquisition
connection_serial.write(b't')
for i in range(0, NUMBER_OF_SAMPLES):
    current_signal_temp.append(float(connection_serial.readline()))
connection_serial.close()

# ------------------------------------------------------------------------------------------------------------------
# Plot do gráfico
# ------------------------------------------------------------------------------------------------------------------

plt.plot(current_signal_temp)
plt.show()
