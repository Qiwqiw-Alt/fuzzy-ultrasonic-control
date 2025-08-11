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
            d1, d2, d3, d4, direction_val = map(int, line.split(","))
            direction_enum = Direction(direction_val)
            print(f"D1= {d1}, D2= {d2}, D3= {d3}, D4= {d4} -> Direction: {direction_enum.name}")
        except Exception as e:
            print("Data error:", line, e)
              