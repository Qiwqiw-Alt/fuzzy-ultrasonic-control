import serial
from enum import Enum

class Direction(Enum):
    BACKWARD = 0
    LEFT = 1
    RIGHT = 2
    STRAIGHT_FORWARD = 3

ser = serial.Serial('COM4', 9600, timeout= 1)

while True:
    line = ser.readline().decode().strip()
    if line:
        try:
            direction_val = int(line)
            print(f"Direction: {direction_val}")
        except Exception as e:
            print("Data error:", line, e)
              
