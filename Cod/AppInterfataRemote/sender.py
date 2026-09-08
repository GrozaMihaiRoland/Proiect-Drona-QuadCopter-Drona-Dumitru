import socket
import struct

isRunning = True

# Command Headers
COMMAND_HEADER_WATCH_DOG =                  0x00 # Use WTDOG

COMMAND_HEADER_SET_IDLE  =                  0x21 # Use SIDLE
COMMAND_HEADER_SET_ACTIVE =                 0x22 # Use SACTV

COMMAND_HEADER_SET_DUTY =                   0x10 # use SDUTY
COMMAND_HEADER_SET_COMMANDS =               0x11 # use SCOMD

COMMAND_HEADER_TRANSMIT_MODE_IDLE =         0x30 # use TIDLE
COMMAND_HEADER_TRANSMIT_MODE_TELEMETRY =    0x31 # use TTELE
COMMAND_HEADER_TRANSMIT_MODE_DUTY =         0x32 # use TDUTY
COMMAND_HEADER_TRANSMIT_MODE_VOLTAGE =      0x33 # use TVOL

# Connection data
esp_port = 6000
esp_ip_address = '192.168.4.1'
conn_data = esp_ip_address, esp_port

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f'Started transmission to port {esp_port}...')

def send_to_sock(header, p1, p2, p3, p4):
    packet = struct.pack('<Iffff', header, p1, p2, p3, p4)
    sock.sendto(packet, conn_data)

while isRunning:
    command = input('Enter command:')
    header, *payload = command.split()
    print(header, payload)

    if header == 'WTDOG':
        send_to_sock(COMMAND_HEADER_WATCH_DOG, 0, 0, 0, 0)

    elif header == 'SIDLE':
        send_to_sock(COMMAND_HEADER_SET_IDLE, 0, 0, 0, 0)
        pass

    elif header == 'SACTV':
        send_to_sock(COMMAND_HEADER_SET_ACTIVE, 0, 0, 0, 0)
        pass

    elif header == 'SDUTY':
        m1, m2, m3, m4 = payload
        # pack header and motor duty cyles in little endian format, header is uint32, commands are all floats
        send_to_sock(COMMAND_HEADER_SET_DUTY, float(m1), float(m2), float(m3), float(m4))

    elif header == 'SCOMD':
        #z_height, pitch, roll, yaw = payload
        # pack header and drone user commands in little endian format, header is uint32, commands are all floatss
        #send_to_sock(COMMAND_HEADER_SET_COMMANDS, float(z_height), float(pitch), float(roll), 0)

        for m1 in range(40, 61, 5):
            send_to_sock(COMMAND_HEADER_SET_DUTY, m1/100, 0, 0, 0)
            input("Next:")

    elif header == 'TIDLE':
        send_to_sock(COMMAND_HEADER_TRANSMIT_MODE_IDLE, 0, 0, 0, 0)

    elif header == 'TTELE':
        send_to_sock(COMMAND_HEADER_TRANSMIT_MODE_TELEMETRY, 0, 0, 0, 0)

    elif header == 'TDUTY':
        send_to_sock(COMMAND_HEADER_TRANSMIT_MODE_DUTY, 0, 0, 0, 0)

    elif header == 'TVOL':
        send_to_sock(COMMAND_HEADER_TRANSMIT_MODE_VOLTAGE, 0, 0, 0, 0)        

    else:
        print("ERROR: Invalid command")
