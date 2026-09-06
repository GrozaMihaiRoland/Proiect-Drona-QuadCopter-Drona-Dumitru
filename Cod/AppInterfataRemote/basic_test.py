import socket
import struct
import threading

esp_ip_address = "192.168.4.1"
esp_port = 6000
laptop_port = 6000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', laptop_port))
sock.settimeout(1.0)
running = True

# send command to esp32
def command_loop():
    global running
    while running:
        user_input = input("Enter esp32 command:").split()

        #logic for different command lengths
        if len(user_input) == 4:
            command, byte1, byte2, byte3, byte4 = user_input
        if len(user_input) == 1:
            command = user_input

        if command == "com": # update commands
            packet = struct.pack('<BBbbb', 0x10, int(byte1), int(byte2), int(byte3), int(byte4))
            sock.sendto(packet, (esp_ip_address, esp_port))

        elif command == "stp":  # emergency stop command
            packet = struct.pack('<BBbbb', 0x20, 0, 0, 0, 0)
            sock.sendto(packet, (esp_ip_address, esp_port))

        elif command == "lnd":  # landing command
            packet = struct.pack('<BBbbb', 0x21, 0, 0, 0, 0)
            sock.sendto(packet, (esp_ip_address, esp_port))

        elif command == "hov":  # hover command
            packet = struct.pack('<BBbbb', 0x22, 0, 0, 0, 0)
            sock.sendto(packet, (esp_ip_address, esp_port))

        elif command == "wdg":  # watchdog command
            packet = struct.pack('<BBbbb', 0x00, 0, 0, 0, 0)
            sock.sendto(packet, (esp_ip_address, esp_port))

        else:
            print("ERROR: Invalid command")

# receive response from esp32
def receive_loop():
    while running:
        try:
            data, addr = sock.recvfrom(16)
            thrust, pitch, roll, yaw = struct.unpack('<Bbbb', data)
            print(f"thrust: {thrust}, pitch: {pitch}, roll: {roll}, yaw: {yaw} ")

        except socket.timeout:
            continue


rec_thread = threading.Thread(target=receive_loop)
rec_thread.start()

command_loop()

rec_thread.join()
sock.close()