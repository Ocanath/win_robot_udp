/*
	Simple UDP Server
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>
#include <WS2tcpip.h>
#include "winserial.h"
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 512	//Max length of buffer
#define PORT 10103	//The port on which to listen for incoming data


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


int main()
{
	HANDLE usb_serial_port;
	if (connect_to_usb_serial(&usb_serial_port, "\\\\.\\COM4", 460800) == 0)
		printf("found com port success\r\n");
	else
		printf("failed to find com port\r\n");


	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char r_buf[BUFLEN];	//receive buffer
	char t_buf[BUFLEN];	//transmit buffer
	WSADATA wsa;

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	/*obtain and display the host IP address to console before beginning*/
	char namebuf[256] = { 0 };
	char inet_addr_buf[256] = { 0 };	/*Buffer to be used for displaying string-format IP addresses*/
	int rc = gethostname(namebuf, 256);
	printf("hostname: %s\r\n", namebuf);
	hostent* phost = gethostbyname(namebuf);
	for (int i = 0; phost->h_addr_list[i] != NULL; i++)
	{
		PCSTR retv = inet_ntop(AF_INET, phost->h_addr_list[i], (PSTR)inet_addr_buf, 256);
		printf("Host has IP address %d: %s\r\n", i, inet_addr_buf);
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	//server.sin_addr.s_addr = inet_addr(inet_addr_buf);	//assign the server with one of the IPV4 addresses (the last one) in the host address list.
	server.sin_addr = in4addr_any;	//assign the desired port # to the special address 0.0.0.0, which is a 'meta address' used to specify that the server can have any IP address we like on the LAN!
	server.sin_port = htons(PORT);

	inet_ntop(AF_INET, &server.sin_addr.s_addr, (PSTR)inet_addr_buf, 256);	//convert again the value we copied thru and display
	printf("Binding to server address: %s with port %d\r\n", inet_addr_buf, PORT);
	
	//Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	//keep listening for data
	while (1)
	{
		//printf("Waiting for data...");
		//fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(r_buf, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, r_buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			//exit(EXIT_FAILURE);
		}

		//forward the shit over usb serial
		DWORD dwbyteswritten;
		int uart_write_len = recv_len;
		if (uart_write_len > BUFLEN)
			uart_write_len = BUFLEN;
		WriteFile(usb_serial_port, r_buf, uart_write_len, &dwbyteswritten, NULL);
		
		
		//PCSTR retv = inet_ntop(AF_INET, &si_other.sin_addr, (PSTR)inet_addr_buf, 256);
		//printf("Received packet from %s:%d\n", inet_addr_buf, ntohs(si_other.sin_port));
		//printf("Data: %s\n", r_buf);


		memset(t_buf, '\0', BUFLEN);
		sprintf(t_buf, "why did you send me ");
		int hdr_len = strlen(t_buf);
		int cpy_idx = 0;
		for (int i = hdr_len; (r_buf[cpy_idx] != 0) && (i < BUFLEN); i++)
		{
			t_buf[i] = r_buf[cpy_idx++];
		}
		int t_len = strlen(t_buf);

		//now reply the client with the same data
		if (sendto(s, t_buf, t_len, 0, (struct sockaddr*)&si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			//exit(EXIT_FAILURE);
		}
	}

	closesocket(s);
	WSACleanup();

	return 0;
}