import socket
import time
import numpy as np
import struct

udp_server_addr = ("192.168.0.255", 50134)
bufsize = 512

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client_socket.settimeout(1.0)

try:
	while(1):
		
		t = time.time()
		fingerpos = []
		for ch in range(0,6):
			fingerpos.append((np.cos(t)+1)*30)
		fingerpos[5] = -fingerpos[5]
		
		barr = []
		for fp in fingerpos:
			b4 = struct.pack('<f', fp)
			for b in b4:
				barr.append(b)
		b4 = struct.pack('<L',3000)
		for b in b4:
			barr.append(b)
		
		barr = bytearray(barr)
		
		client_socket.sendto(barr, udp_server_addr)
		#udp_server_response = client_socket.recvfrom(bufsize)
		#msg = "response from server {}".format(udp_server_response[0])
		time.sleep(.010)
except KeyboardInterrupt:
	pass
