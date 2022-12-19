/*
	Simple UDP Server
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>
#include <WS2tcpip.h>
#include "winserial.h"
#include <stdint.h>
#include "WinUdpBkstServer.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 512	//Max length of buffer
#define PORT 10103	//The port on which to listen for incoming data
#define NO_DATA WSAEWOULDBLOCK

/*
To review my knowledge:
A UDP server is a socket which is bound to an address and port.
A single address can have multiple ports.
There are special addresses on a given network which can be used for different things.

For instance, if you bind the server to address in4addr_any, it means that messages directed EITHER
to the IP of the host or broadcasted to the entire network (there's a special address for that too)
will make it to the server, as long as it's on the right port #.

The server can tell who it received a packet from, and direct out a response to the sender specifically.
multiple servers can use the same port, but they'll receive traffic to that port if they're on the ANY ip address.

it's totally possible for a UDP server to get flooded with bogus packets from a malicious sender!

also looks like you need to allow the UDP server on the windows firewall or it'll get squished by default.
*/


typedef union u32_fmt_t
{
	uint32_t u32;
	int32_t i32;
	float f32;
	int16_t i16[sizeof(uint32_t) / sizeof(int16_t)];
	uint16_t ui16[sizeof(uint32_t) / sizeof(uint16_t)];
	int8_t i8[sizeof(uint32_t) / sizeof(int8_t)];
	uint8_t ui8[sizeof(uint32_t) / sizeof(uint8_t)];
}u32_fmt_t;



/*
Generic hex checksum calculation.
TODO: use this in the psyonic API
*/
uint32_t get_checksum32(uint32_t* arr, int size)
{
	int32_t checksum = 0;
	for (int i = 0; i < size; i++)
		checksum += (int32_t)arr[i];
	return -checksum;
}


int main()
{
	HANDLE usb_serial_port;
	if (connect_to_usb_serial(&usb_serial_port, "\\\\.\\COM4", 460800) == 0)
		printf("found com port success\r\n");
	else
		printf("failed to find com port\r\n");

	WinUdpBkstServer udp_server(PORT);
	udp_server.set_nonblocking();

	u32_fmt_t * fmt_buf;
	fmt_buf = (u32_fmt_t*)(&udp_server.r_buf[0]);

	//keep listening for data
	while (1)
	{
		//printf("Waiting for data...");
		//fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(udp_server.r_buf, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		int rc = udp_server.read();
		if (rc != 0 && rc != NO_DATA)
		{
			printf("recvfrom() failed with error code : %d", rc);
			//exit(EXIT_FAILURE);
		}

		if (udp_server.recv_len > 0)
		{
			int u32_len = udp_server.recv_len / sizeof(u32_fmt_t);
			if (get_checksum32((uint32_t*)udp_server.r_buf, u32_len - 1) == fmt_buf[u32_len - 1].u32)
			{
				printf("received: ");
				for (int i = 0; i < u32_len - 1; i++)
				{
					printf("%0.4f, ", fmt_buf[i].f32);
				}
				printf("\r\n");
			}
			else
			{
				printf("received %d bytes, checksum mismatch\r\n", udp_server.recv_len);
			}

			//forward the shit over usb serial
			DWORD dwbyteswritten;
			int uart_write_len = udp_server.recv_len;
			if (uart_write_len > BUFLEN)
				uart_write_len = BUFLEN;
			WriteFile(usb_serial_port, udp_server.r_buf, uart_write_len, &dwbyteswritten, NULL);

			memset(udp_server.t_buf, '\0', BUFLEN);
			sprintf((char*)(udp_server.t_buf), "why did you send me ");
			int hdr_len = strlen((char*)udp_server.t_buf);
			int cpy_idx = 0;
			for (int i = hdr_len; (udp_server.r_buf[cpy_idx] != 0) && (i < BUFLEN); i++)
			{
				udp_server.t_buf[i] = udp_server.r_buf[cpy_idx++];
			}
			int t_len = strlen((char*)udp_server.t_buf);

			//now reply the client with the same data
			if (sendto(udp_server.s, (const char *)udp_server.t_buf, t_len, 0, (struct sockaddr*)&udp_server.si_other, udp_server.slen) == SOCKET_ERROR)
			{
				printf("sendto() failed with error code : %d", WSAGetLastError());
				//exit(EXIT_FAILURE);
			}
		}
		//else
		//{
		//	printf("nothing yet...\r\n");
		//}
	}

	closesocket(udp_server.s);
	WSACleanup();

	return 0;
}