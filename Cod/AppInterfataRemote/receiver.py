import socket
import struct

laptop_port = 6000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', laptop_port))

print(f'Started listening on port {laptop_port}...')

isRunning = True

while isRunning:
    try:
        data, addr = sock.recvfrom(16)
        thrust, pitch, roll, yaw = struct.unpack('<Bbbb', data)
        print(f"thrust: {thrust}, pitch: {pitch}, roll: {roll}, yaw: {yaw} ")

    except socket.timeout:
        continue
