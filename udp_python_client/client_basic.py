import socket
import time
import numpy as np
import struct

udp_server_addr = ("127.0.0.1", 3121)
bufsize = 512

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# client_socket.settimeout(0.0)
client_socket.sendto(bytearray("hello...".encode()), udp_server_addr)

