import socket


udp_server_addr = ("192.168.29.255", 10103)
bufsize = 512

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client_socket.settimeout(1.0)

client_socket.sendto(str.encode("yo this python biiiitch"), udp_server_addr)
udp_server_response = client_socket.recvfrom(bufsize)
msg = "response from server {}".format(udp_server_response[0])
print(msg)
