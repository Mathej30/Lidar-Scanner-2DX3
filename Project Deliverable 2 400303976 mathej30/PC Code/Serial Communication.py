#Jibin Mathew
#mathej30
#400303976
#Serical Comminication

import serial
import math

s = serial.Serial('COM4', 115200, timeout=10)

# initialize the variables
angle = 0
xdistance = 200

# send the character 's' to MCU via UART
# This will signal MCU to start the transmission
s.write('s'.encode())

# receive 8 measurements from UART of MCU
f = open("data.xyz", "a")
while(1):
    input("Press Enter to start communication...")
    s.reset_input_buffer()
    s.reset_output_buffer()
    xdistance = xdistance + 200
    for i in range(32): 
        file = s.readline()
        distance = int(file.decode().strip())
        angle = angle + 11.25
        radians = math.radians(angle)
        x_point = xdistance
        y_point = distance * math.cos(radians)
        z_point = distance * math.sin(radians)
        f.write(f"{x_point} {y_point} {z_point}\n")
        print(distance, " ", x_point, " ", y_point, " ", z_point)
    f.flush()
f.close()
# close the port
s.close()

