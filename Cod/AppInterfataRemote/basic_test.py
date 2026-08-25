import socket
import struct
import threading

esp_ip_address = "192.168.100.200"
esp_port = 8000
laptop_port = 7500

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', laptop_port))
sock.settimeout(1.0)
running = True

# send command to esp32
def command_loop():
    global running
    while running:
        command = input("Enter esp32 command:")

        if command == "y":
            sock.sendto(b'\x01', (esp_ip_address, esp_port))
        elif command == "n":
            sock.sendto(b'\x02', (esp_ip_address, esp_port))
        elif command == 'e':
            running = False
        else:
            print("ERROR: Invalid command")

# receive response from esp32
def receive_loop():
    while running:
        try:
            data, addr = sock.recvfrom(16)
            motor_voltage, motor_current, battery_voltage = struct.unpack('<HHH', data)
            print(f"Motor2: {motor_voltage} mV, {motor_current} mA, Battery: {battery_voltage} mV")
        except socket.timeout:
            continue


rec_thread = threading.Thread(target=receive_loop)
rec_thread.start()

command_loop()

rec_thread.join()
sock.close()